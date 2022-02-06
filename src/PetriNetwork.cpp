//
// Created by gabrielv on 16/5/21.
//


#include "../include/PetriNetwork.hpp"

void PetriNetwork::printMark() {
    std::cout << "[";
    for (auto e : *disc_mark) {
        std::cout << " " << e << " ";
    }
    std::cout << "]\n";
}

void PetriNetwork::setName(std::string&& name){
    petri_name = std::move(name);
}

std::string& PetriNetwork::getName(){
    return petri_name;
}

void PetriNetwork::setMark(std::vector<uint32_t>* mark)
{
    std::copy(std::execution::unseq, mark->begin(), mark->end(), disc_mark->begin());
    updateSensitized();
}

void PetriNetwork::resetNetworkStatus()
{
    std::copy(std::execution::unseq, disc_mark->begin(), disc_mark->end(), initial_disc_mark->begin());
    updateSensitized();
}

bool PetriNetwork::isSensitized(uint32_t transition_number) {
    return current_sensitized->at(transition_number);
}

void PetriNetwork::setExtendedMatrixes(std::unique_ptr<PetriMatrix<uint8_t>> inhibitor, std::unique_ptr<PetriMatrix<uint8_t>> writer)
{
    this->inhibitor_matrix = std::move(inhibitor);
    this->reader_matrix    = std::move(writer);
}

void PetriNetwork::setAuxiliarsVectors(std::unique_ptr<std::vector<uint8_t>> b_vector, std::unique_ptr<std::vector<uint8_t>> l_vector)
{
    q_vec       = std::make_unique<std::vector<uint8_t>>(this->places);
    w_vec       = std::make_unique<std::vector<uint8_t>>(this->places);
    q_vec_aux   = std::make_unique<std::vector<uint8_t>>(this->places);
    w_vec_aux   = std::make_unique<std::vector<uint8_t>>(this->places);
    b_vec_aux   = std::make_unique<std::vector<uint8_t>>(transitions);
    l_vec_aux   = std::make_unique<std::vector<uint8_t>>(transitions);
    this->b_vec = std::move(b_vector);
    this->l_vec = std::move(l_vector);
}

void PetriNetwork::setGuarAndEvent(std::unique_ptr<std::vector<uint8_t>> guards, std::unique_ptr<std::vector<uint8_t>> events)
{
    this->guard_vec = std::move(guards);
    this->event_vec = std::move(events);
}

void PetriNetwork::setTimeLogic(std::shared_ptr<TimeLogic> scale)
{
    this->time_unit = scale;
}

void PetriNetwork::setTemporalWindow(std::unique_ptr<std::vector<std::pair<uint32_t, uint32_t>>> z_matrix)
{
    temporal_window = std::move(z_matrix);
    temporal_logic = std::make_unique<std::unordered_map<uint32_t, TimeVarianceAuthority>>();
    for (auto t = 0; t < temporal_window->size(); t++) {
        TimeVarianceAuthority tva = { temporal_window->at(t).first,temporal_window->at(t).second,std::this_thread::get_id(),std::nullopt };
        temporal_logic->insert({t,tva});
    }
}

int64_t PetriNetwork::isTemporalSensitized(uint32_t transition)
{
    if (temporal_logic->at(transition).timestamp.has_value()) {
        return time_unit->isTemporalSensitized(temporal_window->at(transition).first, temporal_window->at(transition).second, temporal_logic->at(transition).timestamp.value());
    }
    else {
        return -1;
    }
    
}

bool PetriNetwork::isEarly(uint32_t transition, const std::chrono::high_resolution_clock::time_point& timestamp)
{
    return time_unit->isEarly(temporal_window->at(transition).first, timestamp);
}

