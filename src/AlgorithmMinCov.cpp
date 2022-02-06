#include "../include/AlgorithmMinCov.hpp"

void AlgorithmMinCov::runAlgorithm(std::shared_ptr<PetriNetwork> petri_instance)
{
    this->petrinet = petri_instance;
    std::cout << "Running minimal coverability tree algorithm...\n";
    auto s = std::chrono::high_resolution_clock::now();
    runMinCov();
    auto e = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> time = e - s;
    std::cout << "***DONE***\t Algorithm minimal coverability tree has end successfully.\n";
    std::cout << "Take " << time.count() << " us\n";
    std::cout << "\n\n";
    OutputParser::MinCovOut(processed_set.get(),time.count(),petrinet.get());
    exit(0);
}

void AlgorithmMinCov::runMinCov()
{
    // Setup all containers
    unprocessed_set = std::make_unique<std::deque<PotentialNode_t>>();
    processed_set = std::make_unique<std::unordered_map<std::string, std::shared_ptr<NodeState>>>();
    accelereted_set = std::make_unique<std::vector<std::shared_ptr<OmegaTransition>>>();
    transitions = std::make_unique<std::vector<std::shared_ptr<OmegaTransition>>>();
    filter_set = std::make_unique<std::unordered_set<std::string>>();

    // Generate all OmegaTransitions from the petri instance
    for (auto t = 0; t < petrinet->getTransitions(); t++)
    {
        auto row_pre = std::make_unique<std::vector<int>>(petrinet->getPlaces());
        auto row_inc = std::make_unique<std::vector<int>>(petrinet->getPlaces());
        std::copy(petrinet->getRow(t).begin(), petrinet->getRow(t).end(), row_inc->begin());
        std::copy(petrinet->getRowPre(t).begin(), petrinet->getRowPre(t).end(), row_pre->begin());
        transitions->emplace_back(std::make_shared<OmegaTransition>(std::move(row_inc), std::move(row_pre), petrinet));
    }

    // Crete the root node
    auto init_mark = std::make_unique<std::vector<uint32_t>>(petrinet->getPlaces());
    std::copy(petrinet->getInitialMark().begin(), petrinet->getInitialMark().end(),
              init_mark->begin());
    auto init_sens = std::make_unique<std::vector<uint8_t>>(petrinet->getTransitions());
    std::copy(petrinet->getCurrenSensitized().begin(), petrinet->getCurrenSensitized().end(),
              init_sens->begin());

    root_node = std::make_shared<NodeState>(-1, nullptr, 0);
    root_node->setMark(std::move(init_mark));
    root_node->setSensitized(std::move(init_sens));
    processed_set->insert_or_assign(generateHash(root_node->getMarkAssociate()), root_node);
    root_node->setNodeId(processed_set->size());

    current_node = root_node;

    // 4 o 5 test con red acotadas y luego con snoopy hacer las pruebas omega.

    //===========================================================
    //=================== INIT ALGORITHM ========================
    //===========================================================

    updateFrontSet();

    // While there is unprocessed nodes...
    while (!unprocessed_set->empty())
    {
        auto potential_node = unprocessed_set->front();

        amount_created_nodes++;

        current_node = potential_node.parent_node;

        // Take one node, the parent node only if an active node...
        if (current_node->isActive()){
            //Generate the mark from the node...
            auto new_mark = std::make_unique<std::vector<uint32_t>>(petrinet->getPlaces());
            transitions->at(potential_node.transition)->fireFromMark(current_node->getMarkAssociate(), new_mark.get());

            //Remove candidate node from unprocessed_set...
            unprocessed_set->pop_front();

            if(filter_set->contains(generateHash(new_mark.get()))){
                continue;
            }

            // Step 1) Verify if the node is currently dominated...
            if(!isDominatedNode(new_mark.get(),root_node.get())){
                //if not dominated...

                // Step 2) Accelerate node
                accelerate(potential_node,new_mark.get());

                amount_accelerated_nodes++;

                auto child = std::make_shared<NodeState>(potential_node.transition,current_node,current_node->getDeep()+1);

                child->setMark(std::move(new_mark));

                processed_set->insert_or_assign(generateHash(child->getMarkAssociate()),child);

                child->setNodeId(processed_set->size());

                filter_set->insert(generateHash(child->getMarkAssociate()));

                current_node->addChild(child);

                current_node = child;

                current_node->markAncestors();

                root_node->pruneNode(current_node->getMarkAssociate());

                current_node->unmarkAncestors();

                current_node->setActive();

                updateFrontSet();
            }
        } else {
            unprocessed_set->pop_front();
        }
        
    }
}


