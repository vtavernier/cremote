#include <Arduino.h>
#include <EEPROM.h>

#include "Step.hpp"
#include "z85.hpp"

#include "Hm11.hpp"
#include "Terminal.hpp"

// Main I/O components
static Hm11 ble;
static Terminal term;

// Program memory
static Program<MAX_PROGRAM_STEPS> program;

enum GadgetState {
    GS_Standby,
    GS_Downloading,
    GS_Running,
    GS_Toggle,
};

static GadgetState state = GS_Standby;
static volatile GadgetState requested_state = GS_Standby;

static ProgramState program_state;

// Interrupt handlers
static volatile bool btn_triggered = false;
static volatile unsigned long btn_trigger_at;

void handleBtnTrigger() {
    btn_triggered = true;
    btn_trigger_at = millis();
}

static void onStartRunning() {
    // Reset program state and program step data
    program_state.init();
    program.init();
}

static void onStopRunning() {
    digitalWrite(OUTPUT_1, LOW);
    digitalWrite(OUTPUT_2, LOW);
}

void setup() {
    // Status LED
    pinMode(LED_BUILTIN, OUTPUT);

    // Button pin
    pinMode(BTN_TRIGGER, INPUT_PULLUP);

    // 4n25 pins
    pinMode(OUTPUT_1, OUTPUT);
    pinMode(OUTPUT_2, OUTPUT);

    // Setup BLE
    ble.begin(9600);
    ble.reset();

    // Prepare main serial
    term.begin(9600);

    // Load default program
    program.set_step_count(4);

    program.steps()[0].data()[0] = SN_WAIT;
    program.steps()[0].data()[1] = 0x1; // Delay in seconds
    program.steps()[0].data()[2] = 0x0;
    program.steps()[0].data()[3] = 0x1; // one unit

    program.steps()[1].data()[0] = SN_SET_HALFPRESS;
    program.steps()[1].data()[1] = 0x0; // Delay in milliseconds
    program.steps()[1].data()[2] = 0x0;
    program.steps()[1].data()[3] = 0x0; // zero units

    program.steps()[2].data()[0] = SN_PRESS;
    program.steps()[2].data()[1] = 0x0; // Delay in milliseconds
    program.steps()[2].data()[2] = 500 / 255;
    program.steps()[2].data()[3] = 500 % 255;

    program.steps()[3].data()[0] = SN_LOOP;
    program.steps()[3].data()[1] = 0x0;
    program.steps()[3].data()[2] = 0x2; // LC = 2
    program.steps()[3].data()[3] = 0x0; // TG = 0

    // Handle button interrupts
    EIFR = (1 << digitalPinToInterrupt(BTN_TRIGGER));
    attachInterrupt(digitalPinToInterrupt(BTN_TRIGGER), handleBtnTrigger, RISING);
}

