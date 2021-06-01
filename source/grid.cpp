#include "grid.hpp"
#include "ecs.hpp"
#include "player.hpp"
#include "seagull.hpp"
#include "building.hpp"
#include "enemy.hpp"
#include "dirt.hpp"
#include "bone.hpp"
#include "litter.hpp"
#include "ball.hpp"
#include "cloud.hpp"

// default constructor
Grid::Grid(){}

// create the grid over the game map
void Grid::initGrid(int w, int h, int s) 
{ 
	width = w;
	height = h;
	scale = s;

	int rows = (int)std::ceil(w / s);
	int columns = (int)std::ceil(h / s);

	// initialize the size of the grid
	this->grid = std::vector<std::vector<std::shared_ptr<Node>>>(
		rows, 
		std::vector<std::shared_ptr<Node>>(columns));

	// populate the grid with Nodes
	for (int x = 0; x < rows; x++)
	{
		for (int y = 0; y < columns; y++)
		{
			grid[x][y] = std::make_shared<Node>((int)((x*scale) + scale/2), (int)((y*scale) + scale/2));
		}
	}
	markNodesBlocked();
}

// check if a given position is inside the grid
bool Grid::isInBounds(vec2 position)
{
	return (position.x >= 0 && position.x < this->width && 
		position.y >= 0 && position.y < this->height);
}

// get the traversable neighbors of a node
std::vector<Node*> Grid::getNeighbors(Node* n) 
{ 
	std::vector<Node*> nodes;

	std::vector<vec2> moves = {
		vec2{-this->scale, 0}, // left
		vec2{this->scale, 0},  // right
		vec2{0, -this->scale}, // down
		vec2{0, this->scale}   // up
	};

	for (vec2 move : moves)
	{
		vec2 neighbour = n->position + move;
		int gridX = (neighbour.x - scale / 2) / scale;
		int gridY = (neighbour.y - scale / 2) / scale;
		if (isInBounds(neighbour) && !this->grid[gridX][gridY]->blocked
			&& !isNearEnemy(n->position) && !isNearPlayer(n->position) && !isNearSeagull(n->position))
		{
			nodes.emplace_back(grid[gridX][gridY].get());
		}
	}
	return nodes;
}

// get the traversable neighbors of a node considering the entities involved in path finding
std::vector<Node*> Grid::getNeighborsWithoutEntities(Node* n, entt::entity start, entt::entity end)
{
	std::vector<Node*> nodes;

	std::vector<vec2> moves = {
		vec2{-this->scale, 0}, // left
		vec2{this->scale, 0},  // right
		vec2{0, -this->scale}, // down
		vec2{0, this->scale}   // up
	};

	for (vec2 move : moves)
	{
		vec2 neighbour = n->position + move;
		int gridX = (neighbour.x - scale / 2) / scale;
		int gridY = (neighbour.y - scale / 2) / scale;
		if (isInBounds(neighbour) && !this->grid[gridX][gridY]->blocked &&
			!isNearOtherEntities(neighbour, start, end))
		{
			nodes.emplace_back(grid[gridX][gridY].get());
		}
	}
	return nodes;
}

bool Grid::isNearOtherEntities(vec2 pos, entt::entity start, entt::entity end)
{
	auto motionView = _ecs.view<Motion>();
	float minDistance = 40.f;
	for (auto entity : motionView)
	{
		if (entity != start && entity != end && 
			!_ecs.has<Dirt>(entity) && !_ecs.has<Seagull>(entity) &&
			!_ecs.has<Bone>(entity) && !_ecs.has<Litter>(entity) &&
			!_ecs.has<Ball>(entity) && !_ecs.has<Cloud>(entity) &&
			!_ecs.has<Building>(entity))
		{
			auto& entityMotion = _ecs.get<Motion>(entity);
			/*if ((pos.x > (entityMotion.position.x -
						  abs(entityMotion.scale.x / 2.f)) &&
				 pos.x < (entityMotion.position.x +
						  abs(entityMotion.scale.x / 2.f))) &&
				(pos.y > (entityMotion.position.y -
						  abs(entityMotion.scale.y / 2.f)) &&
				 pos.y < (entityMotion.position.y +
						  abs(entityMotion.scale.y / 2.f))))
				return false;*/
			vec2 distance = pos - entityMotion.position;
			float distanceSq = dot(distance, distance);
			if (distanceSq < pow(minDistance, 2.f))
				return true;
		}
		continue;
	}
	return false;
}

int Grid::findCost(Node* n) 
{ 
	if (isNearPlayer(n->position) || isNearSeagull(n->position) || isNearEnemy(n->position))
		return 3;
	else
		return 1;
}

// check if a position is near the player
bool Grid::isNearPlayer(vec2 pos) 
{
	auto player = _ecs.view<Player>()[0];
	auto& playerMotion = _ecs.get<Motion>(player);
	float minDistance = 200.f;
	vec2 distance = pos - playerMotion.position;
	float distanceSq = dot(distance, distance);
	if (distanceSq <= pow(minDistance, 2))
		return true;
	else
		return false;
}

// check if a position is near any seagull
bool Grid::isNearSeagull(vec2 pos)
{
	float minDistance = 200.f;
	for (auto seagull : _ecs.view<Seagull>())
	{
		auto& motion = _ecs.get<Motion>(seagull);
		vec2 distance = pos - motion.position;
		float distanceSq = dot(distance, distance);
		if (distanceSq <= pow(minDistance, 2))
			return true;
	}
	return false;
}

// check if a position is near any enemy
bool Grid::isNearEnemy(vec2 pos)
{
	float minDistance = 200.f;
	for (auto enemy : _ecs.view<Enemy>())
	{
		auto& motion = _ecs.get<Motion>(enemy);
		vec2 distance = pos - motion.position;
		float distanceSq = dot(distance, distance);
		if (distanceSq <= pow(minDistance, 2))
			return true;
	}
	return false;
}

// mark all grid nodes that have a static entity in them
void Grid::markNodesBlocked() 
{
	// iterate over all buildings (static entities)
	// convert their range of pixels to grid sized nodes
	// add all such nodes in the blockedNodes
	for (auto building : _ecs.view<Building>())
	{
		auto& motion = _ecs.get<Motion>(building);
		vec2 upperLeft = {motion.position.x - abs(motion.scale.x / 2),
						  motion.position.y - abs(motion.scale.y / 2)};
		vec2 lowerRight = {motion.position.x + abs(motion.scale.x / 2),
						   motion.position.y + abs(motion.scale.y / 2)};
		vec2 scaledUpperLeft = {std::floor(upperLeft.x / this->scale),
								std::floor(upperLeft.y / this->scale)};
		vec2 scaledLowerRight = {std::floor(lowerRight.x / this->scale),
								 std::floor(lowerRight.y / this->scale)};
		for (int x = (int)scaledUpperLeft.x; x < (int)scaledLowerRight.x; x++)
		{
			for (int y = (int)scaledUpperLeft.y; y < (int)scaledLowerRight.y; y++)
			{
				this->grid[x][y]->blocked = true;
			}
		}
	}
}

