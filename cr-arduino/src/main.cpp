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

// Line buffer to transmit only whole commands to the HM11
#define SERIAL_BUFFER_SIZE 64
static RingBuf<char, SERIAL_BUFFER_SIZE, int8_t> serial_buffer;

// Triggering status
static unsigned long long triggerMs = 0;
static bool triggering = false;

static void handleBtChar(uint8_t c) {
	Serial.print((char)c);

	if (c == 'H') {
		triggering = true;
	}
}

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
}

void loop() {  
	char c;

	if (Serial.available()) {
		c = Serial.read();

		if (c != '\r') {
			if (c == '\n') {
				int8_t start1,
				       len1,
				       start2,
				       len2;
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
			} else if (serial_buffer.full()) {
				serial_buffer.clear();
			}

			if (c != '\n') {
				serial_buffer.push_back(c);
			}
		}
	}

	// Read button status
	auto btnVal = digitalRead(BTN_TRIGGER);

	if (btnVal == HIGH) {
		digitalWrite(LED_BUILTIN, LOW);
		if (triggering) {
			triggerMs = millis();
			triggering = false;
		}
	} else {
		digitalWrite(LED_BUILTIN, HIGH);
		triggering = true;
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
