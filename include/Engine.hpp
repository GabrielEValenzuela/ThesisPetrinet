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
#include "AlgorithmMinCov.hpp"
#include "OutputParser.hpp"
#include "../lib/util.hpp"
#include "../lib/AnalysisEngine.hpp"
#include "../lib/json.hpp"

/*
* Main class used to control the entire program
* The engine it's the entry point to simulate o run algorithms of Petrinetworks
*/
class Engine{
    private:
        std::unique_ptr<ReadJSON> json_file;
        std::unique_ptr<ReadXML>  pnml_file;
        uint64_t max_fires{ 0 };
        std::shared_ptr<Monitor> the_monitor;
        std::shared_ptr<PetriNetwork> instance;
        std::shared_ptr<TimeLogic> the_timelogic;
        std::unique_ptr<PetriBuilder> petri_type;
        std::vector<std::unique_ptr<Agent>> pool_agent;
        std::unique_ptr<PetriDirector> petri_director;
        Timescale_Choice::unit scale_time;
        AgentFactory factory;
        bool json_pnml{true};
        void simulationFactory();
        void petriFactory();
        void configureTimeScale();
        bool validateFilename(std::string& filename);
        void runSimulation();
        void runAlgorithm();
    public:
        void run(std::string&& filename,uint8_t operation);
};