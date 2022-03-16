
#pragma once
#include <semaphore>
#include <memory>
#include <thread>
#include <atomic>
#include <vector>
#include "Agent.hpp"
#include "Monitor.hpp"

/*
    Implementation of an specific agent type.
    This agent has the objective to fire immediate transitions only.
*/
class AgentImmediate : public Agent{
public:
    //~AgentImmediate();
    void execute() override;
    void executeWL() override;
    void resume() override;
    void suspend() override;
    void stop() override;
    void setMonitor(std::shared_ptr<Monitor> monitor) override;
    void setFireSequence(std::unique_ptr<std::vector<uint32_t>> firesequence) override;
    const std::thread::id getId() override;
    std::string getStrId() override;
private:
    std::jthread worker;
    std::stop_source source;
    std::binary_semaphore controller{0}; 
    std::atomic<bool> suspend_guard{false}; //Used for the suspend and resume
    std::unique_ptr<std::vector<uint32_t>> fire_sequence;
    std::shared_ptr<Monitor> monitor;
};
