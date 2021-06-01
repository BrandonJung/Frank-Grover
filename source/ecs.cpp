#include "ecs.hpp"

#include "ai.hpp"
#include "background.hpp"
#include "ball.hpp"
#include "bone.hpp"
#include "building.hpp"
#include "cloud.hpp"
#include "conversation.hpp"
#include "debug.hpp"
#include "decision_tree.hpp"
#include "dirt.hpp"
#include "dumpster.hpp"
#include "enemy.hpp"
#include "grid.hpp"
#include "litter_quest_girl.hpp"
#include "litter.hpp"
#include "node.hpp"
#include "pathfinder.hpp"
#include "physics.hpp"
#include "player.hpp"
#include "rabbit.hpp"
#include "render.hpp"
#include "seagull.hpp"
#include "socks.hpp"
#include "ui.hpp"
#include "unicyclist.hpp"
#include "water_bowl.hpp"
#include "world.hpp"
#include "waves.hpp"

void printECSState(entt::registry& registry)
{
// For platform independant DEBUG conditional:
// https://stackoverflow.com/questions/8591762/ifdef-debug-with-cmake-independent-from-platform
#ifndef NDEBUG
	bool debugEntities = true;
	std::cout << "========================================" << std::endl;
	std::cout << "EnTT State:" << std::endl;
	registry.each([&](entt::entity entity) {
		std::cout << "  - Entity ["
				  << std::to_string(static_cast<ENTT_ID_TYPE>(entity)) << "]"
				  << std::endl;
		if (debugEntities)
		{
			std::cout << "      -> ";

			if (registry.has<AI>(entity))							std::cout << "AI, ";
			if (registry.has<Animator>(entity))						std::cout << "Animator, ";
			if (registry.has<Background>(entity))					std::cout << "Background, ";
			if (registry.has<Ball>(entity))							std::cout << "Ball, ";
			if (registry.has<Bone>(entity))							std::cout << "Bone, ";
			if (registry.has<Building>(entity))						std::cout << "Building, ";
			if (registry.has<Cloud>(entity))						std::cout << "Cloud, ";
			if (registry.has<Physics>(entity))						std::cout << "Physics, ";
			if (registry.has<PhysicsSystem::Collision>(entity))		std::cout << "Collision, ";
			if (registry.has<ColorTimer>(entity))					std::cout << "ColorTimer, ";
			if (registry.has<Conversation>(entity))					std::cout << "Conversation, ";
			if (registry.has<DebugLine>(entity))					std::cout << "DebugLine, ";
			if (registry.has<Dirt>(entity))							std::cout << "Dirt, ";
			if (registry.has<Dumpster>(entity))						std::cout << "Dumpster, ";
			if (registry.has<Enemy>(entity))						std::cout << "Enemy, ";
			if (registry.has<Interaction>(entity))					std::cout << "Interaction, ";
			if (registry.has<Litter>(entity))						std::cout << "Litter, ";
			if (registry.has<LitterQuestGirl>(entity))				std::cout << "LitterQuestGirl, ";
			if (registry.has<Motion>(entity))						std::cout << "Motion, ";
			if (registry.has<PathFinder>(entity))					std::cout << "PathFinder, ";
			if (registry.has<Player>(entity))						std::cout << "Player, ";
			if (registry.has<Rabbit>(entity))						std::cout << "Rabbit, ";
			if (registry.has<Seagull>(entity))						std::cout << "Seagull, ";
			if (registry.has<ShadedMeshRef>(entity))				std::cout << "ShadedMeshRef, ";
			if (registry.has<Socks>(entity))						std::cout << "Socks, ";
			if (registry.has<UILabel>(entity))						std::cout << "UILabel, ";
			if (registry.has<UIPanel>(entity))						std::cout << "UIPanel, ";
			if (registry.has<UIImage>(entity))						std::cout << "UIImage, ";
			if (registry.has<Unicyclist>(entity))					std::cout << "Unicyclist, ";
			if (registry.has<WaterBowl>(entity))					std::cout << "WaterBowl, ";
			if (registry.has<Waves>(entity))
				std::cout << "Waves, ";

			std::cout << std::endl;
		}
	});
	std::cout << "========================================" << std::endl;
#endif
}
