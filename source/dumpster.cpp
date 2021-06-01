#include "dumpster.hpp"
#include "render.hpp"


entt::entity Dumpster::createDumpster(vec2 position) {

	auto entity = _ecs.create();

	std::string key = "dumpster";
	ShadedMesh& resource = cache_resource(key);
	if (resource.effect.program.resource == 0)
	{
		resource = ShadedMesh();
		RenderSystem::createSprite(resource, textures_path("dumpster.png"),
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
		vec2({1.5f, 1.5f}) * static_cast<vec2>(resource.texture.size);
	motion.depth = 2;

	_ecs.emplace<Dumpster>(entity);

	return entity;
}