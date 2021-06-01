#include "unicyclist.hpp"
#include "render.hpp"

entt::entity Unicyclist::createUnicyclist(vec2 position)
{
	auto entity = _ecs.create();

	// create rendering components
	std::string key = "unicyclist";
	ShadedMesh& resource = cache_resource(key);
	if (resource.effect.program.resource == 0)
	{
		resource = ShadedMesh();
		RenderSystem::createSprite(resource, textures_path("unicyclist.png"),
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
		vec2({0.9f, 0.9f}) * static_cast<vec2>(resource.texture.size);
	motion.depth = 1;

	// Create an (empty) component to be able to refer to the unicyclist
	_ecs.emplace<Unicyclist>(entity);

	return entity;
}
