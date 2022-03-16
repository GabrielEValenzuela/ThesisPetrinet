#include <iomanip>
#include "../include/Monitor.hpp"


Monitor::Monitor(std::shared_ptr<PetriNetwork> petri_network) {
    petri_instance = petri_network;
    temporal_filter = std::make_unique<std::unordered_map<uint32_t, std::thread::id>>();
    immediate_queue = std::make_unique<Queue>();
    temporal_queue = std::make_unique<Queue>();
    hodor = std::make_unique<std::binary_semaphore>(1);
}

bool Monitor::fireImmediate(Agent *agent, uint32_t transition) {
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
    try {
        if (petri_instance->isSensitized(transition)) {
            #ifdef PROFILING_ENABLE
            TracyMessageL("Agent is enable to fire transition");
            #endif
            auto new_mark = std::make_unique<std::vector<uint32_t>>(petri_instance->getMark()->size());
            MathEngine::fire(petri_instance->getMark(), petri_instance->getRow(transition), new_mark.get());
            petri_instance->setMark(std::move(new_mark));
            petri_instance->updateSensitized();
            fire_counter.fetch_add(1);
            wakeUp();
            guard.unlock();
            hodor->release();
            #ifdef PROFILING_ENABLE
            TracyMessageL("Agent leaves monitor after fire transition");
            #endif
            return true;

        } else {
            #ifdef PROFILING_ENABLE
            TracyMessageL("Agent is not enable to fire transition");
            #endif
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
    catch (const std::exception &ex) {
        std::cout << "***ERROR***\t" << ex.what() << "\n";
        exit(-1);
    }
}

bool Monitor::fireTemporal(Agent *agent, uint32_t transition) {
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
                    auto new_mark = std::make_unique<std::vector<uint32_t>>(petri_instance->getMark()->size());
                    MathEngine::fire(petri_instance->getMark(), petri_instance->getRow(transition), new_mark.get());
                    petri_instance->setMark(std::move(new_mark));
                    petri_instance->updateSensitized();
                    fire_counter.fetch_add(1);
                    temporal_filter->erase(transition);
                    wakeUp();
                    guard.unlock();
                    hodor->release();
                    #ifdef PROFILING_ENABLE
                    TracyMessageL("Agent leaves monitor after fire temporal transition");
                    #endif
                    return true;
                } else {
                    //I'm the first, I'm sensitized but I'm not inside window...
                    //So, I have two choices:
                    //If I'm early, i.e. before alpha time, I go to sleep the remaining time...
                    if (inside > 0) {
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
        checkAndUnlock();
        guard.unlock();
        hodor->release();

        temporal_queue->addAgent(transition, agent);
        #ifdef PROFILING_ENABLE
        TracyMessageL("Agent leaves monitor after puts itself in queue");
        #endif
        return false;
    }
    catch (const std::exception &ex) {
        std::cout << "***ERROR***\t" << ex.what() << "\n";
        exit(-1);
    }
    return false;
}

uint64_t Monitor::getFireCount() {
    return fire_counter.load();
}

void Monitor::resetFireCount() {
    fire_counter = 0;
    immediate_queue->clearQueue();
    temporal_queue->clearQueue();
}

bool Monitor::stillSensitized() {
    return petri_instance->stillSensitized();
}

void Monitor::checkAndUnlock() {
    if (immediate_queue->getSize() + temporal_queue->getSize() == total_agents) {
        wakeUp();
    }

}

bool Monitor::isAnotherFirst(uint32_t transition, Agent *agent) const {
    try {
        //There is an temporal agent record for a specific transition
        //Verify if is the same agent...
        if (temporal_filter->at(transition) == agent->getId()) {
            return false;
        } else {
            return true;
        }
    }
    catch (const std::out_of_range &e) {
        //No record for the transition, that means I'M FIRST
        //I made a record
        temporal_filter->insert({transition, agent->getId()});
        return false;
    }

}

void Monitor::setTotalAgents(uint64_t total) {
    total_agents = total;
}

void Monitor::wakeUp() {
    auto waiting = immediate_queue->transitionsWaiting();
    for (unsigned int wait_number: waiting) {
        if (petri_instance->isSensitized(wait_number)) {
            auto awake = immediate_queue->getAgent(wait_number, 0);
            if (awake != nullptr) {
                awake->resume();
                return;
            }
        }
    }
    waiting = temporal_queue->transitionsWaiting();
    for (unsigned int wait_number: waiting) {
        if (petri_instance->isSensitized(wait_number)) {
            auto awake = temporal_queue->getAgent(wait_number, 0);
            if (awake != nullptr) {
                awake->resume();
                return;
            }
        }
    }
}

uint64_t Monitor::generateTimestamp() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

std::deque<logger::record> *Monitor::getFireLog() {
    return &logger;
}

bool Monitor::fireImmediateWL(Agent *agent, uint32_t transition) {
    hodor->acquire();
    std::unique_lock<std::mutex> guard(mutex);
    try {
        if (petri_instance->isSensitized(transition)) {
            logger::record r = {
                    .transition=transition,
                    .type="immediate",
                    .action="fire",
                    .timestamp=std::move(generateTimestamp()),
                    .thread_id=agent->getStrId()
            };
            logger.emplace_back(r);
            auto new_mark = std::make_unique<std::vector<uint32_t>>(petri_instance->getMark()->size());
            MathEngine::fire(petri_instance->getMark(), petri_instance->getRow(transition), new_mark.get());
            petri_instance->setMark(std::move(new_mark));
            petri_instance->updateSensitized();
            fire_counter.fetch_add(1);
            wakeUpWL();
            guard.unlock();
            hodor->release();
            return true;

        } else {
            logger::record r = {
                    .transition=transition,
                    .type="immediate",
                    .action="sleep",
                    .timestamp=std::move(generateTimestamp()),
                    .thread_id=agent->getStrId()
            };
            logger.emplace_back(r);
            checkAndUnlock();
            guard.unlock();
            immediate_queue->addAgent(transition, agent);
            hodor->release();
            return false;
        }
    }
    catch (const std::exception &ex) {
        std::cout << "***ERROR***\t" << ex.what() << "\n";
        exit(-1);
    }
}

bool Monitor::fireTemporalWL(Agent *agent, uint32_t transition) {
    hodor->acquire();
    std::unique_lock<std::mutex> guard(mutex);
    try {
        if (petri_instance->isSensitized(transition)) {
            if (!isAnotherFirst(transition, agent)) {
                auto inside = petri_instance->isTemporalSensitized(transition);
                if (!inside) {
                    logger::record r = {
                            .transition=transition,
                            .type="temporal",
                            .action="fire",
                            .timestamp=std::move(generateTimestamp()),
                            .thread_id=agent->getStrId()
                    };
                    logger.emplace_back(r);
                    auto new_mark = std::make_unique<std::vector<uint32_t>>(petri_instance->getMark()->size());
                    MathEngine::fire(petri_instance->getMark(), petri_instance->getRow(transition), new_mark.get());
                    petri_instance->setMark(std::move(new_mark));
                    petri_instance->updateSensitized();
                    fire_counter.fetch_add(1);
                    temporal_filter->erase(transition);
                    wakeUpWL();
                    guard.unlock();
                    hodor->release();
                    return true;
                } else {
                    if (inside > 0) {
                        logger::record r = {
                                .transition=transition,
                                .type="immediate",
                                .action="early",
                                .timestamp=std::move(generateTimestamp()),
                                .thread_id=agent->getStrId()
                        };
                        logger.emplace_back(r);
                        agent->sleep_for(inside);
                        guard.unlock();
                        hodor->release();
                        return false;
                    }
                }
            }
        }
        logger::record r = {
                .transition=transition,
                .type="temporal",
                .action="sleep",
                .timestamp=std::move(generateTimestamp()),
                .thread_id=agent->getStrId()
        };
        logger.emplace_back(r);
        checkAndUnlock();
        guard.unlock();
        hodor->release();
        temporal_queue->addAgent(transition, agent);
        return false;
    }
    catch (const std::exception &ex) {
        std::cout << "***ERROR***\t" << ex.what() << "\n";
        exit(-1);
    }
}

void Monitor::wakeUpWL() {
    auto waiting = immediate_queue->transitionsWaiting();
    for (unsigned int wait_number: waiting) {
        if (petri_instance->isSensitized(wait_number)) {
            auto awake = immediate_queue->getAgent(wait_number, 0);
            if (awake != nullptr) {
                logger::record r = {
                        .transition=wait_number,
                        .type="immediate",
                        .action="wakeUp",
                        .timestamp=generateTimestamp(),
                        .thread_id=awake->getStrId()};
                logger.emplace_back(r);
                awake->resume();
                return;
            }
        }
    }
    waiting = temporal_queue->transitionsWaiting();
    for (unsigned int wait_number: waiting) {
        if (petri_instance->isSensitized(wait_number)) {
            auto awake = temporal_queue->getAgent(wait_number, 0);
            if (awake != nullptr) {
                logger::record r = {
                        .transition=wait_number,
                        .type="temporal",
                        .action="wakeUp",
                        .timestamp=generateTimestamp(),
                        .thread_id=awake->getStrId()};
                logger.emplace_back(r);
                awake->resume();

                return;
            }
        }
    }
}
