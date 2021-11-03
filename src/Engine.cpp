#include "../include/Engine.hpp"

void Engine::run() {
    while (true) {
        displayMenu();
        int option;
        std::cout<<">> ";
        std::cin >> option;
        switch (option) {
        case 1:{
            runSimulation();
            break;
        }

        case 2:{
            menuAlgorithms();
            break;
        }

        case 3: {
        std::cout << "Goodbye!\n";
        exit(0);
        }
        default:{
            system("clear");
            std::cout<<"Invalid option\n";
        }
        }
    }
}

void Engine::menuAlgorithms() {
    std::cout << "Petrinet SE (Simulator&Emulator) \U0001F6E0\n";
    std::cout << "\t\t\t\tVersion - " << major << "." << minor << "\n";
    std::cout << "Select algorithm to run:\n";
    std::cout << "\t 1) Run Minimal Coverability Tree\n";
    std::cout << "\t 2) Run invariants transitions\n";
    std::cout << "\t 3) Run Siphons&Traps detection\n";
    std::cout << "\t x) Return main menu\n";

    int option;
    std::cout << ">> ";
    std::cin >> option;
    switch (option) {
    case 1: {
        if(instance.get()!=nullptr){
            AlgorithmMinCov* algo = new AlgorithmMinCov();
            algo->runAlgorithm(instance);            
        } else {
            std::string file_name;
            std::cout<<"Please, introduce the filename:";
            std::cin >> file_name;

             std::regex pattern("(\\w+)(\\.)(\\w+)");
        std::smatch sm_file;
        std::regex_search(file_name,sm_file,pattern);
        if(!std::strcmp(sm_file.str(3).c_str(),"json")){
            std::ifstream f(file_name);
            if(f.bad()){
                std::cout<<"File <"<<sm_file.str(1)<<"> not found. Please try again\n";
                return;
            }
            json_file = std::make_unique<ReadJSON>();
            if (json_file->readFile(file_name) == -1) {
                std::cout << "Invalid JSON file\n";
                return;
            }
            
            EngineFactory();
            AlgorithmMinCov* algo = new AlgorithmMinCov();
            algo->runAlgorithm(instance);
        }
            
        }
    }
    case 2: {
        if (simulation_done) {
            std::cout << logger->getFireHistory() << "\n";
            AnalysisEngine::findInvariantsTransitions(logger->getFireHistory());
        }
        return;
    }
    case 3: {
        std::cout << "To be implemented\n";
        return;
    }
    default:
    {
        return;
    }
    }
}
void Engine::displayMenu() {
    std::cout << "Petrinet SE (Simulator&Emulator) \U0001F6E0\n";
    std::cout << "\t\t\t\tVersion - " << major << "." << minor << "\n";
    std::cout << "Select operation:\n";
    std::cout << "\t 1) Run simulation\n";
    std::cout << "\t 2) Run algorithm\n";
    std::cout << "\t 3) Exit\n";
}


void Engine::runSimulation(){
    std::string file_name;
    std::cout<<"Please, introduce the filename:";
    std::cin >> file_name;
    /*!
    * Testing regex
    */
    try{
        std::regex pattern("(\\w+)(\\.)(\\w+)");
        std::smatch sm_file;
        std::regex_search(file_name,sm_file,pattern);
        if(!std::strcmp(sm_file.str(3).c_str(),"json")){
            std::ifstream f(file_name);
            if(f.bad()){
                std::cout<<"File <"<<sm_file.str(1)<<"> not found. Please try again\n";
                return;
            }
            json_file = std::make_unique<ReadJSON>();
            if (json_file->readFile(file_name) == -1) {
                std::cout << "Invalid JSON file\n";
                return;
            }
            
            simulateJSON();
        }
        else if(!std::strcmp(sm_file.str(3).c_str(),"pnml")){
            std::cout<<"Is a PNML\n";
        }
        else{
            std::cout<<"Invalid file extension. Only JSON or PNLM format supported\n";
            exit(0);
        }
    }
    catch(std::regex_error& e){
        std::cout<<e.what()<<"\n";
    }
}

void Engine::EngineFactory(){
    //logger = std::make_shared<Logger>();
    switch (json_file->getNetworkType()){
    case Network_Choice::type::DISCRETE_IMMEDIATE: {
        petri_type = std::make_unique<PlaceTransitionNet>();
        petri_type->setFile(std::move(json_file));
        petri_director->buildPlaceTransitionNet(petri_type.get());
        instance = petri_type->getPetrinet();
        the_monitor = std::make_shared<Monitor>(instance);
        //the_monitor->setLogger(logger);
        //instance->setLogger(logger);
        factory = AgentFactory(the_monitor, instance);
        the_monitor->resetFireCount();
        break;
    };
    case Network_Choice::type::DISCRETE_TEMPORAL: {
        configureTimeScale();
        logger = std::make_shared<Logger>();
        petri_type = std::make_unique<TemporalPlaceTransitionNet>();
        petri_type->setFile(std::move(json_file));
        petri_director = std::make_unique<PetriDirector>();
        petri_director->buildPlaceTransitionNet(petri_type.get());
        instance = petri_type->getPetrinet();
        instance->setTimeLogic(the_timelogic);
        the_monitor = std::make_shared<Monitor>(instance);
        the_monitor->setLogger(logger);
        instance->setLogger(logger);
        factory = AgentFactory(the_monitor, instance);
        the_monitor->resetFireCount();
        break;
    };
    default:
        break;
    }

}

