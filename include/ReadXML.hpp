#pragma once
#include "../lib/pugixml.hpp"
#include "../lib/PetriMatrix.hpp"
#include "../lib/entityPN.hpp"
#include "ReadFile.hpp"
#include <fstream>

class ReadXML : public ReadFile {
private:
    pugi::xml_node net_node;
    pugi::xml_document pnml_file;
public:
    int8_t readFile(std::string& file_name) override;
    void readPlacesDiscrete(std::map<uint32_t, uint32_t>* map_discrete_places) override;
    void readTransitions(std::map<uint32_t, std::pair<uint8_t, uint8_t>>* map_immediate_transitions, std::vector<std::pair<uint32_t, uint32_t>>* temporal_window) override;
    void readArcsPlaceTranstion(PetriMatrix<int32_t>* pre_incidence, PetriMatrix<int32_t>* post_incidence) override;
    void readArcsInhibitor(PetriMatrix<uint8_t>* inhibitor_matrix) override;
    void readArcsReader(PetriMatrix<uint8_t>* reader_matrix) override;
    void readArcsReset(std::vector<uint8_t>* reset_vector) override;
    Timescale_Choice::unit getTimeScale();
    Network_Choice::type getNetworkType();
    uint32_t getNumberPlaces() override;
    uint32_t getNumberTransitions() override;
    std::string networkName() override;
};