static void cmd_handler(Stream &stream, RingBuf<char, LINE_BUFFER_SIZE, int8_t> &buffer) {
    if (buffer.size() >= 4) {
        const char cmd_name = buffer[3];
        size_t processed = 0;

        if (cmd_name == 'T') {
            // Trigger command, request state toggle
            requested_state = GS_Toggle;
            processed = 4;

            // Notify the sender
            stream.write("OK+T");
        } else if (cmd_name == 'C') {
            // Clear command, start uploading
            requested_state = GS_Downloading;
            processed = 4;

            // Notify the sender
            stream.write("OK+C");
        } else if (cmd_name == 'L') {
            // Load program from EEPROM
            uint8_t sc = EEPROM.read(0);
            if (sc == 0 || sc > MAX_PROGRAM_STEPS) {
                stream.write("NO+L");
            } else {
                uint8_t last_sc = EEPROM.read(2 + 4 * sc);
                if (last_sc != sc)
                {
                    stream.write("NO+L");
                }
                else
                {
                    program.set_step_count(sc);
                    for (size_t i = 0; i < sc; ++i) {
                        for (int j = 0; j < 4; ++j) {
                            auto address = 1 + 4 * i + j;
                            digitalWrite(LED_BUILTIN, address % 2 == 0 ? HIGH : LOW);
                            program.steps()[i].data()[j] = EEPROM.read(address);
                        }
                    }

                    digitalWrite(LED_BUILTIN, ble.state() == BS_Connected ? HIGH : LOW);
                    // Notify the sender
                    stream.write("OK+L");
                }
            }

            processed = 4;
        } else if (cmd_name == 'S') {
            // Save program to EEPROM
            size_t sc = program.step_count();
            EEPROM.update(2 + 4 * sc, 0);
            EEPROM.update(0, sc);
            for (size_t i = 0; i < sc; ++i) {
                for (int j = 0; j < 4; ++j) {
                    auto address = 1 + 4 * i + j;
                    digitalWrite(LED_BUILTIN, address % 2 == 0 ? HIGH : LOW);
                    EEPROM.update(address, program.steps()[i].data()[j]);
                }
            }

            EEPROM.update(2 + 4 * sc, sc);

            digitalWrite(LED_BUILTIN, ble.state() == BS_Connected ? HIGH : LOW);
            processed = 4;

            // Notify the sender
            stream.write("OK+S");
        } else if (cmd_name == 'D') {
            // Download command, accept new program steps
            if (buffer.size() >= 5) {
                uint8_t control = buffer[4];

                // Extract fields from control
                uint8_t is_last = control & 0x80;
                uint8_t last_value_index = control & 0x3;
                uint8_t offset = (control & 0x7C) >> 2;

                // Check that we received everything in the ring buffer
                uint8_t data_size = (last_value_index + 1) * 5;
                if (buffer.size() >= 5 + data_size) {
                    // Everything is ready in the buffer, decode the commands
                    Z85_decode(buffer.begin() + 5, reinterpret_cast<char*>(program.steps()[offset].data()), 5);
                    if (last_value_index >= 1)
                        Z85_decode(buffer.begin() + 10, reinterpret_cast<char*>(program.steps()[offset + 1].data()), 5);
                    if (last_value_index >= 2)
                        Z85_decode(buffer.begin() + 15, reinterpret_cast<char*>(program.steps()[offset + 2].data()), 5);

                    // Compute the last index, update program size if needed
                    uint8_t total_step_count = offset + last_value_index + 1;
                    if (total_step_count > program.step_count())
                        program.set_step_count(total_step_count);

                    // If this is the last download message, switch back to standby
                    if (is_last)
                        requested_state = GS_Standby;

                    // We processed the entire message
                    processed = 5 + data_size;

                    // Notify the sender
                    stream.write("OK+D");
                }
            }
        }

        // Erase processed command
        if (processed)
            buffer.erase_front(processed);
    }
}

static void ble_handler(RingBuf<char, LINE_BUFFER_SIZE, int8_t> &buffer) {
    cmd_handler(ble.serial(), buffer);
}

void loop() {
    // Check for stuff from the bluetooth module
    ble.poll(ble_handler);

    if (state != GS_Downloading) {
        // Put the bluetooth module to sleep if it's not in use
        // This only happens when not downloading, to handle reconnects faster
        if (ble.state() == BS_Standby && ble.last_change_elapsed() > 5000)
            ble.sleep();
    }

    // Handle state change requests
    if (state == GS_Standby && (requested_state == GS_Toggle || requested_state == GS_Running)) {
        // Start running program
        onStartRunning();

        requested_state = state = GS_Running;
    } else if (state == GS_Running && (requested_state == GS_Toggle || requested_state == GS_Standby)) {
        // Abort everything
        onStopRunning();

        requested_state = state = GS_Standby;
    } else if (state != GS_Downloading && requested_state == GS_Downloading) {
        // Stop running if it was the case
        if (state == GS_Running)
            onStopRunning();

        // Switching to download mode is initiated by a clear
        program.set_step_count(0);
    } else if (state == GS_Downloading && requested_state == GS_Standby) {
        // Completing a download
        requested_state = state = GS_Standby;
    }

    // Are we running a program?
    if (state == GS_Running) {
        auto result = program_state.next_step(program);

        // Detect program termination
        if (result == PS_Completed) {
            requested_state = state = GS_Standby;
        }
    }

    // Check if the interrupt was triggered
    auto now = millis();
    if (btn_triggered) {
        if (now - btn_trigger_at > REBOUND_MS) {
            btn_triggered = false;
            requested_state = GS_Toggle;
        }
    }
}

static void serial_handler(RingBuf<char, LINE_BUFFER_SIZE, int8_t> &buffer) {
    if (buffer.size() >= 3 &&
        buffer[0] == 'C' &&
        buffer[1] == 'R' &&
        buffer[2] == '+') {
        // Handle the same way as bluetooth if it looks like a command
        cmd_handler(Serial, buffer);
    } else {
        int8_t start1, len1, start2, len2;
        const char *data = buffer.data();

        // Read buffer parts
        buffer.read_parts(start1, len1, start2, len2);

        // Write buffer parts as bulk
        if (len1) ble.serial().write(data + start1, len1);
        if (len2) ble.serial().write(data + start2, len2);
        if (len1) Serial.write(data + start1, len1);
        if (len2) Serial.write(data + start2, len2);

        Serial.write('\n');

        // Clear line as it has been processed
        buffer.clear();
    }
}

void serialEvent() {
    term.poll(serial_handler);
}
