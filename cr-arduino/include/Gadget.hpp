#pragma once

#include "Config.hpp"
#include "ProgramState.hpp"

#include "RingBuf.hpp"

enum GadgetState {
    GS_Standby,
    GS_Downloading,
    GS_Running,
};

class Gadget {
    // State machine variables
    GadgetState state_;
    volatile GadgetState requested_state_;

    // Program memory
    Program<MAX_PROGRAM_STEPS> program_;
    ProgramState program_state_;

    void start_program();

    void stop_program();
public:
    Gadget();

    void init();

    void load_default_program();

    void request_start_program();

    void request_stop_program();

    void request_toggle();

    void request_downloading();

    bool load_program(int final_led_state);
    void save_program(int final_led_state);

    int download_program(RingBuf<char, LINE_BUFFER_SIZE, int8_t> &buffer, size_t data_offset);

    void loop();

    inline GadgetState state() const
    { return state_; }
};
