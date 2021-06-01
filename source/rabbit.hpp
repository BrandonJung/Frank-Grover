#pragma once

#include "ecs.hpp"
#include "common.hpp"

struct Rabbit
{
public:
	static entt::entity createRabbit(vec2 position);
};