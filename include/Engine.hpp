#pragma once
#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <regex>
#include <fstream>
#include "PetriDirector.hpp"
#include "PetriBuilder.hpp"
#include "ReadJSON.hpp"
#include "AgentFactory.hpp"
#include "ReadXML.hpp"
#include "PlaceTransitionNet.hpp"
#include "TemporalPlaceTransitionNet.hpp"
#include "PetriNetwork.hpp"
#include "Monitor.hpp"
#include "Logger.hpp"
#include "TimeLogic.hpp"
#include "NanoTimeLogic.hpp"
#include "MicrTimeLogic.hpp"
#include "MilliTimeLogic.hpp"
#include "SecoTimeLogic.hpp"
#include "MinuTimeLogic.hpp"
#include "HourTimeLogic.hpp"
#include "../lib/util.hpp"
#include "../lib/AnalysisEngine.hpp"
#include "../lib/json.hpp"

/*
* Main class used to control the entire program
* The engine it's the entry point to simulate o run algorithms of Petrinetworks
*/
class Engine{
    private:
        void EngineFactory();
        std::unique_ptr<ReadJSON> json_file;
        void simulateJSON();
        void displaySimulateResult();
        void configureTimeScale();
        void confinAndRun();
        void displayMenu();
        const uint16_t major = 2;
        const uint16_t minor = 0;
        uint64_t max_fires{ 0 };
        std::shared_ptr<Monitor> the_monitor;
        std::shared_ptr<PetriNetwork> instance;
        std::shared_ptr<TimeLogic> the_timelogic;
        std::shared_ptr<Logger> logger;
        std::unique_ptr<PetriBuilder> petri_type;
        std::vector<std::unique_ptr<Agent>> pool_agent;
        std::unique_ptr<PetriDirector> petri_director;
        Timescale_Choice::unit scale_time;
        AgentFactory factory;
        void config_pool();
        bool simulation_done;
        void menuAlgorithms();
    public:
        void runSimulation();
        void run();
};