#pragma once
#include <deque>
#include <limits>
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <sstream>

class NodeState{
	private:

	const int32_t OMEGA = std::numeric_limits<int32_t>::max();

	std::unique_ptr<std::vector<uint32_t>> network_mark;
	//Mark associate

	std::unique_ptr<std::vector<uint8_t>> network_sensitized;
	//Sensitized associate

	std::shared_ptr<NodeState> node_parent;
	//Parent node

	std::unique_ptr<std::deque<std::shared_ptr<NodeState>>> children;
	//Childs nodes

	uint16_t deep{0};
	//Deep's node

	uint16_t id_count{0};
	//Numerical id used to print the node

	bool is_ancestor{false};
	//Ancestor node flag (V2)

	bool is_accelerated{false};
	//Flag accelerated

	bool is_active{true};
	//Flag active node

	int32_t who_fire_me{0};
	//Who create the node

	uint16_t num_omegas{0};

	uint16_t num_not_omegas{0};
	public:
	NodeState(int32_t fire,std::shared_ptr<NodeState> new_parent,uint16_t new_deep){
        who_fire_me = fire;
        node_parent = new_parent;
        deep = new_deep;
        children = std::make_unique<std::deque<std::shared_ptr<NodeState>>>();
    }

	void markAncestors(){
		auto Node = this;
		while(Node != nullptr){
			Node->is_ancestor = true;
			Node = Node->getParentNode().get();
		}
	}

    void unmarkAncestors(){
		auto Node = this;
		while(Node != nullptr){
			Node->is_ancestor = false;
			Node = Node->getParentNode().get();
		}
	}

	void setMark(std::unique_ptr<std::vector<uint32_t>> mark){
        network_mark = std::move(mark);
    }

	void setSensitized(std::unique_ptr<std::vector<uint8_t>> sensitized){
        network_sensitized = std::move(sensitized);
    }

	uint8_t isSensitizedAt(uint32_t transition){
        return network_sensitized->at(transition);
    }

	void setNodeId(uint16_t id){
        id_count = id;
    }

	void setAccelerated(){
        is_accelerated = true;
    }

	void setInactive(){
        is_active = false;
    }

    void setActive(){
        is_active = true;
    }

	void addChild(std::shared_ptr<NodeState> child){
        children->emplace_back(child);
    }

    void popChild(){
        children->pop_back();
    }

    uint32_t isAccelerated(){
        return std::count_if(network_mark->begin(),network_mark->end(),[&](uint32_t m){return m==OMEGA;});
    }

	bool isActive(){
        return is_active;
    }

    int32_t getFire(){
        return who_fire_me;
    }

    std::deque<std::shared_ptr<NodeState>>* getChildren(){
        return children.get();
    }

    std::vector<uint32_t>* getMarkAssociate(){
        return network_mark.get();
    }

	uint16_t getNodeId() const{
        return id_count;
    }

    uint16_t getDeep() const{
        return deep;
    }

    std::shared_ptr<NodeState> getParentNode(){
        return node_parent;
    }

    void deactivateSubtree(){
        setInactive();
        for(auto const& child : *children){
            child->deactivateSubtree();
        }
    }

    void pruneNode(std::vector<uint32_t>* rhs_node){
        if(((!is_ancestor) || is_active) && this->hasSmallerMark(network_mark.get(),rhs_node)){
            deactivateSubtree();
        } else {
            for(auto const& child : *children){
                child->pruneNode(rhs_node);
            }
        }
    }

    bool hasSmallerMark(std::vector<uint32_t>* lhs_node, std::vector<uint32_t>* rhs_node){
    auto aux_vector = std::make_unique<std::vector<uint8_t>>(network_mark->size());
    std::transform(lhs_node->begin(), lhs_node->end(), rhs_node->begin(), aux_vector->begin(),
                   [](uint32_t lhs_value, uint32_t rhs_value)
                   {
                       return lhs_value <= rhs_value;
                   });
    return std::all_of(aux_vector->begin(), aux_vector->end(), [](uint8_t result)
                       { return result != 0; });
    }

    std::string getInfo(){
        std::stringstream str_builder;
        str_builder << "[";
        for (auto m : *this->network_mark) {
            if (m == OMEGA){
                str_builder << "W ";
            } else {
                str_builder << m << " ";
            }
        }
        str_builder << "]";
        return str_builder.str();
    }
    
};