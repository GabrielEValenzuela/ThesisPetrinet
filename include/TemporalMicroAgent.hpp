#pragma once
#include <chrono>
#include <semaphore>
#include <memory>
#include <thread>
#include <atomic>
#include <vector>
#include "Agent.hpp"

class TemporalMicroAgent : public Agent{
public:
    void execute() override;
    void executeWL() override;
    void resume() override;
    void suspend() override;
    void stop() override;
    void sleep_for(uint32_t time) override;
    void setMonitor(std::shared_ptr<Monitor> monitor) override;
    void setFireSequence(std::unique_ptr<std::vector<uint32_t>> firesequence) override;
    const std::thread::id getId() override;
    std::string getStrId() override;
private:
    std::jthread worker;
    std::stop_source source;
    std::binary_semaphore controller{0}; 
    std::atomic<bool> suspend_guard{false}; //Used for the suspend and resume
    std::atomic<bool> sleep_guard{false};
    std::unique_ptr<std::vector<uint32_t>> fire_sequence;
    std::shared_ptr<Monitor> monitor;
    std::chrono::high_resolution_clock::time_point timestamp;
    uint32_t sleep_time{1}; //Measured in miliseconds
};
