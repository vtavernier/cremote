#include <Arduino.h>
#include <NeoSWSerial.h>

#include <RingBuf.hpp>

#define OUTPUT_1 6
#define OUTPUT_2 5
#define BTN_TRIGGER 2
#define BTH_RST 7
#define BTH_RX 8
#define BTH_TX 9

NeoSWSerial mySerial(BTH_RX, BTH_TX);

// Input line buffers for serial and bluetooth
#define LINE_BUFFER_SIZE 64
static RingBuf<char, LINE_BUFFER_SIZE, int8_t> serial_buffer;
static RingBuf<char, LINE_BUFFER_SIZE, int8_t> bluetooth_buffer;

// Triggering status
static unsigned long long triggerMs = 0xFFFFFFFFll;
static volatile bool triggering = false;

// Interrupt handlers
void handleBtChar(uint8_t c) { Serial.print((char)c); bluetooth_buffer.push_over(c); }
void handleBtnTrigger() { triggering = true; }

void setup() {
    // Status LED
    pinMode(LED_BUILTIN, OUTPUT);

    // Button pin
    pinMode(BTN_TRIGGER, INPUT_PULLUP);

    // 4n25 pins
    pinMode(OUTPUT_1, OUTPUT);
    pinMode(OUTPUT_2, OUTPUT);

    // RESET pin
    pinMode(BTH_RST, OUTPUT);

    // RESET sequence on BTH
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(BTH_RST, LOW);
    delay(75);
    digitalWrite(BTH_RST, HIGH);
    digitalWrite(LED_BUILTIN, LOW);

    // Prepare BTH serial
    mySerial.attachInterrupt(handleBtChar);
    mySerial.begin(9600);

    // Prepare main serial
    Serial.begin(9600);

    // Handle button interrupts
    attachInterrupt(digitalPinToInterrupt(BTN_TRIGGER), handleBtnTrigger, RISING);
}

void loop() {
    // Check for stuff from the bluetooth module
    while (bluetooth_buffer.size() > 3) {
        if (bluetooth_buffer[0] == 'O' &&
            bluetooth_buffer[1] == 'K' &&
            bluetooth_buffer[2] == '+') {
            // OK+ : AT notification from module
            // Read CONN or LOST
            if (bluetooth_buffer.size() >= 7) {
                if (bluetooth_buffer[3] == 'C' &&
                    bluetooth_buffer[4] == 'O' &&
                    bluetooth_buffer[5] == 'N' &&
                    bluetooth_buffer[6] == 'N')
                {
                    digitalWrite(LED_BUILTIN, HIGH);
                } else if (bluetooth_buffer[3] == 'L' &&
                           bluetooth_buffer[4] == 'O' &&
                           bluetooth_buffer[5] == 'S' &&
                           bluetooth_buffer[6] == 'T') {
                    digitalWrite(LED_BUILTIN, LOW);
                }

                // Consume command
                bluetooth_buffer.erase_front(7);
            }
        } else if (bluetooth_buffer[0] == 'C' &&
                   bluetooth_buffer[1] == 'R' &&
                   bluetooth_buffer[2] == '+') {
            // CR+ : Canon Remote command
            if (bluetooth_buffer.size() >= 4) {
                const char cmd_name = bluetooth_buffer[3];
                if (cmd_name == 'T') {
                    // Trigger
                    triggering = true;
                    // Erase processed command
                    bluetooth_buffer.erase_front(4);
                }
            }
        } else {
            // Are we reading garbage? Pop one-by-one
            bluetooth_buffer.pop_front();
        }
    }

    // Check if the interrupt was triggered
    if (triggering) {
        triggerMs = millis();
        triggering = false;
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

        if (c != '\r') continue;

        if (c == '\n') {
            int8_t start1, len1, start2, len2;
            const char *serial_data = serial_buffer.data();

            // Read buffer parts
            serial_buffer.read_parts(start1, len1, start2, len2);

            // Write buffer parts as bulk
            if (len1) mySerial.write(serial_data + start1, len1);
            if (len2) mySerial.write(serial_data + start2, len2);
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