void Engine::displaySimulateResult() {
    std::cout << "Simulation finished\nLoading results...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Total fire count " << the_monitor->getFireCount() << "\n";
    std::cout << "Total fire expected " << max_fires << "\n";
    double err{ static_cast<double>((the_monitor->getFireCount() - max_fires)) / max_fires };
    std::cout << "Error " << std::setprecision(2) << err * 100 << "% \n";
    std::cout << "Initial mark [ ";
    for (auto m = 0; m < instance->getInitialMark().size(); m++) {
        std::cout << instance->getInitialMark().at(m) << " ";
    }
    std::cout << "]\n";
    std::cout << "Final mark [ ";
    for (auto m = 0; m < instance->getMark()->size(); m++) {
        std::cout << instance->getMark()->at(m) << " ";
    }
    std::cout << "]\n";
    simulation_done = true;
}

void Engine::simulateJSON(){
    EngineFactory();
    config_pool();
    the_monitor->setTotalAgents(pool_agent.size());
    auto start = std::chrono::high_resolution_clock::now();
    for (auto& agent : pool_agent) {
        agent->execute();
    }
    float progress = 0.0;
    int previous_count = 0;
    while (the_monitor->getFireCount() < max_fires && the_monitor->stillSensitized()) {
        previous_count = the_monitor->getFireCount();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        if (previous_count == the_monitor->getFireCount()) {
            break;
        }
    }
    for (auto& agent : pool_agent) {
        agent->stop();
    }
    //if (the_monitor->hasBlock()) {
      //  std::cout << "Block on Monitor...\n";
        //logger->error("Blocked Monitor");
    //}
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "\n===================\n";
    std::cout << "Simulation finished\nLoading results...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Total fire count "<< the_monitor->getFireCount() << "\n";
    std::cout << "Total fire expected " << max_fires << "\n";
    double err{ static_cast<double>((the_monitor->getFireCount() - max_fires)) / max_fires };
    std::cout << "Error " <<std::setprecision(2)<< err *100<< "% \n";
    std::cout << "Total simulation time [ms] " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "\n";
    std::cout << "Initial mark [ ";
    for (auto m = 0; m < instance->getInitialMark().size(); m++) {
        std::cout << instance->getInitialMark().at(m) << " ";
    }
    std::cout << "]\n";
    std::cout << "Final mark [ ";
    for (auto m = 0; m < instance->getMark()->size(); m++) {
        std::cout << instance->getMark()->at(m) << " ";
    }
    std::cout << "]\n";
    //displaySimulateResult();
    simulation_done = true;
}

void Engine::configureTimeScale()
{
    scale_time = json_file->getTimeScale();
    switch (json_file->getTimeScale())
    {
    case Timescale_Choice::unit::NANOSECOND: {
        the_timelogic = std::make_shared<NanoTimeLogic>();
        break;
    }
    case Timescale_Choice::unit::MICROSECOND: {
        the_timelogic = std::make_shared<MicrTimeLogic>();
        break;
    }
    case Timescale_Choice::unit::MILLISECOND: {
        the_timelogic = std::make_shared<MilliTimeLogic>();
        break;
    }
    case Timescale_Choice::unit::SECOND: {
        the_timelogic = std::make_shared<SecoTimeLogic>();
        break;
    }
    case Timescale_Choice::unit::MINUTE: {
        the_timelogic = std::make_shared<MinuTimeLogic>();
        break;
    }
    case Timescale_Choice::unit::HOUR: {
        the_timelogic = std::make_shared<HourTimeLogic>();
        break;
    }
 }
}

void Engine::config_pool() {
    pool_agent.clear();
    json config_json;
    std::string config_file_name;
    std::cout<<"Please, enter a valid config file: ";
    std::cin >> config_file_name;
    std::ifstream file(config_file_name);
    if (file.bad()) {
        std::cout << "Failed open config file\n";
        return;
    }
    config_json = json::parse(file);
    if (!config_json["firesequence_agents"].is_null()) {
        auto sequences = config_json["firesequence_agents"];
        for (auto& seq : sequences) {
            std::vector<uint32_t> fire = seq["sequence"];
            if (seq["type"].get<std::string>().compare("immediate")==0) {
                pool_agent.push_back(std::move(factory.getAgent(scale_time,Agent_Choice::type::Immediate, std::make_unique<std::vector<uint32_t>>(fire))));
            }
            else {
                pool_agent.push_back(std::move(factory.getAgent(scale_time,Agent_Choice::type::Temporal, std::make_unique<std::vector<uint32_t>>(fire))));
            }
        }
    }
    else {
        std::cout << "Not fire sequence provided, please try again \n";
        return;
    }
    if (!config_json["max_fire"].is_null()) {
        max_fires = config_json["max_fire"].get<uint64_t>();
    } else {
        max_fires = 100;
    }
}