#pragma once
#include <vector>
#include <memory>
#include <chrono>
#include <functional>
#include <algorithm>
#include <execution>
#include <numeric>
#include <optional>
#include "Agent.hpp"
#include "Logger.hpp"
#include "TimeLogic.hpp"
#include "../lib/PetriMatrix.hpp"

/*
    Main logic class that contains all information about the petri network
*/
class PetriNetwork {
private:

    std::unique_ptr<std::vector<uint8_t>> q_vec;
    std::unique_ptr<std::vector<uint8_t>> w_vec;
    std::unique_ptr<std::vector<uint8_t>> b_vec;
    std::unique_ptr<std::vector<uint8_t>> l_vec;

    std::unique_ptr<std::vector<uint8_t>> q_vec_aux;
    std::unique_ptr<std::vector<uint8_t>> w_vec_aux;
    std::unique_ptr<std::vector<uint8_t>> b_vec_aux;
    std::unique_ptr<std::vector<uint8_t>> l_vec_aux;

    std::unique_ptr<std::vector<uint8_t>> guard_vec;

    std::unique_ptr<std::vector<uint8_t>> event_vec;

    std::unique_ptr<std::vector<uint8_t>> aux_vec;
    /*
        Initial mark for discrete places
    */
    std::unique_ptr<std::vector<uint32_t>> initial_disc_mark;
    /*
        Current mark for discrete places
    */
    std::unique_ptr<std::vector<uint32_t>> disc_mark;
    /*
        Matrix for discrete place and transitions. Type: Incidence (I)
    */
    std::unique_ptr<PetriMatrix<int32_t>> disc_matrix;
    /*
        Matrix for discrete place and transitions. Type: Pre-incidence (I+)
    */
    std::unique_ptr<PetriMatrix<int32_t>> disc_pre_matrix;
    /*
        Matrix for discrete place and transitions. Type: Post-incedence (I-)
    */
    std::unique_ptr<PetriMatrix<int32_t>> disc_post_matrix;

    /*
        Inhibitor matrix
    */
    std::unique_ptr<PetriMatrix<uint8_t>> inhibitor_matrix;
    /*
        Inhibitor matrix
    */
    std::unique_ptr<PetriMatrix<uint8_t>> reader_matrix;

    /*
        Vector of sensitized transitions, if True the transition is able to fire.
    */
    std::unique_ptr<std::vector<uint8_t>> current_sensitized;

    std::unique_ptr<std::vector<uint8_t>> last_sensitized;

    /*
        Total number of places
    */
    uint32_t places        = 0;
    /*
        Total number of transitions
    */
    uint32_t transitions   = 0;

    std::string petri_name;

    int getValue(uint32_t place,uint32_t transition);

    /*
        Time unit object, reponsable to calculate all time staff
    */
   std::shared_ptr<TimeLogic> time_unit;
   /*
       For temporal network, a temporal transition has a temporal window, that means,
       it can only fire if is between alpha and beta.
   */
   std::unique_ptr<std::vector<std::pair<uint32_t, uint32_t>>> temporal_window;

   struct TimeVarianceAuthority {
       uint32_t alpha{0};
       uint32_t beta{0};
       std::thread::id agent_id;
       std::optional <std::chrono::high_resolution_clock::time_point> timestamp;
   };

   std::unique_ptr <std::unordered_map<uint32_t, TimeVarianceAuthority>> temporal_logic;
   void checkSpecialCase(uint32_t transition);
   void updateTimestamps();

public:
    void setName(std::string&& name);
    std::string& getName();
    /*
        Set number places for petri network
        @param places Set the total number of places
    */
    void setPlaces(uint32_t places) {
        this->places = places;
    }

    /*
        Set number transitions for petri network
        @param transitions Set the total number of transitions
    */
    void setTransitions(uint32_t transitions) {
        this->transitions = transitions;
    }    
    /*
        Dependency injection, move the new mark from the builder class or from the Monitor result.
    */
    void setMark(std::unique_ptr<std::vector<uint32_t>> new_mark);
    //brett.searles@attobotics.net
    /*
        Dependency injection, move the new mark from the builder class.
    */
    void setSensitized(std::unique_ptr<std::vector<uint8_t>> new_sensitized);

    void setInitialMark(std::unique_ptr<std::vector<uint32_t>> new_mark);
    
    void setDiscreteMatrixes(std::unique_ptr<PetriMatrix<int32_t>> incidence_matrix, std::unique_ptr<PetriMatrix<int32_t>> pre_matrix, std::unique_ptr<PetriMatrix<int32_t>> post_matrix);

    bool isSensitized(uint32_t transition_number);

    void setExtendedMatrixes(std::unique_ptr<PetriMatrix<uint8_t>> inhibitor, std::unique_ptr<PetriMatrix<uint8_t>> writer);

    void setAuxiliarsVectors(std::unique_ptr<std::vector<uint8_t>> b_vector, std::unique_ptr<std::vector<uint8_t>> l_vector);

    void setGuarAndEvent(std::unique_ptr<std::vector<uint8_t>> guards, std::unique_ptr<std::vector<uint8_t>> events);
    
    /*
        Establish time scale unit
    */
    void setTimeLogic(std::shared_ptr<TimeLogic> scale);

    void setTemporalWindow(std::unique_ptr<std::vector<std::pair<uint32_t, uint32_t>>> z_matrix);

    int64_t isTemporalSensitized(uint32_t transition);

    bool isEarly(uint32_t transition,const std::chrono::high_resolution_clock::time_point &timestamp);

    void updateSensitized();

    bool stillSensitized();

    std::vector<uint32_t>* getMark();

    std::vector<uint32_t>& getInitialMark();

    std::vector<uint8_t>& getCurrenSensitized();

    const std::vector<int>& getRow(uint32_t row_number);

    const std::vector<int>& getRowPre(uint32_t row_number);

    const std::vector<int>& getRowPost(uint32_t row_number);

    void printMark();

    void setMark(std::vector<uint32_t>* mark);

    void resetNetworkStatus();

    uint32_t getTransitions() const{
        return transitions;
    }

    uint32_t getPlaces() const {
        return places;
    }

    std::shared_ptr<std::vector<uint8_t>> applyExtendedEquation(std::vector<uint32_t>* mark,std::vector<int32_t>* row_pre);
};