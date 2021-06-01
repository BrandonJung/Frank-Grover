#include "litter.hpp"
#include "render.hpp"

char* assets[8] = {"bananaPeel.png", "fruitPeels.png", "cans.png",
						"shoes.png", "plasticCup.png", "paperCup.png",
						"bag.png", "teddy.png"};

entt::entity Litter::createLitter(vec2 position, LitterAsset assetIndex)
{
	// Reserve en entity
	auto entity = _ecs.create();

	// Create the rendering components
	std::string path = assets[assetIndex];
	ShadedMesh& resource = cache_resource(path);
	if (resource.effect.program.resource == 0)
	{
		resource = ShadedMesh();
		RenderSystem::createSprite(resource, textures_path(path), "textured");
	}

	// Store a reference to the potentially re-used mesh object (the value is
	// stored in the resource cache)
	_ecs.emplace<ShadedMeshRef>(entity, resource);

	// Initialize the position, scale, and physics components
	auto& motion = _ecs.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.position = position;
	motion.scale =
		vec2({1.5f, 1.5f}) * static_cast<vec2>(resource.texture.size);
	motion.depth = 2;

	_ecs.emplace<Interaction>(entity);

	// Create an (empty) Litter component to be able to refer to all Litter
	// and save the litter index so that can keep track which litter has
	// the player picked up
	auto& litter = _ecs.emplace<Litter>(entity);
	litter.asset = assetIndex;
	return entity;
}