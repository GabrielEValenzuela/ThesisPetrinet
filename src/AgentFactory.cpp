#include "../include/AgentFactory.hpp"

AgentFactory::AgentFactory(std::shared_ptr<Monitor> monitor, std::shared_ptr<PetriNetwork> petrinet)
{
    this->monitor = monitor;
    this->petrinetwork = petrinet;
};

std::unique_ptr<Agent> AgentFactory::getAgent(Timescale_Choice::unit scale, Agent_Choice::type type,std::unique_ptr<std::vector<uint32_t>> vector,uint32_t first_parameter,uint32_t second_parameter) {
    switch (type) {
    case Agent_Choice::type::Immediate: {
            std::unique_ptr<Agent> agent = std::make_unique<AgentImmediate>();
            agent->setMonitor(monitor);
            agent->setFireSequence(std::move(vector));
            return agent;
        }
        case Agent_Choice::type::Temporal: {
            std::unique_ptr<Agent> agent;
            switch (scale)
            {
            case Timescale_Choice::unit::NANOSECOND: {
                agent = std::make_unique<TemporalNanoAgent>();
                break; }
            case Timescale_Choice::unit::MICROSECOND: {
                agent = std::make_unique<TemporalMicroAgent>();
                break; }
            case Timescale_Choice::unit::MILLISECOND: {
                agent = std::make_unique<TemporalMilliAgent>();
                break; }
            case Timescale_Choice::unit::SECOND: {
                agent = std::make_unique<TemporalSecondAgent>();
                break; }
            case Timescale_Choice::unit::MINUTE: {
                agent = std::make_unique<TemporalMinuteAgent>();
                break;
            }
            case Timescale_Choice::unit::HOUR: {
                agent = std::make_unique<TemporalHourAgent>();
                break;
            }
            default: {
                std::cout << "Error. Invalid time scale\n";
                exit(-1);
                }
            }
            agent->setMonitor(monitor);
            agent->setFireSequence(std::move(vector));
            return agent;
        }
    }
    return nullptr;
}