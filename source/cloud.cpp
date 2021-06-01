// Header
#include "cloud.hpp"
#include "render.hpp"

entt::entity Cloud::createCloud(vec2 position, vec2 scale, float parallax)
{
	// Reserve en entity
	auto entity = _ecs.create();

	// Initialize the position, scale, and physics components
	auto& motion = _ecs.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.position = position;
	motion.scale = scale;
	motion.depth = -999;

	// Initialize the AI component
	auto& ai = _ecs.emplace<AI>(entity);
	ai.collisionPosition = position;

	_ecs.emplace<Cloud>(entity, Cloud{parallax});

	return entity;
}
