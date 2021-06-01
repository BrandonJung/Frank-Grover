#include "socks.hpp"
#include "render.hpp"

entt::entity Socks::createSocks(vec2 position)
{
	// Reserve en entity
	auto entity = _ecs.create();

	// Create the rendering components
	std::string key = "socks";
	ShadedMesh& resource = cache_resource(key);
	if (resource.effect.program.resource == 0)
	{
		resource = ShadedMesh();
		RenderSystem::createSprite(resource, textures_path("socks.png"),
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
		vec2({0.8f, 0.8f}) * static_cast<vec2>(resource.texture.size);
	motion.depth = 1;

	_ecs.emplace<Interaction>(
		entity); // just to designate that the dirt can be interacted with

	// Create an (empty) component to be able to refer to the socks
	_ecs.emplace<Socks>(entity);

	return entity;
}
