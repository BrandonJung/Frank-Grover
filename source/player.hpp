#include "ecs.hpp"
#include "common.hpp"
#include "render_components.hpp"
#pragma once

// Main player
struct Player {
	static entt::entity createPlayer(vec2 pos);
	Mesh collisionMesh{};
};