#include "dirt.hpp"
#include "render.hpp"

entt::entity Dirt::createDirt(vec2 position)
{
	// Reserve en entity
	auto entity = _ecs.create();

	// Create the rendering components
	std::string key = "dirt";
	ShadedMesh& resource = cache_resource(key);
	if (resource.effect.program.resource == 0)
	{
		resource = ShadedMesh();
		RenderSystem::createSprite(resource, textures_path("dirt.png"),
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
		vec2({1.3f, 1.3f}) * static_cast<vec2>(resource.texture.size);
    motion.depth = 2;
    
    _ecs.emplace<Interaction>(entity);  // just to designate that the dirt can be interacted with

	// Create an (empty) component to be able to refer to all Dirt
	_ecs.emplace<Dirt>(entity);
	

	return entity;
}
