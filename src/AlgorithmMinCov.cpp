#include "../include/AlgorithmMinCov.h"

void AlgorithmMinCov::runAlgorithm(std::shared_ptr<PetriNetwork> petri_instance)
{
	this->petrinet			= petri_instance;
	std::cout << "Runing minimal coverability tree algorithm...\n";
	auto s = std::chrono::high_resolution_clock::now();
	start();
	auto e = std::chrono::high_resolution_clock::now();
	auto time = std::chrono::duration_cast<std::chrono::milliseconds>(e - s).count();
	std::cout << "***DONE***\t Algorithm minimal coverability tree has end successfully.\n";
	std::cout << "Take " << time << " ms\n";
	return;
}

void AlgorithmMinCov::start(){
	setupAlgorithm();
	while (unprocessed_set->size()>0)
	{
		auto current_node = bfs_trasversal ? unprocessed_set->begin()->second : unprocessed_set->end()->second;
		auto current_parent_node = current_node->getParentNode();

		checkAccelerationsSet(current_node);
	}
	
}

void AlgorithmMinCov::setupAlgorithm(){
	this->unprocessed_set 	= std::make_unique<std::unordered_map<std::string, std::shared_ptr<NodeState>>>();
	this->processed_set 	= std::make_unique< std::vector<std::shared_ptr<NodeState>>>();
	this->accelereted_set   = std::make_unique<std::map<uint32_t,std::unique_ptr<std::vector<int32_t>>>>();

	amount_places_pn 			= petrinet->getPlaces();
	amount_transitions_pn 		= petrinet->getTransitions();

	auto init_mark 			= std::make_unique< std::vector<uint32_t>>(amount_places_pn);
	auto init_sens 			= std::make_unique< std::vector<uint8_t>>(amount_transitions_pn);

	std::copy(std::execution::par,petrinet->getInitialMark().begin(), petrinet->getInitialMark().end(), init_mark->begin());
	std::copy(std::execution::par,petrinet->getCurrenSensitized().begin(), petrinet->getCurrenSensitized().end(), init_sens->begin());

	this->root 				= std::make_shared<NodeState>(-1, std::move(init_mark), std::move(init_sens), nullptr);

	processed_set->emplace_back(root);
	updateUnprocessedSet();
}

void AlgorithmMinCov::updateUnprocessedSet(){
	for(auto pair_unproc : *unprocessed_set){
		if(pair_unproc.second->isActive()){
			generateAllChildren(pair_unproc.second);
		}
	}
}

void AlgorithmMinCov::generateAllChildren(std::shared_ptr<NodeState>& parent_node){
	for (size_t t = 0; t < amount_transitions_pn; t++)
	{
		if(parent_node->getSensitizedAssociate()->at(t)){
			auto new_mark = std::make_unique<std::vector<uint32_t>>(amount_places_pn);
			auto new_sensitized = std::make_unique<std::vector<bool>>(amount_transitions_pn);

			MathEngine::fire(parent_node->getMarkAssociate(), petrinet->getRow(t), new_mark.get());

			petrinet->setMark(new_mark.get());

			std::copy(std::execution::par_unseq, petrinet->getCurrenSensitized().begin(), petrinet->getCurrenSensitized().end(), new_sensitized->begin());

			auto child_node = std::make_shared<NodeState>(t,std::move(new_mark), std::move(new_sensitized), parent_node);

			if (unprocessed_set->find(child_node->getNodeId()) == 0) {
				parent_node->addChildren(child_node);
				unprocessed_set->insert({ child_node->getNodeId(),child_node });
			}
		}
	}	
}

void AlgorithmMinCov::checkAccelerationsSet(std::shared_ptr<NodeState>& parent_node){
	
}

