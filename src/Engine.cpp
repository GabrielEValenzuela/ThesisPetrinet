#include "../include/Engine.hpp"

void Engine::run(std::string &&filename, uint8_t operation)
{
    auto file_name = std::move(filename);
    if (!validateFilename(file_name))
    {
        std::cout << "*** ERROR ***\t Invalid file\n";
        return;
    }
    switch (operation)
    {
    case 0:
        runSimulation();
        break;
    case 1:
        runAlgorithm();
        break;
    }
}

bool Engine::validateFilename(std::string &filename)
{
    try
    {
        std::regex pattern("(\\w+)(\\.)(\\w+)");
        std::smatch sm_file;
        std::regex_search(filename, sm_file, pattern);
        if (!std::strcmp(sm_file.str(3).c_str(), "json"))
        {
            std::ifstream f(filename);
            if (f.bad())
            {
                std::cout << "File <" << sm_file.str(1) << "> not found. Please try again\n";
                return false;
            }
            json_file = std::make_unique<ReadJSON>();
            if (json_file->readFile(filename) == -1)
            {
                std::cout << "Invalid JSON file\n";
                return false;
            }
            return true;
        }
        else if (!std::strcmp(sm_file.str(3).c_str(), "pnml"))
        {
            std::ifstream f(filename);
            if (f.bad())
            {
                std::cout << "File <" << sm_file.str(1) << "> not found. Please try again\n";
                return false;
            }
            pnml_file = std::make_unique<ReadXML>();
            if (pnml_file->readFile(filename) == -1)
            {
                std::cout << "Invalid PNML file\n";
                return false;
            }
            json_pnml = false;
            return true;
        }
    }
    catch (std::regex_error &e)
    {
        std::cout << e.what() << "\n";
    }
    return false;
}

void Engine::runSimulation()
{
    //Setup all components.
    simulationFactory();

    the_monitor->setTotalAgents(pool_agent.size());
    auto start = std::chrono::high_resolution_clock::now();
    for (auto &agent : pool_agent)
    {
        agent->execute();
    }
    float progress = 0.0;
    int previous_count = 0;
    while (the_monitor->getFireCount() < max_fires && the_monitor->stillSensitized())
    {
        previous_count = the_monitor->getFireCount();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        if (previous_count == the_monitor->getFireCount())
        {
            break;
        }
    }
    for (auto &agent : pool_agent)
    {
        agent->stop();
    }

    auto end = std::chrono::high_resolution_clock::now();

    // Show simulation result
    std::cout << "\n===================\n";
    std::cout << "Simulation finished\nLoading results...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Total fire count " << the_monitor->getFireCount() << "\n";
    std::cout << "Total fire expected " << max_fires << "\n";
    double err{static_cast<double>((the_monitor->getFireCount() - max_fires)) / max_fires};
    std::cout << "Error " << std::setprecision(2) << err * 100 << "% \n";
    std::cout << "Total simulation time [ms] " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "\n";
    std::cout << "Initial mark [ ";
    for (auto m = 0; m < instance->getInitialMark().size(); m++)
    {
        std::cout << instance->getInitialMark().at(m) << " ";
    }
    std::cout << "]\n";
    std::cout << "Final mark [ ";
    for (auto m = 0; m < instance->getMark()->size(); m++)
    {
        std::cout << instance->getMark()->at(m) << " ";
    }
    std::cout << "]\n";
}

