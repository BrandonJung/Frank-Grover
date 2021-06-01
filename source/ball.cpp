#include "ball.hpp"
#include "render.hpp"
#include "physics.hpp"

entt::entity Ball::createBall(vec2 position)
{
	// Reserve en entity
	auto entity = _ecs.create();

	// Create the rendering components
	std::string key = "ball";
	ShadedMesh& resource = cache_resource(key);
	if (resource.effect.program.resource == 0)
	{
		resource = ShadedMesh();
		RenderSystem::createSprite(resource, textures_path("ball.png"),
								   "textured");
	}

	// Store a reference to the potentially re-used mesh object (the value is
	// stored in the resource cache)
	_ecs.emplace<ShadedMeshRef>(entity, resource);

	// Initialize the position, scale, and physics components
	auto& motion = _ecs.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.position = position;
	// Setting initial values, scale is negative to make it face the opposite
	// way
	motion.scale =
		vec2({0.1f, 0.1f}) * static_cast<vec2>(resource.texture.size);
    motion.depth = 1;
    
    _ecs.emplace<Interaction>(entity);
    
    // Ball has physics applied
    auto& physics = _ecs.emplace<Physics>(entity);
    physics.mass = 1.f;
    vec2 gravity = {0.f, 980.f * physics.mass};
    physics.forces += gravity;
    physics.groundLevel = motion.position.y + motion.scale.y / 2.f;
    physics.restitution = {0.6f, 0.75f};
    physics.originalScale = motion.scale;

	// Create an (empty) Ball component to be able to refer to all Balls
	_ecs.emplace<Ball>(entity);

	return entity;
}
