#pragma once

#include "Step.hpp"

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
