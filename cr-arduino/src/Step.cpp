#include <Arduino.h>

#include "Config.hpp"
#include "ProgramState.hpp"

void Step::handle(ProgramState &state) {
    if (this->name() == SN_PRESS) {
        auto d = this->step_micros();
        auto halfpress_delay = state.halfpress_delay();

        auto now_micros = micros();
        auto elapsed_micros = state.elapsed_micros(now_micros);

        if (elapsed_micros > d + halfpress_delay) {
            digitalWrite(state.fullpress_output(), LOW);
            digitalWrite(state.halfpress_output(), LOW);

            // Go to next step after press
            state.increment_pc();
        } else if (elapsed_micros > halfpress_delay) {
            // In case we somehow missed the first step
            digitalWrite(state.halfpress_output(), HIGH);
            digitalWrite(state.fullpress_output(), HIGH);
        } else {
            digitalWrite(state.halfpress_output(), HIGH);
        }
    } else if (this->name() == SN_WAIT) {
        if (state.elapsed(millis()) > this->step_millis()) {
            state.increment_pc();
        }
    } else if (this->name() == SN_LOOP) {
        if (this->loop_count_dec()) {
            // true: keep iterating
            state.increment_pc(true, this->loop_target());
        } else {
            // false: stop iterating
            state.increment_pc();
        }
    } else if (this->name() == SN_SET_HALFPRESS) {
        state.set_halfpress_delay(this->step_micros());
        state.increment_pc();
    } else if (this->name() == SN_SET_OUTPUTMAP) {
        state.set_halfpress_output(this->output_map_halfpress());
        state.set_fullpress_output(this->output_map_fullpress());
        state.increment_pc();
    }
}
