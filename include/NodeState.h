#pragma once
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <utility>
#include <limits>


#include "../lib/sha1.hpp"

class NodeState {
private:
	SHA1 hasher;
	std::string uuid;
	const uint32_t OMEGA = std::numeric_limits<uint32_t>::max();
	std::unique_ptr<std::vector<uint32_t>> network_mark;
	std::unique_ptr<std::vector<uint8_t>> network_sensitized;
	std::unique_ptr<std::vector<std::shared_ptr<NodeState>>> node_children;
	NodeState* node_parent;
	uint8_t is_node_active{1};
	uint8_t is_accelerated{0};
	int32_t who_fire_me;
	uint16_t deep{0};
	void make_hash();
public:
	NodeState(int32_t fire, std::unique_ptr<std::vector<uint32_t>> mark, std::unique_ptr<std::vector<uint8_t>> sensitized, NodeState* new_parent);
	uint8_t isActive();
	uint8_t isParent();
	uint8_t isAccelerated();
	uint16_t getDeep();
	void addDeep();
	void addChildren(std::shared_ptr<NodeState> new_child);
	void activateNode();
	void deactivateNode();
	void deactiveDescendents();
	void setAccelerated();
	void changeMarkAssociate(std::unique_ptr<std::vector<uint32_t>> mark);
	std::string& getNodeId();
	std::vector<uint8_t>* getSensitizedAssociate();
	std::vector<uint32_t>* getMarkAssociate();
	std::vector<std::shared_ptr<NodeState>>& getChildren();
	NodeState* getParentNode();
	int32_t getFire();

	friend std::ostream& operator<<(std::ostream& os, NodeState* node) {
		os << "Node: "<<node->uuid<<"\n"
		<< "Acclerated? " << (node->is_accelerated==1 ? "yes" : "no")<< "\n"
		<< "Mark associate: [";
		for(auto tokens : *(node->network_mark)){
			if(tokens==node->OMEGA){
				os << " w ";
			} else {
				os << " " << tokens << " ";
			}
		}
		os << "]\n" << "Sensitized associate: [";
		for(auto tokens : *(node->network_sensitized)){
			os << (tokens == 1 ? "True" : "False") << " ";
		}
		os << "]\nAmount childrens: "<<node->node_children->size()<<"\t Who fire me was: "<<node->who_fire_me<<"\tDeep:"<<node->deep<<"\n";
		if(node->node_children->size()>0){
			os << "My child are: \n";
			for(auto child : *(node->node_children)){
				os << "\t" << child->uuid <<"\n";
			}
		}
		return os;
	}
};