#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <execution>
#include <memory>
#include <stdexcept>
#include "Agent.hpp"

/*
    The Queue object store the pointer to all agents on an indexed way,
    each transitions has a vector of agents associate.
*/
class Agent;
using type_queue = std::unique_ptr<std::unordered_map<uint32_t, std::unique_ptr<std::vector<Agent*>>>>;
class Queue{    
private:
    type_queue queues;
    std::unique_ptr<std::vector<uint32_t>> keys;
public:
    Queue(){
        queues = std::make_unique<std::unordered_map<uint32_t, std::unique_ptr<std::vector<Agent*>>>>();
        keys   = std::make_unique<std::vector<uint32_t>>();
    }

    void addAgent(uint32_t transition,Agent* agent);
    Agent* getAgent(uint32_t transition,uint32_t agent_policy);
    std::vector<uint32_t>& transitionsWaiting();
    std::size_t getSize() {
        return queues->size();
    }
    
};