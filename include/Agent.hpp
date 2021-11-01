#pragma once
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <sstream>
//----- Custom headers -----
#include "Monitor.hpp"

/*
* The Agent objet is in charge of carrying out the fire concurrent using the Monitor object.
* Each Agent is compose of a jthread and a fire sequence
* THis interface has all common functions of the immediate and temporal agents.
* */
class Monitor; //Class forwarding
class Agent {
public:
    /*
    * Run the fire sequence
    */
    virtual void execute() = 0;
    /*
    * Relese the guard semaphore and restart the agent's execution
    */
    virtual void resume() = 0;
    /*
    * Put the agent to acquire the guard semaphore
    */
    virtual void suspend() = 0;
    /*
    * Send the stop token sign to stop work
    */
    virtual void stop() = 0;
    /*
    * Assign reference of the Monitor's object
    * @param shared pointer of Monitor object
    */
    virtual void setMonitor(std::shared_ptr<Monitor> monitor) = 0;
    /*
    * Take ownership of the fire sequence
    * @param unique pointer with fire sequence
    */
    virtual void setFireSequence(std::unique_ptr<std::vector<uint32_t>> firesequence) = 0;
    /*
    * Return the ID of the associate jthread. This is used for log proporse or
    * for the record of temporal agent.
    * @return thread id (worker)
    */
    virtual const std::thread::id getId() = 0;

    /*
    * Return the ID of the associate jthread as string. This is used for log proporse or
    * for the record of temporal agent.
    * @return thread id (worker)
    */
    virtual std::string getStrId() = 0;

    //=========== Special functions for temporal agents ===========
    /*
    * Set sleep time of thread.
    */
    virtual void sleep_for(uint32_t time){

    };
};