#include <algorithm>
#include <memory>
#include "../include/PlaceTransitionNet.hpp"

void PlaceTransitionNet::setFile(std::unique_ptr<ReadFile> load_file){
    file = std::move(load_file);
    petri_instance = std::make_shared<PetriNetwork>();
    petri_instance->setPlaces(file->getNumberPlaces());
    petri_instance->setTransitions(file->getNumberTransitions());
}

std::shared_ptr<PetriNetwork> PlaceTransitionNet::getPetrinet() {
    setDependencyInjection();
    petri_instance->setName(file->networkName());
    return petri_instance;
}

void PlaceTransitionNet::buildStepPTMark(){
    std::unique_ptr<std::map<uint32_t, uint32_t>> map_discrete_places = std::make_unique<std::map<uint32_t, uint32_t>>();
    file->readPlacesDiscrete(map_discrete_places.get());
    initial_mark = std::make_unique<std::vector<uint32_t>>(file->getNumberPlaces());
    current_mark = std::make_unique<std::vector<uint32_t>>(file->getNumberPlaces());
    for (auto& place : *map_discrete_places) {
        initial_mark->at(place.first) = place.second;
        current_mark->at(place.first) = place.second;
    }
}

void PlaceTransitionNet::auxTransitionsDisc() {
    pre_incidence_matrix = std::make_unique<PetriMatrix<int32_t>>();
    post_incidence_matrix = std::make_unique<PetriMatrix<int32_t>>();
    for (auto t = 0; t < numberTransitions; t++)
    {
        auto row_pre = std::make_unique<std::vector<int32_t>>(numberPlaces);
        auto row_pos = std::make_unique<std::vector<int32_t>>(numberPlaces);
        pre_incidence_matrix->addRow(std::move(row_pre));
        post_incidence_matrix->addRow(std::move(row_pos));
    }
}

void PlaceTransitionNet::buildStepHMatrix()
{
    inhibitor_matrix = std::make_unique<PetriMatrix<uint8_t>>();
    for (auto t = 0; t < numberTransitions; t++) {
        auto row = std::make_unique<std::vector<uint8_t>>(numberPlaces);
        inhibitor_matrix->addRow(std::move(row));
    }
    file->readArcsInhibitor(inhibitor_matrix.get());
}

void PlaceTransitionNet::buildStepWMatrix()
{
    reader_matrix = std::make_unique<PetriMatrix<uint8_t>>();
    for (auto t = 0; t < numberTransitions; t++) {
        auto row = std::make_unique<std::vector<uint8_t>>(numberPlaces);
        reader_matrix->addRow(std::move(row));
    }
    file->readArcsReader(reader_matrix.get());
}

void PlaceTransitionNet::buildStepAVector()
{
    reset_vector = std::make_unique<std::vector<uint8_t>>(numberTransitions);
    file->readArcsReset(reset_vector.get());
}

void PlaceTransitionNet::buildStepPTMatrix(){
    map_immediate_transitions = std::make_unique<std::map<uint32_t, std::pair<uint8_t, uint8_t>>>();
    numberPlaces = file->getNumberPlaces();
    numberTransitions = file->getNumberTransitions();
    incidence_matrix = std::make_unique<PetriMatrix<int32_t>>();
    auxTransitionsDisc();
    file->readTransitions(map_immediate_transitions.get(), z_matrix.get());
    file->readArcsPlaceTranstion(pre_incidence_matrix.get(), post_incidence_matrix.get());
    for (auto transition = 0; transition < numberTransitions; transition++) {
        auto row = std::make_unique<std::vector<int32_t>>();
        auto post_row = post_incidence_matrix->getRow(transition);
        auto pre_row = pre_incidence_matrix->getRow(transition);
        for (auto place = 0; place < numberPlaces; place++) {
            row->push_back(post_row.at(place) - pre_row.at(place));
        }
        incidence_matrix->addRow(std::move(row));
    }
    auxStepSensitized(pre_incidence_matrix.get(), *initial_mark);
}

