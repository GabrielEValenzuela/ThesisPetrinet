#pragma once
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include "../lib/sha1.hpp"

class NodeState {
private:
	SHA1 hasher;
	std::string uuid;
	std::unique_ptr<std::vector<uint32_t>> network_mark;
	std::unique_ptr<std::vector<uint8_t>> network_sensitized;
	std::unique_ptr<std::vector<std::shared_ptr<NodeState>>> node_children;
	std::shared_ptr<NodeState> node_parent;
	uint8_t is_node_active;
	uint8_t is_accelerated;
	int32_t who_fire_me;
	uint16_t deep;
	void make_hash();
public:
	NodeState(int32_t fire, std::unique_ptr<std::vector<uint32_t>> mark, std::unique_ptr<std::vector<uint8_t>> sensitized, std::shared_ptr<NodeState> new_parent);
	uint8_t isActive();
	uint8_t isParent();
	uint8_t isAccelerated();
	uint16_t getDeep();
	void addChildren(std::shared_ptr<NodeState> new_child);
	void activateNode();
	void deactivateNode();
	void setAccelerated();
	std::string& getNodeId();
	std::vector<uint8_t>* getSensitizedAssociate();
	std::vector<uint32_t>* getMarkAssociate();
	std::vector<std::shared_ptr<NodeState>>& getChildren();
	NodeState* getParentNode();
};