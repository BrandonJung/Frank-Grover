#include "bone.hpp"
#include "render.hpp"

char* asset_paths[7] = {"skull.png", "scapula.png", "vertebra1.png", "vertebra2.png", "vertebra3.png", "vertebra4.png", "flipper.png"};

vec2 getScale(ShadedMesh& resource, BoneAsset type)
{
	vec2 scale = static_cast<vec2>(resource.texture.size);
	switch (type)
	{
	case SKULL:
		scale *= vec2({-0.8f, 0.8f});
		break;
	case SCAPULA:
	case VERTEBRA_4:
		scale *= vec2({-0.4f, 0.4f});
		break;
	case VERTEBRA_1:
	case VERTEBRA_2:
	case VERTEBRA_3:
		scale *= vec2({0.2f, 0.2f});
		break;
    case FLIPPER:
        scale *= vec2({0.17f, 0.17f});
        break;
	}
	return scale;
}

Bone::Bone(BoneAsset type) { this->type = type; }

entt::entity Bone::createBone(vec2 position, BoneAsset assetIndex) 
{
	// Reserve en entity
	auto entity = _ecs.create();

	// Create the rendering components
	std::string path = asset_paths[assetIndex];
	ShadedMesh& resource = cache_resource(path);
	if (resource.effect.program.resource == 0)
	{
		resource = ShadedMesh();
		std::cout << "Creating bone, asset index is " << assetIndex
				  << " path is " << path << std::endl;
		RenderSystem::createSprite(resource, textures_path(path),
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
	motion.scale = getScale(resource, assetIndex);
    if (assetIndex != SKULL)
        motion.depth = 50;      // so can hide behind buildings
    else
        motion.depth = 1;
    
    _ecs.emplace<Interaction>(entity);

	// Create an (empty) Bone component to be able to refer to all Bone
	_ecs.emplace<Bone>(entity, assetIndex);

	return entity;
}
