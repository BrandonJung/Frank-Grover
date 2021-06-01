#pragma once

#include "ecs.hpp"
#include "common.hpp"

// Map building
struct Building
{
	// Creates all the associated render resources and default transform
	static entt::entity createBuilding(vec2 position, std::string name);
	std::string type;
    bool isTree = false;
};
