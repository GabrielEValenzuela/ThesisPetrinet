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

    std::unique_ptr<std::vector<int32_t>> total_tokens_incidence;
    std::unique_ptr<std::vector<int32_t>> total_tokens_preincidence;

    uint32_t amount_places_pn;

    uint32_t amount_transitions_pn;

    bool bfs_trasversal {true};
    //==============================

    /*
        This algorithm defines four phases:

        generationPhase -> From the current node, generate all possible children from it only if active, else remove it.

        explorationPhase -> Look for smaller nodes, and deactivate them. If the current mark if the bigger
        try the acceleration.

        accelerationPhase -> For all p in P, for all t in T, check for t if the number of inputs for the place p
        is greater than the number of outputs. If this is True, then the place p for the transition t has a omega token.

        filterPhase -> If the node has been accelerated, deactivate and set this node as parent node, add node to MinCov
        and put the node on the unprocessed set.
    */

    void generationPhase(NodeState* parent_node);

    void explorationPhase(NodeState* current_node);

    void accelerationPhase(NodeState* current_node);

    void updateUnprocessedSet();

    void setupAlgorithm();

    void start();
    /*
    void generateAllChildren(std::shared_ptr<NodeState>& parent_node);

    void checkAccelerationsSet(std::shared_ptr<NodeState>& current_node);

    void checkBiggerMark(std::shared_ptr<NodeState>& current_node);

    void accelerateNode(std::shared_ptr<NodeState>& current_node);

    void deleteAllSmaller(std::shared_ptr<NodeState>& current_node);

    void generateOmegaTransitions(std::shared_ptr<NodeState>& current_node);
    
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