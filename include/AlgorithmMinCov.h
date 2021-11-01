#pragma once
#include <limits>
#include <unordered_map>
#include <map>
#include <chrono>
#include <memory>
#include <unordered_set>
#include <execution>
#include "AlgorithmEngine.h"
#include "NodeState.h"
#include "../lib/MathEngine.hpp"


class AlgorithmMinCov : public AlgorithmEngine {
private:
    uint32_t amount_accelerated_nodes{0};
    uint32_t amount_processed_nodes {0};
    const int32_t OMEGA = std::numeric_limits<uint32_t>::max();
    std::shared_ptr<PetriNetwork> petrinet;
    //Front a.k.a unprocessed nodes
    std::unique_ptr<std::unordered_map<std::string, std::shared_ptr<NodeState>>> unprocessed_set;
    //MinCov a.k.a processed nodes
    std::unique_ptr<std::vector<std::shared_ptr<NodeState>>> processed_set;
    //Accelerated set of transitions
    std::unique_ptr<std::map<uint32_t,std::unique_ptr<std::vector<int32_t>>>> accelereted_set;
    //Root node
	std::shared_ptr<NodeState> root;

    uint32_t amount_places_pn;

    uint32_t amount_transitions_pn;

    bool bfs_trasversal {true};
    //==============================
    void generateAllChildren(std::shared_ptr<NodeState>& parent_node);

    void checkAccelerationsSet(std::shared_ptr<NodeState>& node);

    void checkBiggerMark(std::shared_ptr<NodeState>& node);

    void accelerateNode(std::shared_ptr<NodeState>& node);

    void deleteAllSmaller(std::shared_ptr<NodeState>& node);

    void generateOmegaTransitions(std::shared_ptr<NodeState>& node);

    void updateUnprocessedSet();

    void setupAlgorithm();

    void start();
    /*
    uint32_t deep = { 1 };
    std::shared_ptr<PetriNetwork> petrinet;
    std::unique_ptr<std::unordered_map<std::string, std::shared_ptr<NodeState>>> unprocessed_set;
    std::unique_ptr<std::vector<std::shared_ptr<NodeState>>> processed_set;
    std::unique_ptr<std::vector<std::unique_ptr<std::vector<int32_t>>>> accelerator_set;
    std::shared_ptr<NodeState> root;
    void generateAllChildren(std::shared_ptr<NodeState>& parent_node);
    void run();
    uint8_t isOmegaMark(std::shared_ptr<NodeState>& node);
    void accelerateNode(std::shared_ptr<NodeState>& node);
    void pruneNodes(std::shared_ptr<NodeState>& node);
    uint32_t thereIsSmallerThan(std::shared_ptr<NodeState>& node);
    bool isAGreaterMark(std::shared_ptr<NodeState> lnode, std::shared_ptr<NodeState> rnode);
    void initSets();
    void updateUnprocessedSet();
    bool check_for_smaller(std::shared_ptr<NodeState> node);
    uint32_t there_is_unprocess_nodes();
    uint32_t amounPlaces;
    uint32_t amountTransitions;*/
public:
	void runAlgorithm(std::shared_ptr<PetriNetwork> petri_instance) override;
    
};