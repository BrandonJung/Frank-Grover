#include "ecs.hpp"
#include "common.hpp"
#include "render_components.hpp"
#pragma once

struct Dumpster
{
	static entt::entity createDumpster(vec2 position);
};