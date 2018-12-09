#include <Arduino.h>


#include "Hm11.hpp"
#include "Terminal.hpp"

// Main I/O components
static Hm11 ble;
static Terminal term;

// Triggering status
static unsigned long long triggerMs = 0xFFFFFFFFll;
static volatile bool triggering = false;
static volatile bool trigger_ack = false;

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
    ble.begin(9600);
    ble.reset();

    // Prepare main serial
    term.begin(9600);

    // Handle button interrupts
    attachInterrupt(digitalPinToInterrupt(BTN_TRIGGER), handleBtnTrigger, RISING);
}

static void ble_handler(RingBuf<char, LINE_BUFFER_SIZE, int8_t> &buffer) {
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
    if (triggering && !trigger_ack) {
        triggerMs = millis();
        trigger_ack = true;

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
    } else if (now - triggerMs > 50) {
        trigger_ack = false;
        triggering = false;

        // Check that the button is still HIGH
        if (digitalRead(BTN_TRIGGER) != HIGH)
        {
            // Abort if the button is still actually LOW (i.e. rebound on press down)
            triggerMs = 0xFFFFFFFFll;
        }
    } else {
        digitalWrite(OUTPUT_1, LOW);
        digitalWrite(OUTPUT_2, LOW);
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
