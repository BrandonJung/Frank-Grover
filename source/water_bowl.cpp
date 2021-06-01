#include "water_bowl.hpp"
#include "render.hpp"

entt::entity WaterBowl::createWaterBowl(vec2 position)
{
	auto entity = _ecs.create();

	// create rendering components
	std::string key = "water_bowl";
	ShadedMesh& resource = cache_resource(key);
	if (resource.effect.program.resource == 0)
	{
		resource = ShadedMesh();
		RenderSystem::createSprite(resource, textures_path("water_bowl.png"),
								   "textured");
	}
	_ecs.emplace<ShadedMeshRef>(entity, resource);

	// Initialize the motion of the entity
	auto& motion = _ecs.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.position = position;
	motion.scale =
		vec2({0.15f, 0.15f}) * static_cast<vec2>(resource.texture.size);
	motion.depth = 1;

	// Create an (empty) component to be able to refer to the water_bowl
	_ecs.emplace<WaterBowl>(entity);

	return entity;
}