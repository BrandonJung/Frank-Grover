#include "rabbit.hpp"
#include "render.hpp"
#include "ai.hpp"

entt::entity Rabbit::createRabbit(vec2 pos)
{

    auto entity = _ecs.create();

    // Set animator component
    auto& animator = _ecs.emplace<Animator>(entity);

    animator.animations[0] = {4.f, 60.f, 0.f};
    std::vector<float> lieAnimation = {4.f, 60.f, 0.f};
    std::vector<float> hopAnimation = {4.f, 80.f, 0.f};
    std::vector<float> runLieAnimation = {4.f, 60.f, 0.f};
    std::vector<float> crouchAnimation = {4.f, 60.f, 0.f};
    std::vector<float> standAnimation = {3.f, 60.f, 0.f};
    
    animator.animations.push_back(lieAnimation);
    animator.animations.push_back(hopAnimation);
    animator.animations.push_back(runLieAnimation);
    animator.animations.push_back(crouchAnimation);
    animator.animations.push_back(standAnimation);
    
    animator.max_num_frames = 4.f;

    // Create rendering primitives
    std::string key = "rabbit";
    ShadedMesh& resource = cache_resource(key);
    
    // SPRITE SHEET CREDIT: https://hiroredbird.itch.io/bunny-sprite-sheet
    if (resource.effect.program.resource == 0)
        RenderSystem::createSprite(resource,
                                   textures_path("bunny_spritesheet.png"),
                                   "textured", animator); // CHANGED

    // Store a reference to the potentially re-used mesh object (the value is
    // stored in the resource cache)
    _ecs.emplace<ShadedMeshRef>(entity, resource);

    // Initialize the motion and all its transform variables
    auto& motion = _ecs.emplace<Motion>(entity);
    motion.angle = 0.f;
    motion.velocity = {0.f, 0.f};
    motion.position = pos;
    motion.scale = vec2({-3.f / animator.max_num_frames, 3.f / (float) animator.animations.size()}) * static_cast<vec2>(resource.texture.size);

    // Create a component for the player
    _ecs.emplace<Rabbit>(entity);
  
    // add pathfinder component
	auto& pathFinder = _ecs.emplace<PathFinder>(entity);
    return entity;
}
