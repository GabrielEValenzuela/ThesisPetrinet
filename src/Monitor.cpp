#include "../include/Monitor.hpp"


Monitor::Monitor(std::shared_ptr<PetriNetwork> petri_network) {
    petri_instance = petri_network;
    temporal_filter = std::make_unique<std::unordered_map<uint32_t, std::thread::id>>();
    immediate_queue = std::make_unique<Queue>();
    temporal_queue = std::make_unique<Queue>();
    sensitized_and_available = std::make_unique<std::vector<bool>>(petri_instance->getTransitions());
    hodor = std::make_unique<std::binary_semaphore>(1);

}

bool Monitor::fire_immediate(Agent* agent, uint32_t transition) {
    #ifdef PROFILING_ENABLE
    ZoneScopedN("ImmediateEntry");
    TracyMessageL("Agent try to acquire semaphore");
    hodor->acquire();
    std::unique_lock<LockableBase(std::mutex)> guard(mutex);
    #else
    hodor->acquire();
    std::unique_lock<std::mutex> guard(mutex);
    #endif
    #ifdef PROFILING_ENABLE
    TracyMessageL("Agent acquire semaphore and mutex succesfully");
    #endif
    try
    {
        
        if (petri_instance->isSensitized(transition)) {
            #ifdef PROFILING_ENABLE
            TracyMessageL("Agent is enable to fire transition");
            #endif
            //logger->record("Agent #" + agent->getStrId() + " will fire transition number " + std::to_string(transition) + "\n");
            //logger->addFire(transition);
            auto new_mark = std::make_unique<std::vector<uint32_t>>(petri_instance->getMark()->size());
            MathEngine::fire(petri_instance->getMark(), petri_instance->getRow(transition), new_mark.get());
            //std::cout << "Fire " << transition << "\n";
            petri_instance->setMark(std::move(new_mark));
            petri_instance->updateSensitized();
            fire_counter.fetch_add(1);
            wake_up();
            guard.unlock();
            hodor->release();
            #ifdef PROFILING_ENABLE
            TracyMessageL("Agent leaves monitor after fire transition");
            #endif
            return true;

        }
        else {
            #ifdef PROFILING_ENABLE
            TracyMessageL("Agent is not enable to fire transition");
            #endif
            //logger->record("Agent #" + agent->getStrId() + " go to queue to wait for transition " + std::to_string(transition) + "\n");
            //logger->record("Immediate queue size " + std::to_string(immediate_queue->getSize()) + "\n");
            checkAndUnlock();
            guard.unlock();
            immediate_queue->addAgent(transition, agent);
            hodor->release();
            #ifdef PROFILING_ENABLE
            TracyMessageL("Agent leaves monitor after put itself in queue");
            #endif
            return false;
        }
    }
    catch (const std::exception& ex) {
        std::cout << "***ERROR***\t" << ex.what() << "\n";
        exit(-1);
    }
}

#ifdef PROFILING_ENABLE
ZoneScopedN("TemporalEntry");
#endif
bool Monitor::fire_temporal(Agent* agent, uint32_t transition) {
    #ifdef PROFILING_ENABLE
    TracyMessageL("Agent try to acquire semaphore");
    hodor->acquire();
    std::unique_lock<LockableBase(std::mutex)> guard(mutex);
    #else
    hodor->acquire();
    std::unique_lock<std::mutex> guard(mutex);
    #endif
    #ifdef PROFILING_ENABLE
    TracyMessageL("Agent acquire semaphore and mutex succesfully");
    #endif
    try {
        if (petri_instance->isSensitized(transition)) {
            #ifdef PROFILING_ENABLE
            TracyMessageL("Agent is sensitized to fire transition");
            #endif
            //logger->record("Temporal agent #" + agent->getStrId() + " can fire transition " + std::to_string(transition) + "\n");
            //Is there another thread trying to fire the same transition ?
            if (!isAnotherFirst(transition, agent)) {
                #ifdef PROFILING_ENABLE
                TracyMessageL("Agent is first to fire transition");
                #endif
                //logger->record("Temporal agent #" + agent->getStrId() + " is the first to fire transition number " + std::to_string(transition) + "\n");
                //If not, Am I inside temporal window ?
                auto inside = petri_instance->isTemporalSensitized(transition);
                //logger->record("Temporal agent #" + agent->getStrId() + " have a difference of " + std::to_string(inside) + "[TU]\n");
                if (!inside) {
                    #ifdef PROFILING_ENABLE
                    TracyMessageL("Agent is inside temporal window to fire transition");
                    #endif
                    //I'm inside window, I'm the first agent trying to fire the transition
                    // and I'm sensitized. I'll make the fire
                    //logger->record("Temporal agent #" + agent->getStrId() + " is inside temporal window to fire transition number " + std::to_string(transition) + "\n");
                    /*
                    * Dos cosas:
                    *   -> Problema: Resulta que cuando hago el sleep, volvio a entrar, sino esta anotado
                    *                no puede disparar.[Check]
                    *   -> Hacer test: Sin tiempo. Despues con tiempo.
                    *   LOG:
                    *           Hilo que dispara
                    *           Instante de tiempo
                    *           Instange de dormir
                    */
                    //logger->record("Temporal agent #" + agent->getStrId() + " will fire transition number " + std::to_string(transition) + "\n");
                    //logger->addFire(transition);
                    auto new_mark = std::make_unique<std::vector<uint32_t>>(petri_instance->getMark()->size());
                    MathEngine::fire(petri_instance->getMark(), petri_instance->getRow(transition), new_mark.get());
                    petri_instance->setMark(std::move(new_mark));
                    petri_instance->updateSensitized();
                    fire_counter.fetch_add(1);
                    temporal_filter->erase(transition);
                    wake_up();
                    guard.unlock();
                    hodor->release();
                    #ifdef PROFILING_ENABLE
                    TracyMessageL("Agent leaves monitor after fire temporal transition");
                    #endif
                    return true;
                }
                else {
                    //I'm the first, I'm sensitized but I'm not inside window...
                    //So, I have two choices:
                    //If I'm early, i.e. before alpha time, I go to sleep the remaining time...
                    if (inside > 0) {
                        //logger->record("Temporal agent #" + agent->getStrId() + " is early fire transition number because time difference is positive" + std::to_string(transition) + "\n");
                        //logger->record("Temporal agent #" + agent->getStrId() + " will sleep for " + std::to_string(inside) + "[UT]\n");
                        agent->sleep_for(inside);
                        guard.unlock();
                        hodor->release();
                        #ifdef PROFILING_ENABLE
                        TracyMessageL("Agent leaves monitor because is early to fire transition");
                        #endif
                        return false;
                    }
                }
            }
        }
        //logger->record("Temporal agent #" + agent->getStrId() + " go to queue to wait for transition " + std::to_string(transition) + "\n");
        //logger->record("Temporal queue size " + std::to_string(temporal_queue->getSize()) + "\n");
        checkAndUnlock();
        guard.unlock();
        hodor->release();
        temporal_queue->addAgent(transition, agent);
        #ifdef PROFILING_ENABLE
        TracyMessageL("Agent leaves monitor after puts itself in queue");
        #endif
        return false;
    }
    catch (const std::exception& ex) {
        std::cout << "***ERROR***\t" << ex.what() << "\n";
        exit(-1);
    }
    return false;
}