/*
void AlgorithmMinCov::run() {
	initSets();
	while (!unprocessed_set->empty()) {
		auto current_node = unprocessed_set->begin()->second;
		if (current_node->isActive()) {
			generateAllChildren(current_node);
			accelerateNode(current_node);
			if (isOmegaMark(current_node)) {
				accelerateNode(current_node);
				pruneNodes(current_node);
			}
			else {
				current_node->deactivateNode();
				processed_set->push_back(current_node);
				unprocessed_set->erase(current_node->getNodeId());
			}
		}
		else{
			unprocessed_set->erase(current_node->getNodeId());
		}
	}
}



void AlgorithmMinCov::accelerateNode(std::shared_ptr<NodeState>& node) {
	auto new_mark = std::make_unique<std::vector<uint32_t>>(amounPlaces);
	for(auto acc = 0; acc < accelerator_set->size();acc++){
		if (node->getSensitizedAssociate()->at(acc)){
			std::vector<int32_t>* accelerated_row = accelerator_set->at(acc).get();
			MathEngine::fire(node->getParentNode()->getMarkAssociate(),*accelerated_row,new_mark.get());
			if((*new_mark) != (*node->getMarkAssociate())){
				node->deactivateNode();
				node->setAccelerated();
				break;
			}
		}		

	}
}

uint8_t AlgorithmMinCov::isOmegaMark(std::shared_ptr<NodeState>& node)
{
	if (node->getParentNode() == nullptr) {
		return 0;
	}

	for (size_t index = 0; index < node->getMarkAssociate()->size(); index++) {
		if ((node->getParentNode()->getMarkAssociate()->at(index) * deep) < node->getMarkAssociate()->at(index)) {
			return true;
		}
	}
	return 1;
}

void AlgorithmMinCov::pruneNodes(std::shared_ptr<NodeState>& node)
{
	for (auto child : node->getChildren()) {
		child->deactivateNode();
	}
	node->deactivateNode();
}

void AlgorithmMinCov::updateUnprocessedSet() {
	for (auto unprocessed_element : *unprocessed_set) {
		if (unprocessed_element.second->isActive()) {
			generateAllChildren(unprocessed_element.second);
		}
	}
}

void AlgorithmMinCov::generateAllChildren(std::shared_ptr<NodeState>& parent_node) {
	//For every enable transition, calculate the new mark, new sensitized vector and create a new node
	//Append the new node to the parent node
	//If the new node it's not in the unprocessed set, it's should be add.
	for (int32_t potential_transition = 0; potential_transition < parent_node->getSensitizedAssociate()->size(); potential_transition++) {
		if (parent_node->getSensitizedAssociate()->at(potential_transition)) {
			auto new_mark = std::make_unique<std::vector<uint32_t>>(amounPlaces);
			auto new_sensitized = std::make_unique<std::vector<bool>>(amountTransitions);
			MathEngine::fire(parent_node->getMarkAssociate(), petrinet->getRow(potential_transition), new_mark.get());
			petrinet->setMark(new_mark.get());
			std::copy(std::execution::par_unseq, petrinet->getCurrenSensitized().begin(), petrinet->getCurrenSensitized().end(), new_sensitized->begin());
			auto child_node = std::make_shared<NodeState>(potential_transition,std::move(new_mark), std::move(new_sensitized), parent_node);
			parent_node->addChildren(child_node);
			if (unprocessed_set->find(child_node->getNodeId()) == 0) {
				unprocessed_set->insert({ child_node->getNodeId(),child_node });
			}
		}
	}
}


void AlgorithmMinCov::initSets()
{
	this->unprocessed_set 	= std::make_unique<std::unordered_map<std::string, std::shared_ptr<NodeState>>>();
	this->processed_set 	= std::make_unique< std::vector<std::shared_ptr<NodeState>>>();
	this->accelerator_set   = std::make_unique<std::vector<std::unique_ptr<std::vector<int32_t>>>>();
	amounPlaces 			= petrinet->getPlaces();
	amountTransitions 		= petrinet->getTransitions();
	auto init_mark 			= std::make_unique< std::vector<uint32_t>>(amounPlaces);
	auto init_sens 			= std::make_unique< std::vector<uint8_t>>(amountTransitions);
	std::copy(std::execution::par,petrinet->getInitialMark().begin(), petrinet->getInitialMark().end(), init_mark->begin());
	std::copy(std::execution::par,petrinet->getCurrenSensitized().begin(), petrinet->getCurrenSensitized().end(), init_sens->begin());
	this->root 				= std::make_shared<NodeState>(-1, std::move(init_mark), std::move(init_sens), nullptr);
	processed_set->emplace_back(root);
}*/