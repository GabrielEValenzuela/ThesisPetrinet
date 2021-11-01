#include "../include/Queue.hpp"


void Queue::addAgent(uint32_t transition,Agent* agent){
    try{
        //unordered_map.at() --> Average case: constant, worst case: linear in size.
        auto queue = queues->at(transition).get();
        queue->push_back(agent);
        //std::cout << "Queue size now " << queues->size() << "\n";
        //std::cout << "Queue size for " << transition << " is " << queue->size() << "\n";
    }                                                                  
    catch (std::out_of_range){                                         
        //No queue for the transition... So I should create one        
        //unordered_map.insert() --> Average case: constant            
        queues->insert({transition,std::make_unique<std::vector<Agent*>>()});
        auto queue = queues->at(transition).get();                           
        queue->push_back(agent);                                       
        //std::cout << "Queue size now " << queues->size() << "\n";      
        //std::cout << "Queue size for " << transition << " is " << queue->size() << "\n";
    }
}

//Implementar todas las transiciones, las keys del map retorno
//Para determinar el and de los vectores de sensibilizadas y las que estan esperando

Agent* Queue::getAgent(uint32_t transition,uint32_t agent_policy){
    //Linear: the number of calls to the destructor of T is
    //the same as the number of elements erased, the assignment operator of T
    //is called the number of times equal to the number of elements in the vector
    //after the erased elements
    try{
        auto queue = queues->at(transition).get();
        Agent* agent = queue->at(agent_policy);
        queue->erase(queue->begin()+agent_policy);
        return agent;
    }
    catch (std::out_of_range){
        return nullptr;
    }
}

std::vector<uint32_t>& Queue::transitionsWaiting() {
    keys->clear();
    for(auto & it : *queues){
        keys->push_back(it.first);
    }
    std::sort(std::execution::par,keys->begin(),keys->end());
    return *keys;
}