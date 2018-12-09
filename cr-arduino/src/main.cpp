#include <Arduino.h>

#include <RingBuf.hpp>

#include "Hm11.hpp"

#define OUTPUT_1 6
#define OUTPUT_2 5
#define BTN_TRIGGER 2

static Hm11 ble;

// Input line buffers for serial and bluetooth
static RingBuf<char, LINE_BUFFER_SIZE, int8_t> serial_buffer;

// Triggering status
static unsigned long long triggerMs = 0xFFFFFFFFll;
static volatile bool triggering = false;

// Interrupt handlers
void handleBtnTrigger() { triggering = true; }

void setup() {
    // Status LED
    pinMode(LED_BUILTIN, OUTPUT);

    // Button pin
    pinMode(BTN_TRIGGER, INPUT_PULLUP);

    // 4n25 pins
    pinMode(OUTPUT_1, OUTPUT);
    pinMode(OUTPUT_2, OUTPUT);

    // Setup BLE
    ble.begin();
    ble.reset();

    // Prepare main serial
    Serial.begin(9600);

    // Handle button interrupts
    attachInterrupt(digitalPinToInterrupt(BTN_TRIGGER), handleBtnTrigger, RISING);
}

void ble_handler(RingBuf<char, LINE_BUFFER_SIZE, int8_t> &buffer) {
    if (buffer.size() >= 4) {
        const char cmd_name = buffer[3];
        if (cmd_name == 'T') {
            // Trigger
            triggering = true;
            // Erase processed command
            buffer.erase_front(4);
        }
    }
}

void loop() {
    // Check for stuff from the bluetooth module
    ble.poll(ble_handler);

    // Put the bluetooth module to sleep if it's not in use
    if (ble.state() == BS_Standby && ble.last_change_elapsed() > 5000)
        ble.sleep();

    // Check if the interrupt was triggered
    if (triggering) {
        triggerMs = millis();
        triggering = false;

        // Start waking up the module
        if (ble.state() == BS_Sleep)
            ble.wake();
    }

    auto now = millis();

    if (now - triggerMs > 500) {
        digitalWrite(OUTPUT_1, LOW);
        digitalWrite(OUTPUT_2, LOW);
    } else if (now - triggerMs > 300) {
        digitalWrite(OUTPUT_2, HIGH);
    } else if (now - triggerMs > 100) {
        digitalWrite(OUTPUT_1, HIGH);
    } else {
        digitalWrite(OUTPUT_1, LOW);
        digitalWrite(OUTPUT_2, LOW);
    }
}

void serialEvent() {
    while (Serial.available()) {
        char c = Serial.read();

        if (c == '\r') continue;

        if (c == '\n') {
            int8_t start1, len1, start2, len2;
            const char *serial_data = serial_buffer.data();

            // Read buffer parts
            serial_buffer.read_parts(start1, len1, start2, len2);

            // Write buffer parts as bulk
            if (len1) ble.serial().write(serial_data + start1, len1);
            if (len2) ble.serial().write(serial_data + start2, len2);
            if (len1) Serial.write(serial_data + start1, len1);
            if (len2) Serial.write(serial_data + start2, len2);

            Serial.write('\n');

            // Clear buffer data
            serial_buffer.clear();

            continue;
        } else if (serial_buffer.full()) {
            serial_buffer.clear();
        }

        serial_buffer.push_back(c);
    }
}