std::shared_ptr<std::vector<uint8_t>> PetriNetwork::applyExtendedEquation(std::vector<uint32_t>* mark,std::vector<int32_t>* row_pre){
    auto sensitized = std::make_shared<std::vector<uint8_t>>(transitions);
    std::transform(std::execution::unseq, mark->begin(), mark->end(), q_vec_aux->begin(), [](uint32_t mark) {return mark > 0; });
    std::transform(std::execution::unseq, mark->begin(), mark->end(), w_vec_aux->begin(), [](uint32_t mark) {return mark <= 0; });


    for (auto transition = 0; transition < transitions; transition++) {
        std::transform(std::execution::unseq, q_vec_aux->begin(), q_vec_aux->end(), inhibitor_matrix->getRow(transition).begin(), aux_vec->begin(), [](uint8_t a, uint8_t b) {return a && b; });
        b_vec_aux->at(transition) = std::any_of(std::execution::unseq, aux_vec->begin(), aux_vec->end(), [](uint8_t b) {return b; });
        std::transform(std::execution::unseq, w_vec_aux->begin(), w_vec_aux->end(), reader_matrix->getRow(transition).begin(), aux_vec->begin(), [](uint8_t a, uint8_t b) {return a && b; });
        l_vec_aux->at(transition) = std::any_of(std::execution::unseq, aux_vec->begin(), aux_vec->end(), [](uint8_t b) {return b; });
    }

    for (size_t t = 0; t < transitions; t++) {
        std::transform(std::execution::unseq, mark->begin(), mark->end(), row_pre->begin(), aux_vec->begin(), [](uint32_t m, int32_t i) { return m >= i; });
        sensitized->at(t) = std::all_of(std::execution::unseq, aux_vec->begin(), aux_vec->end(), [](uint8_t e) {return e; });
    }

    std::transform(std::execution::unseq,sensitized->begin(), sensitized->end(), guard_vec->begin(), sensitized->begin(), [](uint8_t a, uint8_t b) {return a && b; });
    std::transform(std::execution::unseq,sensitized->begin(), sensitized->end(), event_vec->begin(), sensitized->begin(), [](uint8_t a, uint8_t b) {return a && b; });
    std::transform(std::execution::unseq,sensitized->begin(), sensitized->end(), b_vec_aux->begin(), sensitized->begin(), [](uint8_t s, uint8_t b) {return s && !b; });
    std::transform(std::execution::unseq,sensitized->begin(), sensitized->end(), l_vec_aux->begin(), sensitized->begin(), [](uint8_t s, uint8_t b) {return s && !b; });

    return sensitized;
}

void PetriNetwork::updateSensitized() {
    /*
        Update the Q and W vector.
        The Q vector is true if the mark for the place Pi it's greater than 0.
        The W vector is true if the mark for the place Pi it's 0.
    */
    std::transform(std::execution::unseq, disc_mark->begin(), disc_mark->end(), q_vec->begin(), [](uint32_t mark) {return mark > 0; });
    std::transform(std::execution::unseq, disc_mark->begin(), disc_mark->end(), w_vec->begin(), [](uint32_t mark) {return !(mark > 0); });
    /*for (auto place = 0; place < disc_mark->size(); place++) {
        if (disc_mark->at(place) > 0) {
            q_vec->at(place) = true;
            w_vec->at(place) = false;
        }
        else {
            q_vec->at(place) = false;
            w_vec->at(place) = true;
        }
    }*/

    for (auto transition = 0; transition < transitions; transition++) {
        std::transform(std::execution::unseq, q_vec->begin(), q_vec->end(), inhibitor_matrix->getRow(transition).begin(), (*aux_vec).begin(), [](uint8_t a, uint8_t b) {return a && b; });
        b_vec->at(transition) = std::any_of(std::execution::unseq, aux_vec->begin(), aux_vec->end(), [](uint8_t b) {return b; });
        std::transform(std::execution::unseq, w_vec->begin(), w_vec->end(), reader_matrix->getRow(transition).begin(), (*aux_vec).begin(), [](uint8_t a, uint8_t b) {return a && b; });
        l_vec->at(transition) = std::any_of(std::execution::unseq, aux_vec->begin(), aux_vec->end(), [](uint8_t b) {return b; });
    }

    for (size_t t = 0; t < transitions; t++) {
        std::transform(std::execution::unseq, disc_mark->begin(), disc_mark->end(), disc_pre_matrix->getRow(t).begin(), (*aux_vec).begin(), [](uint32_t m, int32_t i) { return m >= i; });
        current_sensitized->at(t) = std::all_of(std::execution::unseq, aux_vec->begin(), aux_vec->end(), [](uint8_t e) {return e; });
    }
    std::transform(std::execution::unseq,current_sensitized->begin(), current_sensitized->end(), guard_vec->begin(), current_sensitized->begin(), [](uint8_t a, uint8_t b) {return a && b; });
    std::transform(std::execution::unseq,current_sensitized->begin(), current_sensitized->end(), event_vec->begin(), current_sensitized->begin(), [](uint8_t a, uint8_t b) {return a && b; });
    std::transform(std::execution::unseq,current_sensitized->begin(), current_sensitized->end(), b_vec->begin(), current_sensitized->begin(), [](uint8_t s, uint8_t b) {return s && !b; });
    std::transform(std::execution::unseq,current_sensitized->begin(), current_sensitized->end(), l_vec->begin(), current_sensitized->begin(), [](uint8_t s, uint8_t b) {return s && !b; });
    updateTimestamps();
}

std::vector<uint32_t>* PetriNetwork::getMark() {
    return disc_mark.get();
}