void PlaceTransitionNet::buildStepGuardAndEvent(){
    guard_vector = std::make_unique<std::vector<uint8_t>>(file->getNumberTransitions());
    event_vector = std::make_unique<std::vector<uint8_t>>(file->getNumberTransitions());
    for(auto transition  = 0; transition < file->getNumberTransitions();transition++){
        guard_vector->at(transition) = map_immediate_transitions->at(transition).first;
        event_vector->at(transition) = map_immediate_transitions->at(transition).second;
    }
}

void PlaceTransitionNet::buildStepSensitized(){
    aux_vec = std::make_unique<std::vector<uint8_t>>(file->getNumberPlaces());
    b_vector = std::make_unique<std::vector<uint8_t>>(file->getNumberTransitions());
    l_vector = std::make_unique<std::vector<uint8_t>>(file->getNumberTransitions());
    buildStepGuardAndEvent();
    auxStepBVector(b_vector.get());
    auxStepLVector(l_vector.get());
    for (auto t = 0; t < file->getNumberTransitions(); t++) {
        ex_sensitized_transitions->at(t) = ex_sensitized_transitions->at(t) && (!(b_vector->at(t) && r_vector->at(t)) && (guard_vector->at(t) && event_vector->at(t)));
    }
}

void PlaceTransitionNet::auxStepSensitized(PetriMatrix<int32_t>* pre_incidence, std::vector<uint32_t>& mark) {
    ex_sensitized_transitions = std::make_unique<std::vector<uint8_t>>(file->getNumberTransitions());
    std::vector<bool> v_temp(mark.size());
    for (size_t t = 0; t < numberTransitions; t++) {
        std::transform(mark.begin(), mark.end(), pre_incidence_matrix->getRow(t).begin(), v_temp.begin(), std::greater_equal<>());
        ex_sensitized_transitions->at(t) = std::all_of(v_temp.begin(), v_temp.end(), [](bool e) {return e; });
    }
}

void PlaceTransitionNet::auxStepLVector(std::vector<uint8_t>* l_vec)
{
    r_vector = std::make_unique<std::vector<uint8_t>>(file->getNumberPlaces());
    std::transform(initial_mark->begin(), initial_mark->end(), r_vector->begin(), [](uint32_t mark) {return mark <= 0; });
    for (auto transition = 0; transition < file->getNumberTransitions(); transition++) {
        std::transform(r_vector->begin(), r_vector->end(), reader_matrix->getRow(transition).begin(), aux_vec->begin(), std::logical_and<>());
        l_vec->at(transition) = std::any_of(aux_vec->begin(), aux_vec->end(), [](uint8_t b) {return b; });
    }
}

void PlaceTransitionNet::auxStepBVector(std::vector<uint8_t>* b_vec){
    q_vector = std::make_unique<std::vector<uint8_t>>(file->getNumberPlaces());
    std::transform(initial_mark->begin(), initial_mark->end(), q_vector->begin(), [](uint32_t mark) {return mark > 0; });
    for (auto transition = 0; transition < file->getNumberTransitions(); transition++) {
        std::transform(q_vector->begin(), q_vector->end(), inhibitor_matrix->getRow(transition).begin(), aux_vec->begin(), std::logical_and<>());
        b_vec->at(transition) = std::any_of(aux_vec->begin(), aux_vec->end(), [](uint8_t b) {return b; });
    }
}

void PlaceTransitionNet::setDependencyInjection() {
    petri_instance->setSensitized(std::move(ex_sensitized_transitions));
    petri_instance->setExtendedMatrixes(std::move(inhibitor_matrix), std::move(reader_matrix));
    petri_instance->setGuarAndEvent(std::move(guard_vector), std::move(event_vector));
    petri_instance->setAuxiliarsVectors(std::move(b_vector), std::move(l_vector));
    petri_instance->setInitialMark(std::move(initial_mark));
    petri_instance->setMark(std::move(current_mark));
    petri_instance->setDiscreteMatrixes(std::move(incidence_matrix), std::move(pre_incidence_matrix), std::move(post_incidence_matrix));
}