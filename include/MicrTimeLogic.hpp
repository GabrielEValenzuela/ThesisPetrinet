#pragma once
#include "TimeLogic.hpp"

class MicrTimeLogic : public TimeLogic
{
public:
    [[nodiscard]] int64_t isTemporalSensitized(const uint32_t alpha, const uint32_t beta, const std::chrono::high_resolution_clock::time_point& timestamp) override {
        auto now = std::chrono::high_resolution_clock::now();
        auto scaled_time = std::chrono::duration_cast<std::chrono::microseconds>(now-timestamp).count();
        if((alpha<=scaled_time) && (scaled_time<=beta)){
            return 0;
        } else {
            return std::chrono::duration_cast<std::chrono::microseconds>(timestamp - now).count() + alpha;
        }
    }

    [[nodiscard]] bool isEarly(const uint32_t alpha,const std::chrono::high_resolution_clock::time_point &timestamp) override{
        auto now = std::chrono::high_resolution_clock::now();
        auto scaled_time = std::chrono::duration_cast<std::chrono::microseconds>(now-timestamp).count();
        return scaled_time < alpha;
    }
};
