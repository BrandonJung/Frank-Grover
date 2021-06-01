#pragma once

#include "ecs.hpp"
#include "common.hpp"
#include "ai.hpp"

// Salmon food
struct Seagull
{
	// Creates all the associated render resources and default transform
	static entt::entity createSeagull(vec2 position);
};
