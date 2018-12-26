#include <Arduino.h>
#include <EEPROM.h>

#include "Gadget.hpp"
#include "z85.hpp"

void Gadget::start_program() {
    // Reset program state and program step data
    program_state_.init();
    program_.init();

    requested_state_ = state_ = GS_Running;
}

void Gadget::stop_program() {
    // Abort outputs
    digitalWrite(program_state_.fullpress_output(), LOW);
    digitalWrite(program_state_.halfpress_output(), LOW);

    requested_state_ = state_ = GS_Standby;
}

Gadget::Gadget() { init(); }

void Gadget::init() {
    state_ = GS_Standby;
    requested_state_ = GS_Standby;
}

void Gadget::load_default_program() {
    program_.set_step_count(4);

    program_.steps()[0].data()[0] = SN_WAIT;
    program_.steps()[0].data()[1] = 0x1;  // Delay in seconds
    program_.steps()[0].data()[2] = 0x0;
    program_.steps()[0].data()[3] = 0x1;  // one unit

    program_.steps()[1].data()[0] = SN_SET_HALFPRESS;
    program_.steps()[1].data()[1] = 0x0;  // Delay in milliseconds
    program_.steps()[1].data()[2] = 0x0;
    program_.steps()[1].data()[3] = 100;  // 100 units

    program_.steps()[2].data()[0] = SN_PRESS;
    program_.steps()[2].data()[1] = 0x0;  // Delay in milliseconds
    program_.steps()[2].data()[2] = 500 / 255;
    program_.steps()[2].data()[3] = 500 % 255;

    program_.steps()[3].data()[0] = SN_LOOP;
    program_.steps()[3].data()[1] = 0x0;
    program_.steps()[3].data()[2] = 0x2;  // LC = 2
    program_.steps()[3].data()[3] = 0x0;  // TG = 0
}

void Gadget::request_start_program() {
    noInterrupts();
    if (state_ == GS_Standby) requested_state_ = GS_Running;
    interrupts();
}

void Gadget::request_stop_program() {
    noInterrupts();
    if (state_ == GS_Running) requested_state_ = GS_Standby;
    interrupts();
}

void Gadget::request_toggle() {
    noInterrupts();
    if (state_ == GS_Running)
        requested_state_ = GS_Standby;
    else if (state_ == GS_Standby)
        requested_state_ = GS_Running;
    interrupts();
}

void Gadget::request_downloading() { requested_state_ = GS_Downloading; }

bool Gadget::load_program(int final_led_state) {
    noInterrupts();
    // Loading program from EEPROM: stop running program
    if (state_ == GS_Running) stop_program();
    interrupts();

    // Load program from EEPROM
    uint8_t sc = EEPROM.read(0);
    if (sc == 0 || sc > MAX_PROGRAM_STEPS) {
        return false;
    } else {
        uint8_t last_sc = EEPROM.read(2 + 4 * sc);
        if (last_sc != sc) {
            return false;
        } else {
            program_.set_step_count(sc);
            for (size_t i = 0; i < sc; ++i) {
                for (int j = 0; j < 4; ++j) {
                    auto address = 1 + 4 * i + j;
                    digitalWrite(LED_BUILTIN, address % 2 == 0 ? HIGH : LOW);
                    program_.steps()[i].data()[j] = EEPROM.read(address);
                }
            }

            digitalWrite(LED_BUILTIN, final_led_state);
        }
    }

    return true;
}

void Gadget::save_program(int final_led_state) {
    // Save program to EEPROM
    size_t sc = program_.step_count();
    EEPROM.update(2 + 4 * sc, 0);
    EEPROM.update(0, sc);
    for (size_t i = 0; i < sc; ++i) {
        for (int j = 0; j < 4; ++j) {
            auto address = 1 + 4 * i + j;
            digitalWrite(LED_BUILTIN, address % 2 == 0 ? HIGH : LOW);
            EEPROM.update(address, program_.steps()[i].data()[j]);
        }
    }

    EEPROM.update(2 + 4 * sc, sc);

    digitalWrite(LED_BUILTIN, final_led_state);
}

int Gadget::download_program(RingBuf<char, LINE_BUFFER_SIZE, int8_t> &buffer, size_t data_offset) {
    if (buffer.size() >= static_cast<int8_t>(data_offset + 1)) {
        uint8_t control = buffer[data_offset];

        // Extract fields from control
        uint8_t is_last = control & 0x80;
        uint8_t last_value_index = control & 0x3;
        uint8_t offset = (control & 0x7C) >> 2;

        // Check that we received everything in the ring buffer
        uint8_t data_size = (last_value_index + 1) * 5;
        if (buffer.size() >= 5 + data_size) {
            // Everything is ready in the buffer, decode the commands
            Z85_decode(buffer.begin() + data_offset + 1,
                       reinterpret_cast<char *>(program_.steps()[offset].data()),
                       5);
            if (last_value_index >= 1)
                Z85_decode(buffer.begin() + data_offset + 6,
                           reinterpret_cast<char *>(
                               program_.steps()[offset + 1].data()),
                           5);
            if (last_value_index >= 2)
                Z85_decode(buffer.begin() + data_offset + 11,
                           reinterpret_cast<char *>(
                               program_.steps()[offset + 2].data()),
                           5);

            // Compute the last index, update program size if needed
            uint8_t total_step_count = offset + last_value_index + 1;
            if (total_step_count > program_.step_count())
                program_.set_step_count(total_step_count);

            // If this is the last download message, switch back to standby
            if (is_last) requested_state_ = GS_Standby;

            // We processed the entire message
            return data_offset + 1 + data_size;
        }
    }

    return 0;
}

void Gadget::loop() {
    // Handle state change requests
    if (state_ == GS_Standby && requested_state_ == GS_Running) {
        // Start running program
        start_program();
    } else if (state_ == GS_Running && requested_state_ == GS_Standby) {
        // Abort everything
        stop_program();
    } else if (state_ != GS_Downloading && requested_state_ == GS_Downloading) {
        noInterrupts();

        // Stop running if it was the case
        if (state_ == GS_Running) stop_program();

        // Switching to download mode is initiated by a clear
        program_.set_step_count(0);

        interrupts();
    } else if (state_ == GS_Downloading && requested_state_ == GS_Standby) {
        // Completing a download
        state_ = GS_Standby;
    }

    // Are we running a program?
    if (state_ == GS_Running) {
        auto result = program_state_.next_step(program_);

        // Detect program termination
        if (result == PS_Completed) {
            request_stop_program();
        }
    }
}