void Engine::petriFactory(){
    if (json_pnml)
    {
        switch (json_file->getNetworkType())
        {
        case Network_Choice::type::DISCRETE_IMMEDIATE:
        {
            petri_type = std::make_unique<PlaceTransitionNet>();
            petri_type->setFile(std::move(json_file));
            petri_director->buildPlaceTransitionNet(petri_type.get());
            instance = petri_type->getPetrinet();
            break;
        };
        case Network_Choice::type::DISCRETE_TEMPORAL:
        {
            configureTimeScale();
            petri_type = std::make_unique<TemporalPlaceTransitionNet>();
            petri_type->setFile(std::move(json_file));
            petri_director = std::make_unique<PetriDirector>();
            petri_director->buildPlaceTransitionNet(petri_type.get());
            instance = petri_type->getPetrinet();
            instance->setTimeLogic(the_timelogic);
            break;
        };
        default:
            break;
        }
    }
    else
    {
        switch (pnml_file->getNetworkType())
        {
        case Network_Choice::type::DISCRETE_IMMEDIATE:
        {
            petri_type = std::make_unique<PlaceTransitionNet>();
            petri_type->setFile(std::move(json_file));
            petri_director->buildPlaceTransitionNet(petri_type.get());
            instance = petri_type->getPetrinet();
            break;
        };
        case Network_Choice::type::DISCRETE_TEMPORAL:
        {
            configureTimeScale();
            petri_type = std::make_unique<TemporalPlaceTransitionNet>();
            petri_type->setFile(std::move(json_file));
            petri_director = std::make_unique<PetriDirector>();
            petri_director->buildPlaceTransitionNet(petri_type.get());
            instance = petri_type->getPetrinet();
            instance->setTimeLogic(the_timelogic);
            break;
        };
        default:
            break;
        }
    }
}

void Engine::simulationFactory()
{
    petriFactory();
    the_monitor = std::make_shared<Monitor>(instance);
    factory = AgentFactory(the_monitor, instance);
    the_monitor->resetFireCount();
    pool_agent.clear();
    json config_json;
    std::string config_file_name;
    std::cout << "Please, enter a valid config file: ";
    std::cin >> config_file_name;
    std::ifstream file(config_file_name);
    if (file.bad())
    {
        std::cout << "Failed open config file. Exiting...\n";
        exit(0);
    }
    config_json = json::parse(file);
    if (!config_json["firesequence_agents"].is_null())
    {
        auto sequences = config_json["firesequence_agents"];
        for (auto &seq : sequences)
        {
            std::vector<uint32_t> fire = seq["sequence"];
            if (seq["type"].get<std::string>().compare("immediate") == 0)
            {
                pool_agent.push_back(std::move(factory.getAgent(scale_time, Agent_Choice::type::Immediate, std::make_unique<std::vector<uint32_t>>(fire))));
            }
            else
            {
                pool_agent.push_back(std::move(factory.getAgent(scale_time, Agent_Choice::type::Temporal, std::make_unique<std::vector<uint32_t>>(fire))));
            }
        }
    }
    else
    {
        std::cout << "Bad JSON file. Please verify config file.\n";
        exit(0);
    }
    if (!config_json["max_fire"].is_null())
    {
        max_fires = config_json["max_fire"].get<uint64_t>();
    }
    else
    {
        max_fires = 100;
    }
}

void Engine::configureTimeScale()
{
    scale_time = json_file->getTimeScale();
    switch (json_file->getTimeScale())
    {
    case Timescale_Choice::unit::NANOSECOND:
    {
        the_timelogic = std::make_shared<NanoTimeLogic>();
        break;
    }
    case Timescale_Choice::unit::MICROSECOND:
    {
        the_timelogic = std::make_shared<MicrTimeLogic>();
        break;
    }
    case Timescale_Choice::unit::MILLISECOND:
    {
        the_timelogic = std::make_shared<MilliTimeLogic>();
        break;
    }
    case Timescale_Choice::unit::SECOND:
    {
        the_timelogic = std::make_shared<SecoTimeLogic>();
        break;
    }
    case Timescale_Choice::unit::MINUTE:
    {
        the_timelogic = std::make_shared<MinuTimeLogic>();
        break;
    }
    case Timescale_Choice::unit::HOUR:
    {
        the_timelogic = std::make_shared<HourTimeLogic>();
        break;
    }
    }
}

void Engine::runAlgorithm(){
    petriFactory();
    auto mincov = std::make_unique<AlgorithmMinCov>();
    mincov->runAlgorithm(instance);
}