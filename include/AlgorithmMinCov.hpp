#pragma once
#include <algorithm>
#include <deque>
#include <limits>
#include <memory>
#include <unordered_set>
#include <string>
#include "AlgorithmEngine.hpp"
#include "NodeState.hpp"
#include "OmegaTransition.hpp"
#include "OutputParser.hpp"
#include "../lib/sha1.hpp"

using namespace std::chrono_literals;
class AlgorithmMinCov : public AlgorithmEngine{
    private:

    struct PotentialNode_t{
        std::shared_ptr<NodeState> parent_node;
        size_t transition;
    };

    SHA1 aux_hash_maker;

    uint32_t amount_created_nodes{0};

    uint32_t amount_accelerated_nodes{0};

    uint32_t amount_del_nodes{0};

    uint32_t max_deep{0};
    
    const int32_t OMEGA = std::numeric_limits<int32_t>::max();

    std::shared_ptr<PetriNetwork> petrinet;

    std::unique_ptr<std::vector<std::shared_ptr<OmegaTransition>>> transitions;

    std::unique_ptr<std::deque<PotentialNode_t>> unprocessed_set;

    std::unique_ptr<std::unordered_map<std::string, std::shared_ptr<NodeState>>> processed_set;

    std::unique_ptr<std::vector<std::shared_ptr<OmegaTransition>>> accelereted_set;

    std::unique_ptr<std::unordered_set<std::string>> filter_set;
    
    std::shared_ptr<NodeState> root_node;

    std::shared_ptr<NodeState> current_node;

    void runMinCov();

    /* 
        Adds new candidates ndoes with the transitions recheable from omega-marking of the current node
    */
    void updateFrontSet();

    /*
        Check if the node is dominated
    */
    bool isDominatedNode(std::vector<uint32_t>* mark,NodeState* current_node);

    void generateOmegaMarking(std::vector<uint32_t>* mark, PotentialNode_t& candidate);

    bool isGrowingUp(uint32_t mark_tokens,uint32_t place,PotentialNode_t& candidate);

    /*
        Apply omega marking
    */
    void accelerate(PotentialNode_t& candidate,std::vector<uint32_t>* mark);
    //Compare two node mark and decides if the right side if bigger than left hand side
    bool hasSmallerMark(std::vector<uint32_t>* lhs_node,std::vector<uint32_t>* rhs_node);

    std::string generateHash(std::vector<uint32_t>* marking);

    public:
    void runAlgorithm(std::shared_ptr<PetriNetwork> petri_instance) override;
};