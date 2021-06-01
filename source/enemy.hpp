#include "ecs.hpp"
#include "common.hpp"

struct Enemy
{
	// Creates all the associated render resources and default transform
	static entt::entity createEnemy(vec2 position, std::string name);
	bool takenBone;
	std::string name;
};