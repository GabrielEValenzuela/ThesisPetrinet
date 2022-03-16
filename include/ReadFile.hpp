#pragma once
#include "../lib/PetriMatrix.hpp"
#include <memory>
#include <string>
#include <cstdint>
#include <map>
#include <vector>

/*
    Interface used to strategy design pattern
*/
class ReadFile{
    protected:
    uint32_t number_places{0};
    uint32_t number_transitions{0};
    public:
    virtual uint32_t getNumberPlaces() = 0;
    virtual uint32_t getNumberTransitions() = 0;
    virtual int8_t readFile(std::string& file_name) = 0;
    virtual void readPlacesDiscrete(std::map<uint32_t, uint32_t>* map_discrete_places) = 0;
    virtual void readTransitions(std::map<uint32_t,std::pair<uint8_t, uint8_t>>* map_immediate_transitions, std::vector<std::pair<uint32_t, uint32_t>>* temporal_window) = 0;
    virtual void readArcsPlaceTranstion(PetriMatrix<int32_t>* pre_incidence, PetriMatrix<int32_t>* post_incidence)=0;
    virtual void readArcsInhibitor(PetriMatrix<uint8_t>* inhibitor_matrix) = 0;
    virtual void readArcsReader(PetriMatrix<uint8_t>* reader_matrix) = 0;
    virtual void readArcsReset(std::vector<uint8_t>* reset_vector) = 0;
    virtual std::string networkName() = 0;
};
