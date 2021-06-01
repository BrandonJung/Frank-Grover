#include "enemy.hpp"
#include "render.hpp"
#include "ai.hpp"

// TODO much of this creation code, and the code in player and building could be
// combined into one common function
entt::entity Enemy::createEnemy(vec2 position, std::string name)
{
	auto entity = _ecs.create();
    
    // Set animator component
    auto& animator = _ecs.emplace<Animator>(entity);

    std::vector<float> walkAnimation = {3.f, 120.f, 0.f};
    animator.animations.push_back(walkAnimation);
    animator.max_num_frames = 3.f;

	// Create rendering primitives
	std::string key = name;
	ShadedMesh& resource = cache_resource(key);
	std::string path = name + ".png";
	if (resource.effect.program.resource == 0)
		RenderSystem::createSprite(resource, textures_path("enemy_spritesheet.png"), "textured", animator);

	// Store a reference to the potentially re-used mesh object (the value is
	// stored in the resource cache)
	_ecs.emplace<ShadedMeshRef>(entity, resource);

	// Initialize the motion
	auto& motion = _ecs.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f}; // 200
	motion.position = position;
	motion.depth = 1; // should it be behind or in front of the player?
	// Setting initial values
    motion.scale = vec2({-1.f / animator.max_num_frames, 1.f / (float) animator.animations.size()}) * static_cast<vec2>(resource.texture.size);
	// Create and (empty) Turtle component to be able to refer to all turtles
	auto& enemy = _ecs.emplace<Enemy>(entity);

	// test for pathfinder
	auto& pathFinder = _ecs.emplace<PathFinder>(entity);

	enemy.name = name;
	enemy.takenBone = false;
	return entity;
}
