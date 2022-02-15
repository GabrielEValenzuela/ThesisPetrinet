#include "../include/TemporalHourAgent.hpp"

void TemporalHourAgent::resume() {
    suspend_guard.store(false);
    controller.release();
}

void TemporalHourAgent::suspend() {
    suspend_guard.store(true);
}

void TemporalHourAgent::sleep_for(uint32_t time) {
    sleep_time = time;
    sleep_guard.store(true);
}

void TemporalHourAgent::setMonitor(std::shared_ptr<Monitor> monitor) {
    this->monitor = monitor;
}

void TemporalHourAgent::stop()
{
    if (suspend_guard.load())
    {
        suspend_guard.store(false);
        controller.release();
    }
    source.request_stop();
}

void TemporalHourAgent::setFireSequence(std::unique_ptr<std::vector<uint32_t>> firesequence) {
    this->fire_sequence = std::move(firesequence);
}

void TemporalHourAgent::execute() {
    worker = std::jthread([&]() {
        const size_t total_fire = fire_sequence->size();
        size_t idx_transition = 0;
        while (!source.get_token().stop_requested()) {
            if (idx_transition == total_fire) {
                idx_transition = 0;
            }
            if (monitor->fireTemporal(this, fire_sequence->at(idx_transition))) {
                idx_transition++;
            }
            else {
                if (sleep_guard.load()) {
                    std::this_thread::sleep_for(std::chrono::hours(sleep_time));
                }
                else {
                    controller.acquire();
                }
            }
        }
        });
}

const std::thread::id TemporalHourAgent::getId() {
    return worker.get_id();
}

std::string TemporalHourAgent::getStrId()
{
    std::ostringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}

void TemporalHourAgent::executeWL() {
    worker = std::jthread([&]() {
        const size_t total_fire = fire_sequence->size();
        size_t idx_transition = 0;
        while (!source.get_token().stop_requested()) {
            if (idx_transition == total_fire) {
                idx_transition = 0;
            }
            if (monitor->fireTemporalWL(this, fire_sequence->at(idx_transition))) {
                idx_transition++;
            }
            else {
                if (sleep_guard.load()) {
                    std::this_thread::sleep_for(std::chrono::hours(sleep_time));
                }
                else {
                    controller.acquire();
                }
            }
        }
    });
}
