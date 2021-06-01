#pragma once

#include "ecs.hpp"
#include "common.hpp"

// Map building
struct Background
{
	// Creates all the associated render resources and default transform
	static entt::entity createBackground(vec2 position, std::string name);
	std::string type;
};
