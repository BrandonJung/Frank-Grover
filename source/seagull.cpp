// Header
#include "seagull.hpp"
#include "render.hpp"

entt::entity Seagull::createSeagull(vec2 position)
{
	// Reserve en entity
	auto entity = _ecs.create();
    
    // Animator component
    auto& animator = _ecs.emplace<Animator>(entity);
    std::vector<float> walkAnimation = {2.f, 150.f, 0.f};
    animator.animations.push_back(walkAnimation);
    animator.max_num_frames = 2.f;

	// Create the rendering components
	std::string key = "seagull";
	ShadedMesh& resource = cache_resource(key);
	if (resource.effect.program.resource == 0)
	{
		resource = ShadedMesh();
		RenderSystem::createSprite(resource, textures_path("seagull_spritesheet.png"),
								   "textured", animator);
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
	motion.scale = vec2({0.5f / animator.max_num_frames, 0.5f / (float) animator.animations.size()}) * static_cast<vec2>(resource.texture.size);
    
    // Initialize the AI component
    auto& ai = _ecs.emplace<AI>(entity);
    ai.collisionPosition = position;

	// Create and (empty) Seagull component to be able to refer to all seagull
	_ecs.emplace<Seagull>(entity);

	return entity;
}
