#include "../include/ReadXML.hpp"

int8_t ReadXML::readFile(std::string &file_name) {
    pugi::xml_parse_result file_xml = pnml_file.load_file(file_name.c_str());
     if(!file_xml){
        return -1;
    }
    net_node = pnml_file.child("pnml").child("net");
    for(auto transition_node = net_node.child("transition");transition_node;transition_node.next_sibling("transition")){
        number_transitions++;
    }
    for(auto transition_node = net_node.child("place");transition_node;transition_node.next_sibling("place")){
        number_places++;
    }
    return 1;
}

void ReadXML::readPlacesDiscrete(std::map<uint32_t, uint32_t>* map_discrete_places){
    for(pugi::xml_node place_node = net_node.child("place");place_node;place_node = place_node.next_sibling("place")){
            if(place_node.attribute("type").as_string()==util_reader::PLACE_DISCRETE){

                uint32_t index = place_node.attribute("idx_id").as_uint();
                uint32_t mark  = std::stoul(place_node.child_value("initialMarking"));
                map_discrete_places->insert(std::pair<uint32_t,uint32_t>{index,mark});
            }
        }
}

void ReadXML::readTransitions(std::map<uint32_t, std::pair<uint8_t, uint8_t>>* map_immediate_transitions, std::vector<std::pair<uint32_t, uint32_t>>* temporal_window) {
    for(pugi::xml_node place_node = net_node.child("transition");place_node;place_node = place_node.next_sibling("transition")){
            if(place_node.attribute("type").as_string()==util_reader::TRANSITION_IMMEDIATE){
                uint32_t index = place_node.attribute("idx_id").as_uint();
                bool guard     = place_node.child("attributes").attribute("guard").as_uint();
                bool event     = place_node.child("attributes").attribute("event").as_uint();
                map_immediate_transitions->insert(std::pair<uint32_t,std::pair<uint8_t, uint8_t>>{index,{guard,event}});
            }
        }
}

void ReadXML::readArcsPlaceTranstion(PetriMatrix<int32_t>* pre_incidence, PetriMatrix<int32_t>* post_incidence) {
    /*
    for(pugi::xml_node arc_node = net_node.child("arc");arc_node;arc_node = arc_node.next_sibling("arc")){
            if(arc_node.attribute("type").as_string()==util_reader::ARC_REGULAR){
                if(arc_node.attribute("from_place").as_bool()){
                    uint32_t idx_place      = arc_node.attribute("source").as_uint();
                    uint32_t idx_transition = arc_node.attribute("target").as_uint();
                    int32_t weight          = std::stoi(arc_node.child_value("weight"));
                    pre_incidence->at((idx_place*number_transitions)+idx_transition) = weight;
                }
                else{
                    uint32_t idx_place      = arc_node.attribute("source").as_uint();
                    uint32_t idx_transition = arc_node.attribute("target").as_uint();
                    int32_t weight         = std::stoi(arc_node.child_value("weight"));
                    post_incidence->at((idx_place*number_transitions)+idx_transition) = weight;
                }
            }
        }*/
}

void ReadXML::readArcsInhibitor(PetriMatrix<uint8_t>* inhibitor_matrix) {
    /*
    for (pugi::xml_node arc_node = net_node.child("arc"); arc_node; arc_node = arc_node.next_sibling("arc")) {
        if (arc_node.attribute("type").as_string() == util_reader::ARC_INHIBITOR) {
            inhibitor_matrix->modifyElement(arc["source"].get<uint32_t>(), arc["target"].get<uint32_t>(), true);
        }
        else {
            inhibitor_matrix->modifyElement(arc["source"].get<uint32_t>(), arc["target"].get<uint32_t>(), false);
        }
    }*/
}

void ReadXML::readArcsReader(PetriMatrix<uint8_t>* reader_matrix) {
    /*json arcs = file_json["arcs"];
    for (auto& arc : arcs) {
        if (arc["type"].get<std::string>() == util_reader::ARC_READER) {
            reader_matrix->modifyElement(arc["source"].get<uint32_t>(), arc["target"].get<uint32_t>(), true);
        }
        else {
            reader_matrix->modifyElement(arc["source"].get<uint32_t>(), arc["target"].get<uint32_t>(), false);
        }
    }*/
}

void ReadXML::readArcsReset(std::vector<uint8_t>* reset_vector) {
    /*json arcs = file_json["arcs"];
    for (auto& arc : arcs) {
        if (arc["type"].get<std::string>() == util_reader::ARC_READER) {
            reset_vector->at(arc["target"].get<uint32_t>()) = true;
        }
        else {
            reset_vector->at(arc["target"].get<uint32_t>()) = false;
        }
    }*/
}

Timescale_Choice::unit ReadXML::getTimeScale()
{
    return Timescale_Choice::unit();
}

Network_Choice::type ReadXML::getNetworkType()
{
    return Network_Choice::type();
}

uint32_t ReadXML::getNumberPlaces(){
    return number_places;
}

uint32_t ReadXML::getNumberTransitions(){
    return number_transitions;
}

std::string ReadXML::networkName(){
    return "";
}