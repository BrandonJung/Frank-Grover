#include <vector>
#include <grid.hpp>
#include <map>
#include <entt/entt.hpp>
#include <ecs.hpp>

namespace Pathfinder
{
	std::vector<vec2> searchPathBetweenEntities(Grid graph, entt::entity start, entt::entity end);

	std::vector<vec2> searchPath(Grid graph, vec2 start, vec2 end);

	float heuristic(Node* a, Node* b);
}