std::vector<uint32_t>& PetriNetwork::getInitialMark() {
    return *initial_disc_mark;
}

std::vector<uint8_t>& PetriNetwork::getCurrenSensitized()
{
    return *current_sensitized;
}


const std::vector<int32_t>& PetriNetwork::getRow(uint32_t row_number) {
    return disc_matrix->getRow(row_number);
}

const std::vector<int>& PetriNetwork::getRowPre(uint32_t row_number){
    return disc_pre_matrix->getRow(row_number);
}

const std::vector<int>& PetriNetwork::getRowPost(uint32_t row_number) {
    return disc_post_matrix->getRow(row_number);
}

void PetriNetwork::checkSpecialCase(uint32_t transition)
{
    if (last_sensitized->at(transition) == current_sensitized->at(transition)) {
        temporal_logic->at(transition).timestamp    = std::chrono::high_resolution_clock::now();
    }

}

void PetriNetwork::updateTimestamps()
{
    if (temporal_logic.get() != nullptr) {
        for (auto t = 0; t < transitions; t++) {
            if (!last_sensitized->at(t) && current_sensitized->at(t)) {
                temporal_logic->at(t).timestamp = std::chrono::high_resolution_clock::now();
            }
            else if (last_sensitized->at(t) && !current_sensitized->at(t)) {
                temporal_logic->at(t).timestamp = std::nullopt;
            }
        }

        std::copy(std::execution::unseq,current_sensitized->begin(), current_sensitized->end(), last_sensitized->begin());
    }
}

/*void PetriNetwork::extendedEquation()
{
    
        //Update the Q and W vector.
        //The Q vector is true if the mark for the place Pi it's greater than 0.
        //The W vector is true if the mark for the place Pi it's 0.
    
    std::transform(std::execution::par, disc_mark->begin(), disc_mark->end(), q_vec->begin(), [](uint32_t mark) {return mark > 0; });
    std::transform(std::execution::par, disc_mark->begin(), disc_mark->end(), w_vec->begin(), [](uint32_t mark) {return !(mark > 0); });
    //for (auto place = 0; place < disc_mark->size(); place++) {
      //  if (disc_mark->at(place) > 0) {
        //    q_vec->at(place) = true;
          //  w_vec->at(place) = false;
        //}
        //else {
          //  q_vec->at(place) = false;
//            w_vec->at(place) = true;
  //      }
    //}

    for (auto transition = 0; transition < transitions; transition++) {
        std::transform(std::execution::par, q_vec->begin(), q_vec->end(), inhibitor_matrix->getRow(transition).begin(), (*aux_vec).begin(), [](uint8_t a, uint8_t b) {return a && b; });
        b_vec->at(transition) = std::any_of(std::execution::par, aux_vec->begin(), aux_vec->end(), [](uint8_t b) {return b; });
        std::transform(std::execution::par, w_vec->begin(), w_vec->end(), reader_matrix->getRow(transition).begin(), (*aux_vec).begin(), [](uint8_t a, uint8_t b) {return a && b; });
        l_vec->at(transition) = std::any_of(std::execution::par, aux_vec->begin(), aux_vec->end(), [](uint8_t b) {return b; });
    }
}*/

void PetriNetwork::setMark(std::unique_ptr<std::vector<uint32_t>> new_mark) {
    disc_mark = std::move(new_mark);
}

void PetriNetwork::setInitialMark(std::unique_ptr<std::vector<uint32_t>> init_mark) {
    initial_disc_mark = std::move(init_mark);
}

void PetriNetwork::setSensitized(std::unique_ptr<std::vector<uint8_t>> new_sensitized) {
    current_sensitized  = std::move(new_sensitized);
    last_sensitized     = std::make_unique<std::vector<uint8_t>>(this->transitions);
    aux_vec             = std::make_unique<std::vector<uint8_t>>(this->places);
    std::copy(current_sensitized->begin(), current_sensitized->end(), last_sensitized->begin());
}

void PetriNetwork::setDiscreteMatrixes(std::unique_ptr<PetriMatrix<int32_t>> incidence_matrix, std::unique_ptr<PetriMatrix<int32_t>> pre_matrix, std::unique_ptr<PetriMatrix<int32_t>> post_matrix) {
    disc_matrix = std::move(incidence_matrix);
    disc_pre_matrix = std::move(pre_matrix);
    disc_post_matrix = std::move(post_matrix);
}

int PetriNetwork::getValue(uint32_t place,uint32_t transition) {
    return disc_matrix->getElement(transition, place);
}

bool PetriNetwork::stillSensitized() {
    return std::any_of(current_sensitized->begin(), current_sensitized->end(),[](uint8_t s){return s;});
}
