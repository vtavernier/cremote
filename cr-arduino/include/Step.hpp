#pragma once

#include <stddef.h>
#include <stdint.h>

enum StepName {
    SN_WAIT = 'W',
    SN_PRESS = 'P',
    SN_LOOP = 'L',
    SN_SET_HALFPRESS = 'H',
};

class ProgramState;

class Step {
    // A Step is an uint32_t
    uint8_t parts_[4];
    uint16_t lc_;

   public:
    inline uint8_t *data() { return parts_; }
    inline const uint8_t *data() const { return parts_; }

    inline StepName name() const { return static_cast<StepName>(parts_[0]); }

    inline void init() {
        if (name() == SN_LOOP) {
            lc_ = static_cast<uint16_t>(parts_[1]) << 8 | static_cast<uint16_t>(parts_[2]);
        }
    }

    inline uint8_t loop_target() const { return parts_[3]; }
    inline uint16_t loop_count() const { return lc_; }
    inline bool loop_count_dec() {
        if (lc_ == 0)
            return false;
        lc_--;
        return true;
    }

    inline unsigned long step_millis() const {
        uint8_t type = parts_[1] & 0x1;
        uint16_t mul = type ? 1000 : 1;
        return mul * (static_cast<uint16_t>(parts_[2]) << 8 | static_cast<uint16_t>(parts_[3]));
    }

    void handle(ProgramState &state);
};

template <size_t MAX_STEPS>
class Program {
    Step steps_[MAX_STEPS];
    size_t step_count_;

   public:
    inline Step *steps() { return steps_; }
    inline const Step *steps() const { return steps_; }

    inline size_t step_count() const { return step_count_; }
    inline void set_step_count(size_t step_count) { step_count_ = step_count; }

    inline void init() {
        for (size_t i = 0; i < step_count_; ++i)
            steps_[i].init();
    }

    Program() : step_count_(0) { memset(steps_, 0, sizeof(steps_)); }
};

enum ProgramStatus {
    PS_Continue,
    PS_Completed,
};

class ProgramState {
    uint8_t program_counter_;
    unsigned long program_step_start_;
    unsigned long halfpress_delay_;

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
