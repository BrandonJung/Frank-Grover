#include "informant_squirrel.hpp"
#include "render.hpp"

entt::entity InformantSquirrel::createInformantSquirrel(vec2 position)
{
	auto entity = _ecs.create();

	// create rendering components
	std::string key = "informant_squirrel";
	ShadedMesh& resource = cache_resource(key);
	if (resource.effect.program.resource == 0)
	{
		resource = ShadedMesh();
		RenderSystem::createSprite(resource, textures_path("informant_squirrel.png"),
								   "textured");
	}
	_ecs.emplace<ShadedMeshRef>(entity, resource);

	// Initialize the motion of the entity
	auto& motion = _ecs.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.position = position;
	// Setting initial values, scale is negative to make it face the opposite
	// way
	motion.scale =
		vec2({-0.5f, 0.5f}) * static_cast<vec2>(resource.texture.size);
	motion.depth = 1;

	// Create an (empty) component to be able to refer to the squirrel
	_ecs.emplace<InformantSquirrel>(entity);

	return entity;
}
