#include <Arduino.h>

#include "Config.hpp"
#include "ProgramState.hpp"

void ProgramState::increment_pc(unsigned long now, bool absolute, int d) {
    if (absolute)
        program_counter_ = d;
    else
        program_counter_ += d;

    program_step_start_ = now;
}

void ProgramState::init() {
    program_counter_ = 0;
    program_step_start_ = millis();
    halfpress_delay_ = DEFAULT_HALFPRESS_DELAY;

    halfpress_output_ = DEFAULT_HALFPRESS_OUTPUT;
    fullpress_output_ = DEFAULT_FULLPRESS_OUTPUT;
}

ProgramState::ProgramState() { init(); }

