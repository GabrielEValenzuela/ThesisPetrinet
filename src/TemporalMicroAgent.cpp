#include "../include/TemporalMicroAgent.hpp"

void TemporalMicroAgent::resume() {
    suspend_guard.store(false);
    controller.release();
}

void TemporalMicroAgent::suspend() {
    suspend_guard.store(true);
}

void TemporalMicroAgent::sleep_for(uint32_t time) {
    sleep_time = time;
    sleep_guard.store(true);
}

void TemporalMicroAgent::setMonitor(std::shared_ptr<Monitor> monitor) {
    this->monitor = monitor;
}

void TemporalMicroAgent::stop()
{
    if (suspend_guard.load())
    {
        suspend_guard.store(false);
        controller.release();
    }
    source.request_stop();
}

void TemporalMicroAgent::setFireSequence(std::unique_ptr<std::vector<uint32_t>> firesequence) {
    this->fire_sequence = std::move(firesequence);
}

void TemporalMicroAgent::execute() {
    worker = std::jthread([&]() {
        const size_t total_fire = fire_sequence->size();
        size_t idx_transition = 0;
        while (!source.get_token().stop_requested()) {
            if (idx_transition == total_fire) {
                idx_transition = 0;
            }
            if (monitor->fire_temporal(this, fire_sequence->at(idx_transition))) {
                idx_transition++;
            }
            else {
                if (sleep_guard.load()) {
                    std::this_thread::sleep_for(std::chrono::microseconds(sleep_time));
                }
                else {
                    controller.acquire();
                }
            }
        }
        });
}

const std::thread::id TemporalMicroAgent::getId() {
    return std::this_thread::get_id();
}

std::string TemporalMicroAgent::getStrId()
{
    std::ostringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}
