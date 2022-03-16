#pragma once
#include <chrono>

class TimeLogic {
public:
    [[nodiscard]] virtual int64_t isTemporalSensitized(const uint32_t alpha,const uint32_t beta,const std::chrono::high_resolution_clock::time_point &timestamp) = 0;
    [[nodiscard]] virtual bool isEarly(const uint32_t alpha,const std::chrono::high_resolution_clock::time_point &timestamp) = 0;
};