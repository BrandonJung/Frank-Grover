#pragma once

#include "ecs.hpp"
#include "common.hpp"

struct Waves
{
	// Creates all the associated render resources and default transform
	static entt::entity createWaves(vec2 position);
};