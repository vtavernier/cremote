#include <HardwareSerial.h>

#include "Terminal.hpp"

void Terminal::begin(uint16_t baudrate) {
    Serial.begin(baudrate);
}

void Terminal::poll(LineHandler handler) {
    while (Serial.available()) {
        char c = Serial.read();

        if (c == '\r') continue;

        if (c == '\n') {
            // Invoke handler for complete line
            handler(buffer_);
            // Clear line as it has been processed by the handler
            buffer_.clear();
            // Discard \n
            continue;
        } else if (buffer_.full()) {
            buffer_.clear();
        }

        buffer_.push_back(c);
    }
}
