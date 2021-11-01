#pragma once
#include <memory>
#include "PetriNetwork.hpp"
class AlgorithmEngine {
public:
	virtual void runAlgorithm(std::shared_ptr<PetriNetwork> petri_instance) = 0;
};