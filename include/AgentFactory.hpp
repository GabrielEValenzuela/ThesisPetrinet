#pragma once
#include <memory>
#include "Agent.hpp"
#include "Monitor.hpp"
#include "PetriNetwork.hpp"
#include "AgentImmediate.hpp"
#include "TemporalHourAgent.hpp"
#include "TemporalMinuteAgent.hpp"
#include "TemporalSecondAgent.hpp"
#include "TemporalMilliAgent.hpp"
#include "TemporalMicroAgent.hpp"
#include "TemporalNanoAgent.hpp"
#include "../lib/entityPN.hpp"

/*
    Factory Method is a creational design pattern that provides an interface for creating objects in a superclass,
    but allows subclasses to alter the type of objects that will be created.
    This is the Creator class, that means, it's declares the factory method that returns new product objects
    with the return type of this method matched with the product interface.


*/
class AgentFactory {
private:
    std::shared_ptr<Monitor> monitor;
    std::shared_ptr<PetriNetwork> petrinetwork;
public:
    AgentFactory() = default;
    AgentFactory(std::shared_ptr<Monitor> monitor, std::shared_ptr<PetriNetwork> petrinet);
    std::unique_ptr<Agent> getAgent(Timescale_Choice::unit scale, Agent_Choice::type type,
                                    std::unique_ptr<std::vector<uint32_t>> vector,
                                    uint32_t first_parameter = 0,
                                    uint32_t second_parameter = 0);
};