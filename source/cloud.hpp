#pragma once

#include "ecs.hpp"
#include "common.hpp"
#include "ai.hpp"

// Cloud with parallax
struct Cloud
{
	float parallaxAmount;
	// Creates all the associated render resources and default transform
	static entt::entity createCloud(vec2 position, vec2 scale, float parallax);
};