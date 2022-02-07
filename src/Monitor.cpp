#include <iomanip>
#include "../include/Monitor.hpp"


Monitor::Monitor(std::shared_ptr<PetriNetwork> petri_network) {
    petri_instance = petri_network;
    temporal_filter = std::make_unique<std::unordered_map<uint32_t, std::thread::id>>();
    immediate_queue = std::make_unique<Queue>();
    temporal_queue = std::make_unique<Queue>();
    hodor = std::make_unique<std::binary_semaphore>(1);
    start = std::chrono::system_clock::now();
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
            logger::record r = {.transition=transition,.type="immediate",.action="fire",.timestamp=generateTimestamp(),.thread_id=agent->getStrId()};
            logger.emplace_back(r);
            auto new_mark = std::make_unique<std::vector<uint32_t>>(petri_instance->getMark()->size());
            MathEngine::fire(petri_instance->getMark(), petri_instance->getRow(transition), new_mark.get());
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
            logger::record r = {.transition=transition,.type="immediate",.action="sleep",.timestamp=generateTimestamp(),.thread_id=agent->getStrId()};
            logger.emplace_back(r);
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

bool Monitor::fire_temporal(Agent* agent, uint32_t transition) {
    #ifdef PROFILING_ENABLE
    ZoneScopedN("TemporalEntry");
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
            //Is there another thread trying to fire the same transition ?
            if (!isAnotherFirst(transition, agent)) {
                #ifdef PROFILING_ENABLE
                TracyMessageL("Agent is first to fire transition");
                #endif
                //If not, Am I inside temporal window ?
                auto inside = petri_instance->isTemporalSensitized(transition);
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
                    logger::record r = {.transition=transition,.type="temporal",.action="fire",.timestamp=generateTimestamp(),.thread_id=agent->getStrId()};
                    logger.emplace_back(r);
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
                        logger::record r = {.transition=transition,.type="temporal",.action="sleep",.timestamp=generateTimestamp(),.thread_id=agent->getStrId()};
                        logger.emplace_back(r);
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
        logger::record r = {.transition=transition,.type="temporal",.action="wait",.timestamp=generateTimestamp(),.thread_id=agent->getStrId()};
        logger.emplace_back(r);
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

void Monitor::setTotalAgents(uint64_t total)
{
    total_agents = total;
}

void Monitor::wake_up() {
    auto waiting = immediate_queue->transitionsWaiting();
    for (unsigned int wait_number : waiting) {
        if (petri_instance->isSensitized(wait_number)) {
            auto awake = immediate_queue->getAgent(wait_number, 0);
            if (awake != nullptr) {
                //logger::record r = {.transition=wait_number,.type="immediate",.action="wake_up",.timestamp=generateTimestamp(),.thread_id=awake->getStrId()};
                //logger.emplace_back(r);
                awake->resume();
                return;
            }
        }
    }
    waiting = temporal_queue->transitionsWaiting();
    for (unsigned int wait_number : waiting) {
        if (petri_instance->isSensitized(wait_number)) {
            auto awake = temporal_queue->getAgent(wait_number, 0);
            if (awake != nullptr) {
                //logger::record r = {.transition=wait_number,.type="immediate",.action="wake_up",.timestamp=generateTimestamp(),.thread_id=awake->getStrId()};
                //logger.emplace_back(r);
                awake->resume();

                return;
            }
        }
    }
}

uint64_t Monitor::generateTimestamp() {
    auto end = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    /*std::time_t now_tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tm = *std::localtime(&now_tt);
    std::stringstream ss;
    ss << std::put_time(&tm, "%F-%T%z");
    return ss.str();*/
}

std::deque<logger::record> *Monitor::getFireLog() {
    return &logger;
}
