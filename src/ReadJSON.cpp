#include <cstdint>
#include "../include/ReadJSON.hpp"

int8_t ReadJSON::readFile(std::string &file_name){
    std::cout << "Name " << file_name << "\n";
    std::ifstream file(file_name.c_str());
    if (file.bad() || !file.is_open()) {
        return -1;
    }
        file_json = json::parse(file);
        number_transitions = file_json["transitions"].size();
        number_places      = file_json["places"].size();
        std::cout <<"Network "<<file_json["network"]["id"].get<std::string>()<<" successfully loaded\n";
        std::cout <<"Amount places (declared/readed) :"<<file_json["network"]["amount_places"]<<"/"<<number_places<<"\n";
        std::cout <<"Amount transitions (declared/readed) :"<<file_json["network"]["amount_transitions"]<<"/"<<number_transitions<<"\n";
        std::cout <<"Network type: " << file_json["network"]["network_type"] << "\t Temporal?: " << file_json["network"]["is_temporal"]<<"\n";
        std::cout << "Time unit configured: " << file_json["network"]["time_scale"] << "\n\n";
        return 1;
}

void ReadJSON::readPlacesDiscrete(std::map<uint32_t, uint32_t>* map_discrete_places){
    json places = file_json["places"];
        for(auto &place : places){
            if(place["type"].get<std::string>()==util_reader::PLACE_DISCRETE){
                map_discrete_places->insert({place["index"].get<uint32_t>(),place["initial_marking"].get<uint32_t>()});
            }
        }
}

void ReadJSON::readTransitions(std::map<uint32_t, std::pair<uint8_t, uint8_t>>* map_immediate_transitions, std::vector<std::pair<uint32_t, uint32_t>>* temporal_window) {
    json transitions = file_json["transitions"];
    if (temporal_window != nullptr) {
        for (auto& transition : transitions) {
            if (transition["type"] == util_reader::TRANSITION_IMMEDIATE) {
                map_immediate_transitions->insert(
                    {
                        transition["index"].get<uint32_t>(),
                        {transition["guard"].get<uint8_t>(),transition["event"].get<uint8_t>()}
                    });
                temporal_window->push_back({ 0, 0 });
            }
            else {
                map_immediate_transitions->insert(
                    {
                        transition["index"].get<uint32_t>(),
                        {transition["guard"].get<uint8_t>(),transition["event"].get<uint8_t>()}
                    });
                temporal_window->push_back({ transition["alpha"].get<uint32_t>(), transition["beta"].get<uint32_t>() });
            }
        }
    }
    else {
        for (auto& transition : transitions) {
            if (transition["type"] == util_reader::TRANSITION_IMMEDIATE) {
                map_immediate_transitions->insert(
                    {
                        transition["index"].get<uint32_t>(),
                        {transition["guard"].get<uint8_t>(),transition["event"].get<uint8_t>()}
                    });
            }
            else {
                map_immediate_transitions->insert(
                    {
                        transition["index"].get<uint32_t>(),
                        {transition["guard"].get<uint8_t>(),transition["event"].get<uint8_t>()}
                    });
            }
        }
    }
}

void ReadJSON::readArcsPlaceTranstion(PetriMatrix<int32_t>* pre_incidence, PetriMatrix<int32_t>* post_incidence) {
    json arcs = file_json["arcs"];
        for(auto &arc : arcs){
            if(arc["type"].get<std::string>() == util_reader::ARC_REGULAR){
                if(arc["from_place"].get<bool>()){
                    //std::cout << "Arc from place " << arc["source"].get<uint32_t>() << " to transtion " << arc["target"].get<uint32_t>() << " with weight " << arc["weight"].get<uint32_t>() << "\n";
                    pre_incidence->modifyElement(arc["source"].get<uint32_t>(), arc["target"].get<uint32_t>(), arc["weight"].get<uint32_t>());
                } else{
                    //std::cout << "Arc from transtion " << arc["source"].get<uint32_t>() << " to place " << arc["target"].get<uint32_t>() << " with weight " << arc["weight"].get<uint32_t>() << "\n";
                    post_incidence->modifyElement(arc["target"].get<uint32_t>(), arc["source"].get<uint32_t>(), arc["weight"].get<uint32_t>());
                }
            }
        }
}

