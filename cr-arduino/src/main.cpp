#include <Arduino.h>
#include <NeoSWSerial.h>

NeoSWSerial mySerial(8, 9); // RX, TX  

static void handleBtChar(uint8_t c) {
	Serial.print((char)c);

	if (c == 'H') {
		digitalWrite(LED_BUILTIN, HIGH);
	} else if (c == 'L') {
		digitalWrite(LED_BUILTIN, LOW);
	}
}

void setup() {  
	pinMode(LED_BUILTIN, OUTPUT);

	// RESET pin
	pinMode(6, OUTPUT);

	// RESET sequence
	digitalWrite(LED_BUILTIN, HIGH);
	digitalWrite(6, LOW);
	delay(75);
	digitalWrite(6, HIGH);
	digitalWrite(LED_BUILTIN, LOW);

	mySerial.attachInterrupt(handleBtChar);
	mySerial.begin(9600);

	Serial.begin(9600);
}

// Line buffer to transmit only whole commands to the HM11
static char buf[32];
static char *bufptr = &buf[0];

void loop() {  
	char c;

	if (Serial.available()) {
		c = Serial.read();
		*bufptr++ = c;

		if (c == '\n') {
			mySerial.write(buf, bufptr - buf);
			Serial.write(buf, bufptr - buf);
			bufptr = &buf[0];
		} else if (bufptr >= buf + sizeof(buf)) {
			bufptr = &buf[0];
		}
	}
}
