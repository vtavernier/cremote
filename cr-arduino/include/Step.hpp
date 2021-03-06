#pragma once

#include <stddef.h>
#include <stdint.h>

enum StepName {
    SN_WAIT = 'W',
    SN_PRESS = 'P',
    SN_LOOP = 'L',
    SN_SET_HALFPRESS = 'H',
    SN_SET_OUTPUTMAP = 'O',
};

enum DurationType {
    DT_MILLIS = 0,
    DT_SECONDS = 1,
    DT_INVERSE_SECONDS = 2,
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
        if (lc_ == 0) {
            // Reset loop counter to allow nested loops
            init();
            return false;
        }

        lc_--;
        return true;
    }

    inline unsigned long step_millis() const {
        uint8_t type = parts_[1] & 0x3;
        unsigned long amount = (static_cast<uint16_t>(parts_[2]) << 8 | static_cast<uint16_t>(parts_[3]));

        if (type == DT_MILLIS)
            return amount;
        else if (type == DT_SECONDS)
            return 1000 * amount;
        else if (type == DT_INVERSE_SECONDS)
            return 1000 / amount;
        else
            return 0;
    }

    inline unsigned long step_micros() const {
        uint8_t type = parts_[1] & 0x3;
        unsigned long amount = (static_cast<uint16_t>(parts_[2]) << 8 | static_cast<uint16_t>(parts_[3]));

        if (type == DT_MILLIS)
            return 1000 * amount;
        else if (type == DT_SECONDS)
            return 1000000 * amount;
        else if (type == DT_INVERSE_SECONDS)
            return 1000000 / amount;
        else
            return 0;
    }

    inline int output_map_halfpress() const { return parts_[1]; }
    inline int output_map_fullpress() const { return parts_[2]; }

    void handle(ProgramState &state);
};
