#pragma once

#include "common.hpp"
#include <vector>
#include "node.hpp"
#include <entt/entt.hpp>


class Grid
{
public:
	// width and height of the game map
	int width;
	int height;
	// dimension of grid nodes
	int scale;

	std::vector<std::vector<std::shared_ptr<Node>>> grid;

	Grid();
	
	void initGrid(int w, int h, int s);

	bool isInBounds(vec2 position);

	std::vector<Node*> getNeighbors(Node* n);

	std::vector<Node*> getNeighborsWithoutEntities(Node* n, entt::entity start,
												   entt::entity end);

	bool isNearOtherEntities(vec2 pos, entt::entity start, entt::entity end);

	int findCost(Node* n);

	bool isNearPlayer(vec2 pos);

	bool isNearSeagull(vec2 pos);

	bool isNearEnemy(vec2 pos);

	void markNodesBlocked();
};