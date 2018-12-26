#include <Arduino.h>

#include "Hm11.hpp"
#include "Terminal.hpp"

#include "Gadget.hpp"

// Main I/O components
static Hm11 ble;
static Terminal term;

static Gadget gadget;

// Interrupt handlers
static volatile bool btn_triggered = false;
static volatile unsigned long btn_trigger_at;

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
    gadget.load_default_program();

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
            gadget.request_toggle();
            processed = 4;

            // Notify the sender
            stream.write("OK+T");
        } else if (cmd_name == 'C') {
            // Clear command, start uploading
            gadget.request_downloading();
            processed = 4;

            // Notify the sender
            stream.write("OK+C");
        } else if (cmd_name == 'L') {
            // Load program from EEPROM
            if (gadget.load_program(ble.state() == BS_Connected ? HIGH : LOW)) {
                stream.write("OK+L");
            } else {
                stream.write("NO+L");
            }

            processed = 4;
        } else if (cmd_name == 'S') {
            // Save program to EEPROM
            gadget.save_program(ble.state() == BS_Connected ? HIGH : LOW);

            // Notify the sender
            stream.write("OK+S");

            processed = 4;
        } else if (cmd_name == 'D') {
            // Download command, accept new program steps
            processed = gadget.download_program(buffer, 4);

            // Notify the sender
            if (processed > 0)
                stream.write("OK+D");
            else
                stream.write("NO+D");
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

    if (gadget.state() != GS_Downloading) {
        // Put the bluetooth module to sleep if it's not in use
        // This only happens when not downloading, to handle reconnects faster
        if (ble.state() == BS_Standby && ble.last_change_elapsed() > 5000)
            ble.sleep();
    }

    // Run state machine
    gadget.loop();

    // Check if the interrupt was triggered
    auto now = millis();
    if (btn_triggered) {
        if (now - btn_trigger_at > REBOUND_MS) {
            btn_triggered = false;
            gadget.request_toggle();
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
