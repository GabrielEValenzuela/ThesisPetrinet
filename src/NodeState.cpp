#include "../include/NodeState.h"

void NodeState::make_hash()
{
	std::stringstream str_builder;
	str_builder << "[ ";
	for (auto m : *network_mark) {
		str_builder << m << " ";
	}
	str_builder << "]";
	hasher.update(str_builder.str());
	uuid = std::move(hasher.final());
}

NodeState::NodeState(int32_t fire, std::unique_ptr<std::vector<uint32_t>> mark, std::unique_ptr<std::vector<uint8_t>> sensitized, std::shared_ptr<NodeState> new_parent)
	:who_fire_me{fire},
	network_mark{std::move(mark)},
	network_sensitized{std::move(sensitized)},
	node_parent{new_parent}
{
	this->node_children = std::make_unique< std::vector<std::shared_ptr<NodeState>>>();
	make_hash();
	this->deep = node_parent->getDeep()+1;
}

uint8_t NodeState::isActive()
{
	return is_node_active;
}

uint8_t NodeState::isParent()
{
	return node_children->size() > 0;
}

uint8_t NodeState::isAccelerated(){
	return is_accelerated;
}

void NodeState::setAccelerated(){
	is_accelerated = 1;
}

uint16_t NodeState::getDeep(){
	return deep;
}

void NodeState::addChildren(std::shared_ptr<NodeState> new_child)
{
	node_children->push_back(new_child);
}

void NodeState::activateNode()
{
	this->is_node_active = 1;
}

void NodeState::deactivateNode()
{
	this->is_node_active = 0;
}

std::string& NodeState::getNodeId()
{
	return uuid;
}

std::vector<uint8_t>* NodeState::getSensitizedAssociate()
{
	return network_sensitized.get();
}

std::vector<uint32_t>* NodeState::getMarkAssociate()
{
	return network_mark.get();
}

std::vector<std::shared_ptr<NodeState>>& NodeState::getChildren()
{
	return *node_children;
}

NodeState* NodeState::getParentNode()
{
	return node_parent.get();
}
