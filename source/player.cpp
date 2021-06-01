#include "player.hpp"
#include "render.hpp"

entt::entity Player::createPlayer(vec2 pos)
{

	auto entity = _ecs.create();

	// Set animator component
	auto& animator = _ecs.emplace<Animator>(entity);

	// Now add an animation to the animator component's list of animations to
	// represent the wagging
	std::vector<float> wagAnimation = {2.f, 60.f, 0.f};
	std::vector<float> walkAnimation = {3.f, 120.f, 0.f};
    std::vector<float> wagWalkAnimation = {6.f, 60.f, 0.f};
	animator.animations.push_back(wagAnimation);
	animator.animations.push_back(walkAnimation);
    animator.animations.push_back(wagWalkAnimation);
	animator.max_num_frames = 6.f; // since the wag-walk animation has 6 frames


	// Create rendering primitives
	std::string key = "player";
	ShadedMesh& resource = cache_resource(key);
	if (resource.effect.program.resource == 0)
		RenderSystem::createSprite(resource,
								   textures_path("dog_spritesheet.png"),
								   "textured", animator); // CHANGED

	// Store a reference to the potentially re-used mesh object (the value is
	// stored in the resource cache)
	_ecs.emplace<ShadedMeshRef>(entity, resource);

	// Initialize the motion and all its transform variables
	auto& motion = _ecs.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.position = pos;
	motion.scale = vec2({-1.f / animator.max_num_frames, 1.f / (float) animator.animations.size()}) * static_cast<vec2>(resource.texture.size);
    
    _ecs.emplace<Interaction>(entity);
    
    _ecs.emplace<Night>(entity);

	// Create a component for the player
	Player& player = _ecs.emplace<Player>(entity);

	// Load the collision mesh
	player.collisionMesh.loadFromOBJFile(meshes_path("player_mesh.obj"));

	return entity;
}
