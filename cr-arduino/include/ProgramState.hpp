#pragma once

#include "Program.hpp"

enum ProgramStatus {
    PS_Continue,
    PS_Completed,
};

class ProgramState {
    uint8_t program_counter_;
    unsigned long program_step_start_;
    unsigned long halfpress_delay_;

    int halfpress_output_;
    int fullpress_output_;

    friend Step;
    void increment_pc(unsigned long now, bool absolute = false, int d = 1);

public:
    ProgramState();

    void init();

    inline unsigned long elapsed(unsigned long now) const
    { return now - program_step_start_; }

    inline unsigned long halfpress_delay() const
    { return halfpress_delay_; }

    inline void set_halfpress_delay(unsigned long halfpress_delay)
    { halfpress_delay_ = halfpress_delay; }

    inline int halfpress_output() const
    { return halfpress_output_; }

    inline void set_halfpress_output(int halfpress_output)
    { halfpress_output_ = halfpress_output; }

    inline int fullpress_output() const
    { return fullpress_output_; }

    inline void set_fullpress_output(int fullpress_output)
    { fullpress_output_ = fullpress_output; }

    template <size_t MAX_STEPS>
    ProgramStatus next_step(Program<MAX_STEPS> &program) {
        if (program_counter_ < program.step_count()) {
            program.steps()[program_counter_].handle(*this);
        }

        // Detect program termination
        if (program_counter_ >= program.step_count())
            return PS_Completed;
        return PS_Continue;
    }
};
