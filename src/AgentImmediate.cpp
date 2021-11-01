#include "../include/AgentImmediate.hpp"

void AgentImmediate::execute() {
    worker = std::jthread([&]() {
        const size_t total_fire = fire_sequence->size();
        size_t idx_transition = 0;
        while (!source.get_token().stop_requested()) {
            if (idx_transition == total_fire) {
                idx_transition = 0;
            }
            if (monitor->fire_immediate(this, fire_sequence->at(idx_transition))) {
                idx_transition++;
            }
            else {
                //Auto sleep thread
                controller.acquire();
            }
        }
    });
}

std::string AgentImmediate::getStrId(){
    std::ostringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}

void AgentImmediate::resume()
{
    suspend_guard.store(false);
    controller.release();
}

void AgentImmediate::suspend()
{
    suspend_guard.store(true);
}

void AgentImmediate::setMonitor(std::shared_ptr<Monitor> monitor)
{
    this->monitor = monitor;
}

void AgentImmediate::stop()
{
    if (suspend_guard.load())
    {   
        suspend_guard.store(false);
        controller.release();
    }
    source.request_stop();
}

AgentImmediate::~AgentImmediate()
{
    stop();
}

void AgentImmediate::setFireSequence(std::unique_ptr<std::vector<uint32_t>> firesequence)
{
    this->fire_sequence = std::move(firesequence);
}

const std::thread::id AgentImmediate::getId(){
    return std::this_thread::get_id();
}