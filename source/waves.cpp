#include "waves.hpp"
#include "render.hpp"
#include "background.hpp"

const float HEIGHT_TO_WIDTH_RATIO = 0.085;

entt::entity Waves::createWaves(vec2 position)
{
	auto entity = _ecs.create();

	// Initialize the motion
	auto& motion = _ecs.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f}; // 200
	motion.position = position;
	//motion.depth = 42; // arbitrary value for depth
	// Setting initial values
	float width = 15000.f;
	motion.scale =
			vec2({width, -HEIGHT_TO_WIDTH_RATIO * width}); 
	_ecs.emplace<Waves>(entity); 
	_ecs.emplace<Background>(entity); 
	return entity;
}