void AlgorithmMinCov::updateFrontSet()
{
    for (uint32_t t = 0; t < petrinet->getTransitions(); t++)
    {
        if (transitions->at(t)->isSensitizedMark(current_node->getMarkAssociate()))
        {
            unprocessed_set->emplace_back(PotentialNode_t{current_node,t});
        }
    }
}

bool AlgorithmMinCov::isDominatedNode(std::vector<uint32_t>* mark,NodeState* currt_node)
{
    if(currt_node->isActive()){
        if(hasSmallerMark(mark,currt_node->getMarkAssociate())){
            return true;
        }
    }

    for(const auto& child : *currt_node->getChildren()){
        if(isDominatedNode(mark,child.get())){
            return true;
        }
    }

    return false;

}

void AlgorithmMinCov::accelerate(PotentialNode_t& candidate,std::vector<uint32_t>* mark){
    auto node = candidate.parent_node;
    while(node != nullptr){
        if(node->isActive() && (hasSmallerMark(node->getMarkAssociate(),mark))){
            generateOmegaMarking(mark,candidate);
        }
        node = node->getParentNode();
    }
}

void AlgorithmMinCov::generateOmegaMarking(std::vector<uint32_t>* mark, PotentialNode_t& candidate){
    for(size_t p = 0; p < petrinet->getPlaces();p++){
        if(petrinet->getRow(candidate.transition)[p]>petrinet->getRowPre(candidate.transition)[p]){
            mark->at(p) = OMEGA;
        }
        else if(petrinet->getRow(candidate.transition)[p] == 0){
            continue;
        }
        else if(petrinet->getRow(candidate.transition)[p]== 1 && isGrowingUp(mark->at(p),p,candidate)){
            mark->at(p) = OMEGA;
        }
    }
}

bool AlgorithmMinCov::isGrowingUp(uint32_t mark_tokens,uint32_t place,PotentialNode_t& candidate){
    auto parent = candidate.parent_node;
    bool alwaysSensitized = true;
    bool growUp = mark_tokens > 0;
    while(parent != nullptr){
        alwaysSensitized &= parent->isSensitizedAt(candidate.transition);
        growUp &= parent->getMarkAssociate()->at(place);
    }
    return growUp && alwaysSensitized;
}

std::string AlgorithmMinCov::generateHash(std::vector<uint32_t>* marking){
    std::stringstream str_builder;
    str_builder << "[";
    for (auto m : *marking) {
        if (m == OMEGA){
            str_builder << "W ";
        } else {
            str_builder << m << " ";
        }
    }
    str_builder << "]";
    aux_hash_maker.update(str_builder.str());
    return aux_hash_maker.final();
}

bool AlgorithmMinCov::hasSmallerMark(std::vector<uint32_t>* lhs_node, std::vector<uint32_t>* rhs_node)
{
    auto aux_vector = std::make_unique<std::vector<uint8_t>>(petrinet->getPlaces());
    std::transform(std::execution::par, lhs_node->begin(), lhs_node->end(), rhs_node->begin(), aux_vector->begin(),
                   [](uint32_t lhs_value, uint32_t rhs_value)
                   {
                       return lhs_value <= rhs_value;
                   });
    return std::all_of(aux_vector->begin(), aux_vector->end(), [](uint8_t result)
                       { return result != 0; });
}