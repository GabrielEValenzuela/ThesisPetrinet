#pragma once
#include <mutex>
#include <chrono>
#include <semaphore>
#include <thread>
#include <random>
#include <unordered_map>
#include <algorithm>
#include <string>
#include "PetriNetwork.hpp"
#include "Agent.hpp"
#include "Queue.hpp"
#include "Logger.hpp"
#include "../lib/MathEngine.hpp"
#define PROFILING_ENABLE 0
#ifdef PROFILING_ENABLE
#include "../lib/tracy/Tracy.hpp"
#endif

/*
    The Monitor class is the main resposable of the program's concurrency
    It's design is based on the Monitor object design pattern.
*/
class PetriNetwork;
class Queue;
class Agent;



class Monitor {
private:
    std::atomic<uint64_t> fire_counter{ 0 };
    int32_t test_counter{ 0 };
    #ifdef PROFILING_ENABLE
    TracyLockableN(std::mutex,mutex,"Mutex's monitor both entries");
    #else
    std::mutex mutex;
    #endif
    std::unique_ptr<std::binary_semaphore> hodor;
    std::shared_ptr<PetriNetwork> petri_instance;
    std::shared_ptr<Logger> logger;
    std::unique_ptr<Queue> immediate_queue;
    std::unique_ptr<Queue> temporal_queue;
    std::unique_ptr<std::unordered_map<uint32_t, std::thread::id>> temporal_filter;
    std::unique_ptr<std::vector<bool>> sensitized_and_available;
    uint64_t total_agents = { 0 };
    //Hacer un OR con la sensibilidad

    /*
        For a transition, who an agent try to fire, check if another agent came before
        @param transition number
        @param agent pointer
    */
    bool isAnotherFirst(uint32_t transition,Agent* agent) const;

    /*
        Pop one agent from a queue and resume his task based on a particular policy
    */
    void wake_up();
public:
    Monitor(std::shared_ptr<PetriNetwork> petri_network);
    bool fire_immediate(Agent* agent,uint32_t transition);
    bool fire_temporal(Agent* agent,uint32_t transition);
    uint64_t getFireCount();
    void resetFireCount();
    void setLogger(std::shared_ptr<Logger> logger);
    void setTotalAgents(uint64_t total);
    bool hasBlock();
    bool stillSensitized();
    void checkAndUnlock();
};