void ReadJSON::readArcsInhibitor(PetriMatrix<uint8_t>* inhibitor_matrix) {
        json arcs = file_json["arcs"];
        for(auto &arc : arcs){
            if (arc["from_place"].get<bool>()) {
                if (arc["type"].get<std::string>() == util_reader::ARC_INHIBITOR) {
                    inhibitor_matrix->modifyElement(arc["source"].get<uint32_t>(), arc["target"].get<uint32_t>(), 1);
                }
                else {
                    inhibitor_matrix->modifyElement(arc["source"].get<uint32_t>(), arc["target"].get<uint32_t>(), 0);
                }
            }
        }
}

void ReadJSON::readArcsReader(PetriMatrix<uint8_t>* reader_matrix) {
    json arcs = file_json["arcs"];
    for (auto& arc : arcs) {
        if (arc["from_place"].get<bool>()) {
            if (arc["type"].get<std::string>() == util_reader::ARC_READER) {
                reader_matrix->modifyElement(arc["source"].get<uint32_t>(), arc["target"].get<uint32_t>(), 1);
            }
            else {
                reader_matrix->modifyElement(arc["source"].get<uint32_t>(), arc["target"].get<uint32_t>(), 0);
            }
        }
    }
}

void ReadJSON::readArcsReset(std::vector<uint8_t>* reset_vector) {
    json arcs = file_json["arcs"];
    for (auto& arc : arcs) {
        if (arc["from_place"].get<bool>()) {
            if (arc["type"].get<std::string>() == util_reader::ARC_RESET) {
                reset_vector->at(arc["source"]) = 1;
            }
            else {
                reset_vector->at(arc["source"]) = 0;
            }
        }
    }
}

uint32_t ReadJSON::getNumberPlaces(){
    return number_places;
}

uint32_t ReadJSON::getNumberTransitions(){
    return number_transitions;
}

Network_Choice::type ReadJSON::getNetworkType(){
    auto type_read = file_json["network"]["network_type"].get<std::string>();
    if (type_read.compare("discrete") == 0) {
        if (isTemporal()) {
            return Network_Choice::type::DISCRETE_TEMPORAL;
        }
        else {
            return Network_Choice::type::DISCRETE_IMMEDIATE;
        }
    }
    else if (type_read.compare("continuos") == 0) {
        if (isTemporal()) {
            return Network_Choice::type::CONTINUOS_TEMPORAL;
        }
        else {
            return Network_Choice::type::CONTINUOS_IMMEDIATE;
        }
    }
    else if (type_read.compare("colored") == 0) {
        if (isTemporal()) {
            return Network_Choice::type::COLORED_TEMPORAL;
        }
        else {
            return Network_Choice::type::COLORED_IMMEDIATE;
        }
    }
    else {
        return Network_Choice::type::ERR;
    }
}

bool ReadJSON::isTemporal() {
    return file_json["network"]["is_temporal"].get<bool>();
}

Timescale_Choice::unit ReadJSON::getTimeScale() {
    auto scale_read = file_json["network"]["time_scale"].get<std::string>();
    if (scale_read.compare("nanosecond") == 0) {
        return Timescale_Choice::unit::NANOSECOND;
    }
    else if (scale_read.compare("microsecond") == 0) {
        return Timescale_Choice::unit::MICROSECOND;
    }
    else if (scale_read.compare("millisecond") == 0) {
        return Timescale_Choice::unit::MILLISECOND;
    }
    else if (scale_read.compare("second") == 0) {
        return Timescale_Choice::unit::SECOND;
    }
    else if (scale_read.compare("minute") == 0) {
        return Timescale_Choice::unit::MINUTE;
    }
    else if (scale_read.compare("hour") == 0) {
        return Timescale_Choice::unit::HOUR;
    }
    else {
        return Timescale_Choice::unit::ERR;
    }
}

std::string ReadJSON::networkName() {
    return file_json["network"]["id"].get<std::string>();
}
