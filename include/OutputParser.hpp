#pragma once

#include<memory>
#include<unordered_map>
#include<string>
#include<deque>
#include<fstream>
#include<iomanip>
#include<unordered_set>
#include"NodeState.hpp"
#include"Logger.hpp"
#include"PetriNetwork.hpp"
#include "../lib/json.hpp"

using json = nlohmann::json;
namespace OutputParser{
    static void MinCovOut(std::unordered_map<std::string, std::shared_ptr<NodeState>>* processed_set,double time,PetriNetwork* instance){
        json out;
        out["network"]      = instance->getName();
        out["places"]       = instance->getPlaces();
        out["transitions"]  = instance->getTransitions();
        out["time"]         = time;
        std::vector<nlohmann::json> nodes;
        std::vector<nlohmann::json> edges;
        std::unordered_set<std::string> filter_node;
        for(const auto& pair_node : *processed_set){
            if (!filter_node.contains(pair_node.second->getInfo())) {
                nlohmann::json pnode;
                pnode["id"]     = "n" + std::to_string(pair_node.second->getNodeId());
                pnode["state"]  = pair_node.second->getInfo();
                if (pair_node.second->isAccelerated()) {
                    pnode["group"] = "omega";
                }
                else {
                    pnode["group"] = "not omega";
                }
                if (pair_node.second->getFire() == -1) {
                    pnode["group"] = "root";
                }
                for (const auto& child : *pair_node.second->getChildren()) {
                    nlohmann::json chnode;
                    nlohmann::json chedge;
                    chedge["from"]  = "n" + std::to_string(pair_node.second->getNodeId());;
                    chedge["to"]    = "n" + std::to_string(child->getNodeId());
                    chedge["path"]  = chedge["from"].get<std::string>() + " --(T" + std::to_string((1 + child->getFire())) + ")--> " + chedge["to"].get<std::string>();
                    chnode["id"]    = "n" + std::to_string(child->getNodeId());;
                    chnode["state"] = child->getInfo();
                    if (child->isAccelerated()) {
                        chnode["group"] = "omega";
                    }
                    else {
                        chnode["group"] = "not omega";
                    }
                    edges.emplace_back(chedge);
                    nodes.emplace_back(chnode);
                    filter_node.insert(child->getInfo());
                }
                if (pair_node.second->getParentNode() != nullptr) {
                    nlohmann::json chedge;
                    chedge["from"]  = "n" + std::to_string(pair_node.second->getParentNode()->getNodeId());
                    chedge["to"]    = "n" + std::to_string(pair_node.second->getNodeId());
                    chedge["path"]  = chedge["from"].get<std::string>() + " --(T" + std::to_string((1 + pair_node.second->getFire())) + ")--> " + chedge["to"].get<std::string>();
                    edges.emplace_back(chedge);
                }
                nodes.emplace_back(pnode);
                filter_node.insert(pair_node.second->getInfo());;
            }
        }
        out["nodes"] = nlohmann::json(nodes);
        out["edges"] = nlohmann::json(edges);
        std::string file_name{"mincov_out.json"};
        std::ofstream o(file_name);
        o << std::setw(4) << out << std::endl;
    }

    static void MonitorOut(std::deque<logger::record>* record_log,double time,PetriNetwork* instance,uint32_t fire_count){
        auto tmp = std::make_unique<std::unordered_map<uint16_t,uint32_t>>();
        std::vector<json> history;
        while(!record_log->empty()){
            json record;
            auto r = record_log->front();
            tmp->insert_or_assign(r.transition,tmp->count(r.transition)+1);
            record["id"]            =   r.thread_id;
            record["transition"]    =   "T"+std::to_string(r.transition);
            record["type"]          =   r.type;
            record["action"]        =   r.action;
            record["timestamp"]     =   r.timestamp;
            record_log->pop_front();
            history.emplace_back(record);
        }
        uint32_t max_fired,min_fired,min=99999,max=0;
        for(auto const& t: *tmp){
            if(t.second < min){
                min_fired   = t.first;
                min         = t.second;
            }
            if(t.second > max){
                max_fired   = t.first;
                max         = t.second;
            }
        }
        json out;
        out["network"]      = instance->getName();
        out["places"]       = instance->getPlaces();
        out["transitions"]  = instance->getTransitions();
        out["time"]         = time;
        out["fire_count"]   = fire_count;
        out["min_fired"]    = min_fired;
        out["max_fired"]    = max_fired;
        out["history"]      = nlohmann::json(history);
        std::string file_name{"monitor_out.json"};
        std::ofstream o(file_name);
        o << std::setw(4) << out << std::endl;
    }
};