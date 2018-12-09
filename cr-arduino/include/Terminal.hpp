#pragma once

#include <stdint.h>

#include <RingBuf.hpp>

#include "Config.hpp"

typedef void (*LineHandler)(RingBuf<char, LINE_BUFFER_SIZE, int8_t> &);

class Terminal {
    RingBuf<char, LINE_BUFFER_SIZE, int8_t> buffer_;

public:
    void begin(uint16_t baudrate);

    void poll(LineHandler handler);
};
