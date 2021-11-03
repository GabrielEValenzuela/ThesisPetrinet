#include "../include/AlgorithmMinCov.h"
using namespace std::chrono_literals;
void AlgorithmMinCov::runAlgorithm(std::shared_ptr<PetriNetwork> petri_instance)
{
	this->petrinet = petri_instance;
	std::cout << "Runing minimal coverability tree algorithm...\n";
	auto s = std::chrono::high_resolution_clock::now();
	start();
	auto e = std::chrono::high_resolution_clock::now();
	auto time = std::chrono::duration_cast<std::chrono::milliseconds>(e - s).count();
	std::cout << "***DONE***\t Algorithm minimal coverability tree has end successfully.\n";
	std::cout << "Take " << time << " ms\n";
	std::cout << "Amount nodes processed: "<<amount_processed_nodes<<"\t Amount nodes accelerated: "<<amount_accelerated_nodes<<"\n";
	std::cout << "Printing nodes...\n";
	std::this_thread::sleep_for(1s);
	for(auto node : *processed_set){
		std::cout << node;
	}

	return;
}

void AlgorithmMinCov::start()
{
	setupAlgorithm();
	std::cout<<"Start algorithm\n";
	while (unprocessed_set->size() > 0)
	{
		auto parent_node = bfs_trasversal ? unprocessed_set->begin()->second : unprocessed_set->end()->second;
		std::cout<<"Processing parent node:\n"<<parent_node<<"\n\n";
		if (parent_node->isActive())
		{
			std::cout<<"Its active\n";
			generationPhase(parent_node.get());

			std::cout<<"generationPhase pass\n"<<parent_node<<"\n\n";

			auto current_node = bfs_trasversal ? parent_node->getChildren().begin()->get() : parent_node->getChildren().end()->get();

			std::cout<<"Processing current node:\n"<<current_node<<"\n\n";

			explorationPhase(current_node);

			std::cout<<"explorationPhase pass\n"<<current_node<<"\n\n";

			if(current_node->isActive()){
				accelerationPhase(current_node);
				std::cout<<"accelerationPhase pass\n"<<current_node<<"\n\n";
			} else {
				std::cout<<"Node not accelerated...generating his children and deactivating him\n";
				processed_set->emplace_back(current_node);
				generationPhase(current_node);
				current_node->deactivateNode();
				parent_node->deactivateNode();
			}
		}

		else
		{
			unprocessed_set->erase(parent_node->getNodeId());
		}
	}
}

void AlgorithmMinCov::generationPhase(NodeState* parent_node)
{
	for (size_t t = 0; t < amount_transitions_pn; t++)
	{
		if (parent_node->getSensitizedAssociate()->at(t))
		{
			auto new_mark = std::make_unique<std::vector<uint32_t>>(amount_places_pn);
			auto new_sensitized = std::make_unique<std::vector<uint8_t>>(amount_transitions_pn);

			MathEngine::fire(parent_node->getMarkAssociate(), petrinet->getRow(t), new_mark.get());

			petrinet->setMark(new_mark.get());

			std::copy(std::execution::par_unseq, petrinet->getCurrenSensitized().begin(), petrinet->getCurrenSensitized().end(), new_sensitized->begin());

			auto child_node = std::make_shared<NodeState>(t, std::move(new_mark), std::move(new_sensitized), parent_node);

			if (unprocessed_set->find(child_node->getNodeId()) == 0)
			{
				parent_node->addChildren(child_node);
				unprocessed_set->insert({child_node->getNodeId(), child_node});
			}
		}
	}
}

void AlgorithmMinCov::explorationPhase(NodeState *current_node)
{
	auto current_mark = current_node->getMarkAssociate();
	for (auto node : *unprocessed_set)
	{
		if (node.second.get() != current_node)
		{
			auto node_mark = node.second->getMarkAssociate();
			for (auto token = 0; token < current_mark->size(); token++)
			{
				if (current_mark->at(token) < node_mark->at(token))
				{
					//There is a bigger mark, the node is redundant and it's deactivated
					current_node->deactivateNode();
					return;
				}
				else
				{
					//The node covers another node mark
					node.second->deactivateNode();
					continue;
				}
			}
		}
	}
}

void AlgorithmMinCov::accelerationPhase(NodeState* current_node){
	bool has_been_accelerated = false;
	for(auto place = 0; place < current_node->getMarkAssociate()->size();place++){
		if(total_tokens_incidence->at(place)>total_tokens_preincidence->at(place)){
			current_node->getMarkAssociate()->at(place) = OMEGA;
			has_been_accelerated = true;
		}
	}
	if(has_been_accelerated){
		current_node->setAccelerated();
		current_node->deactivateNode();
		processed_set->emplace_back(current_node);
		amount_accelerated_nodes++;
	}
}

void AlgorithmMinCov::setupAlgorithm()
{
	this->unprocessed_set = std::make_unique<std::unordered_map<std::string, std::shared_ptr<NodeState>>>();
	this->processed_set = std::make_unique<std::vector<std::shared_ptr<NodeState>>>();
	this->accelereted_set = std::make_unique<std::map<uint32_t, std::unique_ptr<std::vector<int32_t>>>>();

	amount_places_pn = petrinet->getPlaces();
	amount_transitions_pn = petrinet->getTransitions();

	total_tokens_incidence = std::make_unique<std::vector<int32_t>>(amount_places_pn);

	total_tokens_preincidence = std::make_unique<std::vector<int32_t>>(amount_places_pn);

	for(auto t = 0; t < amount_transitions_pn;t++){
		auto row = petrinet->getRow(t);
		auto row_pre = petrinet->getRowPre(t);
		for(auto p = 0; p < amount_places_pn;p++){
			total_tokens_incidence->at(p) += row.at(p);
			total_tokens_preincidence->at(p) += row_pre.at(p);
		}
	}

	auto init_mark = std::make_unique<std::vector<uint32_t>>(amount_places_pn);
	auto init_sens = std::make_unique<std::vector<uint8_t>>(amount_transitions_pn);

	std::copy(std::execution::par, petrinet->getInitialMark().begin(), petrinet->getInitialMark().end(), init_mark->begin());
	std::copy(std::execution::par, petrinet->getCurrenSensitized().begin(), petrinet->getCurrenSensitized().end(), init_sens->begin());

	this->root = std::make_shared<NodeState>(-1, std::move(init_mark), std::move(init_sens), nullptr);

	processed_set->emplace_back(root);
	unprocessed_set->insert({root->getNodeId(),root});
}

/*
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
			auto new_sensitized = std::make_unique<std::vector<uint8_t>>(amount_transitions_pn);

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

void AlgorithmMinCov::checkAccelerationsSet(std::shared_ptr<NodeState>& current_node){
	if(accelereted_set->count(current_node->getFire())){
		auto omega_transition = accelereted_set->at(current_node->getFire()).get();
		auto new_mark = std::make_unique<std::vector<uint32_t>>(amount_places_pn);

		MathEngine::fireOmega(omega_transition, new_mark.get());

		current_node->changeMarkAssociate(std::move(new_mark));

		current_node->setAccelerated();
	}
}

void AlgorithmMinCov::checkBiggerMark(std::shared_ptr<NodeState>& current_node){
	for(auto node : *unprocessed_set){
		for(auto p = 0; p < current_node->getMarkAssociate()->size();p++){
			if(current_node->getMarkAssociate()->at(p) < node.second->getMarkAssociate()->at(p)){
				current_node->deactivateNode();
				return;
			}
		}
	}
}



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