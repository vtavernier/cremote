#pragma once

#include <NeoSWSerial.h>
#include <RingBuf.hpp>

#include "Config.hpp"

enum BleState {
    BS_Standby,
    BS_Connected,
    BS_SleepRequested,
    BS_Sleep,
    BS_WakeRequested
};

typedef void (*MessageHandler)(RingBuf<char, LINE_BUFFER_SIZE, int8_t> &);

class Hm11 {
    // Software serial for communication
    NeoSWSerial serial_;
    // Bluetooth module status
    BleState state_;
    // Ring buffer for reading from module
    RingBuf<char, LINE_BUFFER_SIZE, int8_t> buffer_;
    // Last time the state changed
    uint32_t last_state_change_;

    // Method to update state
    void setBleState(BleState new_state);

    // Friend method to handle input
    friend void handleBleChar(uint8_t);
public:
    Hm11();

    void begin();
    void reset();

    void poll(MessageHandler handler);

    bool sleep();
    bool wake();

    inline BleState state() const { return state_; }
    inline uint32_t last_change_elapsed() const { return millis() - last_state_change_; }

    inline NeoSWSerial &serial() { return serial_; }
    inline const NeoSWSerial &serial() const { return serial_; }
};
