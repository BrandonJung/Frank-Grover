#pragma once

#include <vector>

#include "common.hpp"
#include "ecs.hpp"
#include "grid.hpp"

// Data relevant to the AI entities
struct AI
{
    vec2 collisionPosition = {0, 0};
    bool fleeing = false;
};

struct PathFinder
{
	std::vector<vec2> path;
	int pathIndex = 0;
	bool pathInitialized = false;
};

class AISystem
{
public:
    Grid gameGrid;
	int updateAI = 50;
    void step(float elapsed_ms, vec2 window_size_in_game_units, int currentFrame);
};
