#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include "PetriNetwork.hpp"
#include "ReadFile.hpp"
#include "../lib/PetriMatrix.hpp"
/*
    Builder is a creational design pattern that allows construct complex objects step by step.
    This patten produces different types and representations of an object using the same construction code.

    This is the Builder interface, that declares product construction steps that are common to all types of builders.
*/
class PetriBuilder{
    protected:
    /*
    *   Base information for Petrinetwork.
    *   The base petri network is the place transition
    */
    std::unique_ptr<PetriMatrix<int32_t>> pre_incidence_matrix;
    std::unique_ptr<PetriMatrix<int32_t>> post_incidence_matrix;
    std::unique_ptr<PetriMatrix<int32_t>> incidence_matrix;

    std::unique_ptr<std::vector<uint32_t>> initial_mark;
    std::unique_ptr<std::vector<uint32_t>> current_mark;
    std::unique_ptr<std::vector<uint8_t>> ex_sensitized_transitions;
    /*
    * The base petrinet can expand
    */

    //B = H*Q
    std::unique_ptr<PetriMatrix<uint8_t>> inhibitor_matrix;
    std::unique_ptr<std::vector<uint8_t>> q_vector;
    std::unique_ptr<std::vector<uint8_t>> b_vector;
    //L = R*W
    std::unique_ptr<PetriMatrix<uint8_t>> reader_matrix;
    std::unique_ptr<std::vector<uint8_t>> r_vector;
    std::unique_ptr<std::vector<uint8_t>> l_vector;

    std::unique_ptr<std::vector<uint8_t>> aux_vec;
    //Guard
    std::unique_ptr<std::vector<uint8_t>> guard_vector;
    //Event
    std::unique_ptr<std::vector<uint8_t>> event_vector;
    //Reset
    std::unique_ptr<std::vector<uint8_t>> reset_vector;

    /*
    *   If I have a temporal network, I'll need the Z matrix
    */
    std::unique_ptr<std::vector<std::pair<uint32_t, uint32_t>>> z_matrix;

    /*
    *   If I have a continuos network, I'll need a matrix and mark continuos
    */
    //std::unique_ptr<std::vector<float>> pre_incidence_matrix_cont;
    //std::unique_ptr<std::vector<float>> post_incidence_matrix_cont;
    //std::unique_ptr<std::vector<float>> incidence_matrix_cont;

    //std::unique_ptr<std::vector<float>> initial_mark_cont;
    //std::unique_ptr<std::vector<float>> current_mark_cont;

    /*
    *   If I have a colored network, I'll need a matrix and mark with tokens
    */

    //std::unique_ptr<std::vector<char>> pre_incidence_matrix_col;
    //std::unique_ptr<std::vector<char>> post_incidence_matrix_col;
    //std::unique_ptr<std::vector<char>> incidence_matrix_col;

    //std::unique_ptr<std::vector<char>> initial_mark_col;
    //std::unique_ptr<std::vector<char>> current_mark_col;

    /*
    *   To store capacity, I read the place and transition from file
    */
    std::unique_ptr<ReadFile> file;
    std::unique_ptr<std::map<uint32_t,std::pair<uint8_t, uint8_t>>> map_immediate_transitions;
    std::shared_ptr<PetriNetwork> petri_instance;
    uint32_t numberPlaces{ 0 };
    uint32_t numberTransitions{ 0 };

    bool hasInhibitorArcs;
    bool hasReadersArcs;
    bool hasResetArcs;
    
    public:
    virtual ~PetriBuilder() = default;
    
    virtual void setFile(std::unique_ptr<ReadFile> load_file) = 0;
    virtual std::shared_ptr<PetriNetwork> getPetrinet() = 0;

    virtual void buildStepPTMatrix() {};
    virtual void buildStepHMatrix() {};
    virtual void buildStepWMatrix() {};
    virtual void buildStepAVector() {};
    virtual void buildStepPTMark() {};
    virtual void buildStepSensitized() {};

    virtual void buildStepGuardAndEvent() {};

    virtual void buildStepInhibitor() {};
    virtual void buildStepReader() {};
    virtual void buildStepZMatrix() {};
    virtual void buildStepReset() {};

    virtual void buildStepCTMatrix() {};
    virtual void buildStepCTMark() {};

    virtual void buildStepCLMatrix() {};
    virtual void buildStepCLMark() {};

    virtual void buildNetwork() {};

    virtual void setDependencyInjection() {};

};