#pragma once
#include "PetriBuilder.hpp"
#include "../lib/util.hpp"
/*
    The director defines the order in which to call construction steps,
    so it can create and reuse specific configurations of products.
*/
class PetriDirector{
    public:
    void buildPlaceTransitionNet(PetriBuilder* pbuild){
        pbuild->buildStepPTMark();
        pbuild->buildStepPTMatrix();
        pbuild->buildStepHMatrix();
        pbuild->buildStepWMatrix();
        pbuild->buildStepSensitized();
        pbuild->buildNetwork();
    }

    void buildTemporalPlaceTransitionNet(PetriBuilder* pbuild){
        pbuild->buildStepPTMark();
        pbuild->buildStepPTMatrix();
        pbuild->buildStepHMatrix();
        pbuild->buildStepWMatrix();
        pbuild->buildStepSensitized();
        pbuild->buildNetwork();
    }
    
};