uint64_t Monitor::getFireCount()
{
    return fire_counter.load();
}

void Monitor::resetFireCount()
{
    fire_counter = 0;
}

bool Monitor::stillSensitized() {
    return petri_instance->stillSensitized();
}

void Monitor::checkAndUnlock() {
    if (immediate_queue->getSize() + temporal_queue->getSize() == (total_agents - 1)) {
        //std::cout << "Monitor block... unblocking\n";
        //petri_instance->updateSensitized();
        wake_up();
    }

}

bool Monitor::isAnotherFirst(uint32_t transition, Agent* agent) const {
    try
    {
        //There is an temporal agent record for a specific transition
        //Verify if is the same agent...
        if (temporal_filter->at(transition) == agent->getId()) {
            return false;
        }
        else {
            return true;
        }
    }
    catch (const std::out_of_range& e)
    {
        //No record for the transition, that means I'M FIRST
        //I made a record
        temporal_filter->insert({ transition,agent->getId() });
        return false;
    }

}

void Monitor::setLogger(std::shared_ptr<Logger> logger) {
    this->logger = logger;
}

void Monitor::setTotalAgents(uint64_t total)
{
    total_agents = total;
}

bool Monitor::hasBlock()
{
    return (immediate_queue->getSize()+temporal_queue->getSize())==total_agents;
}

void Monitor::wake_up() {
    auto waiting = immediate_queue->transitionsWaiting();
    for (auto wait_number = 0; wait_number < waiting.size(); wait_number++) {
        if (petri_instance->isSensitized(waiting.at(wait_number))) {
            auto awake = immediate_queue->getAgent(waiting.at(wait_number), 0);
            if (awake != nullptr) {
                //auto stamp = time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now());
                //log_file << date::format("%T", stamp)<< " - Agent #" << awake->getId() << " is awake to fire " << waiting.at(wait_number) << " successfully \n";
                //logger->record("Agent #" + awake->getStrId() + " wake up to fire " + std::to_string(waiting.at(wait_number)) + "\n");
                awake->resume();
                return;
            }
        }
    }
    waiting = temporal_queue->transitionsWaiting();
    for (auto wait_number = 0; wait_number < waiting.size(); wait_number++) {
        if (petri_instance->isSensitized(waiting.at(wait_number))) {
            auto awake = temporal_queue->getAgent(waiting.at(wait_number), 0);
            if (awake != nullptr) {
                //auto stamp = time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now());
                //log_file << date::format("%T", stamp)<< " - Agent #" << awake->getId() << " is awake to fire " << waiting.at(wait_number) << " successfully \n";
                //logger->record("Temporal agent #" + awake->getStrId() + " wake up to fire " + std::to_string(waiting.at(wait_number)) + "\n");
                awake->resume();

                return;
            }
        }
    }
    //logger->record("No agents immediate neiter temporal to wake up. Queues size (I:" + std::to_string(immediate_queue->getSize()) + "/T:" + std::to_string(temporal_queue->getSize()) + ")\n");
}