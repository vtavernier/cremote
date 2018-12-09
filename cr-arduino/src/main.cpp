#include <Arduino.h>

#include "Step.hpp"

#include "Hm11.hpp"
#include "Terminal.hpp"

// Main I/O components
static Hm11 ble;
static Terminal term;

// Program memory
static Program<MAX_PROGRAM_STEPS> program;

enum ProgramState {
    PS_Standby,
    PS_Downloading,
    PS_Running,
    PS_Toggle,
};

static ProgramState state = PS_Standby;
static volatile ProgramState requested_state = PS_Standby;

static size_t program_counter;
static unsigned long program_step_start;
static unsigned long halfpress_delay;

void increment_pc(unsigned long now, bool absolute = false, int d = 1) {
    if (absolute)
        program_counter = d;
    else
        program_counter += d;

    program_step_start = now;
}

// Interrupt handlers
static bool btn_triggered = false;
static unsigned long btn_trigger_at;

void handleBtnTrigger() {
    btn_triggered = true;
    btn_trigger_at = millis();
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
    program.set_step_count(3);

    program.steps()[0].data()[0] = SN_WAIT;
    program.steps()[0].data()[1] = 0x1; // Delay in seconds
    program.steps()[0].data()[2] = 0x0;
    program.steps()[0].data()[3] = 0x1; // one unit

    program.steps()[1].data()[0] = SN_PRESS;
    program.steps()[1].data()[1] = 0x0; // Delay in milliseconds
    program.steps()[1].data()[2] = 500 / 255;
    program.steps()[1].data()[3] = 500 % 255;

    program.steps()[2].data()[0] = SN_LOOP;
    program.steps()[2].data()[1] = 0x0;
    program.steps()[2].data()[2] = 0x2; // LC = 2
    program.steps()[2].data()[3] = 0x0; // TG = 0

    // Handle button interrupts
    attachInterrupt(digitalPinToInterrupt(BTN_TRIGGER), handleBtnTrigger, RISING);
}

static void ble_handler(RingBuf<char, LINE_BUFFER_SIZE, int8_t> &buffer) {
    if (buffer.size() >= 4) {
        const char cmd_name = buffer[3];
        if (cmd_name == 'T') {
            requested_state = PS_Toggle;
            // Erase processed command
            buffer.erase_front(4);
        }
    }
}

void loop() {
    // Check for stuff from the bluetooth module
    ble.poll(ble_handler);

    if (state != PS_Downloading) {
        // Put the bluetooth module to sleep if it's not in use
        // This only happens when not downloading, to handle reconnects faster
        if (ble.state() == BS_Standby && ble.last_change_elapsed() > 5000)
            ble.sleep();
    }

    // Handle state change requests
    if (state == PS_Standby && (requested_state == PS_Toggle || requested_state == PS_Running)) {
        program_counter = 0;
        program_step_start = millis();
        requested_state = state = PS_Running;

        // Reset program state
        size_t max = program.step_count();
        for (size_t i = 0; i < max; ++i)
            program.steps()[i].begin();

        halfpress_delay = DEFAULT_HALFPRESS_DELAY;
    } else if (state == PS_Running && (requested_state == PS_Toggle || requested_state == PS_Standby)) {
        requested_state = state = PS_Standby;

        // Abort everything
        digitalWrite(OUTPUT_1, LOW);
        digitalWrite(OUTPUT_2, LOW);
    }

    auto now = millis();

    // Are we running a program?
    if (state == PS_Running) {
        auto &step = program.steps()[program_counter];

        if (step.name() == SN_PRESS) {
            auto d = step.step_millis();

            if (now - program_step_start > d + halfpress_delay) {
                digitalWrite(OUTPUT_2, LOW);
                digitalWrite(OUTPUT_1, LOW);

                // Go to next step after press
                increment_pc(now);
            } else if (now - program_step_start > halfpress_delay) {
                digitalWrite(OUTPUT_1, HIGH); // In case we somehow missed the first step
                digitalWrite(OUTPUT_2, HIGH);
            } else {
                digitalWrite(OUTPUT_1, HIGH);
            }
        } else if (step.name() == SN_WAIT) {
            if (now - program_step_start > step.step_millis()) {
                increment_pc(now);
            }
        } else if (step.name() == SN_LOOP) {
            if (step.loop_count_dec()) {
                // true: keep iterating
                increment_pc(now, true, step.loop_target());
            } else {
                // false: stop iterating
                increment_pc(now);
            }
        }

        // Detect program termination
        if (program_counter >= program.step_count()) {
            requested_state = state = PS_Standby;
        }
    }

    // Check if the interrupt was triggered
    now = millis();
    if (btn_triggered) {
        if (now - btn_trigger_at > REBOUND_MS) {
            btn_triggered = false;
            requested_state = PS_Toggle;
        }
    }
}

static void serial_handler(RingBuf<char, LINE_BUFFER_SIZE, int8_t> &buffer) {
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
}

void serialEvent() {
    term.poll(serial_handler);
}
