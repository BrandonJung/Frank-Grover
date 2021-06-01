#include "pathfinder.hpp"
#include <queue>
#include <iostream>

namespace Pathfinder
{
	// search a path between two entities, instead of two locations
	// different as it avoids entities except the ones involved in the path
	std::vector<vec2> searchPathBetweenEntities(Grid graph, entt::entity start, entt::entity end)
	{

		auto& startMotion = _ecs.get<Motion>(start);
		auto& endMotion = _ecs.get<Motion>(end);

		/*std::cout << "path start: (" << startMotion.position.x << ", " << startMotion.position.y << ")" << std::endl;
		std::cout << "path end: (" << endMotion.position.x << ", "
				  << endMotion.position.y << ")" << std::endl;*/
				
		Node* first = graph
						  .grid[(int)std::floor(startMotion.position.x / graph.scale)]
							   [(int)std::floor(startMotion.position.y / graph.scale)]
						  .get();
		Node* last = graph
						 .grid[(int)std::floor(endMotion.position.x / graph.scale)]
							  [(int)std::floor(endMotion.position.y / graph.scale)]
						 .get();

		std::map<Node*, Node*> pathHistory;
		std::map<Node*, float> costHistory;
		std::vector<vec2> path;

		std::priority_queue<Node*, std::vector<Node*>, std::less<Node*>>
			frontier;

		first->priority = 0;
		frontier.emplace(first);

		pathHistory.emplace(first, first);
		costHistory.emplace(first, 0.f);

		Node* current = nullptr;

		while (!frontier.empty())
		{
			current = frontier.top();
			frontier.pop();
			if (current->position == last->position)
				break;

			for (auto neighbor : graph.getNeighborsWithoutEntities(current, start, end))
			{
				float cost = costHistory[current] + 1;
				if (!(costHistory.find(neighbor) != costHistory.end()) ||
					(cost < costHistory[neighbor]))
				{
					costHistory[neighbor] = cost;
					pathHistory[neighbor] = current;
					neighbor->priority = cost + heuristic(neighbor, last);
					frontier.emplace(neighbor);
				}
			}
		}

		if (current != nullptr)
		{
			while (current->position != first->position)
			{
				path.push_back({current->position.x, current->position.y});
				current = pathHistory[current];
			}
		}

		std::reverse(path.begin(), path.end());
		return path;
	}

	// calculate the path from start to end in the given graph
	// start and end are in pixels, convert them to grid resolution first
	std::vector<vec2> searchPath(Grid graph, vec2 start, vec2 end) 
	{
		Node *first = graph.grid[(int)std::floor(start.x / graph.scale)]
						  [(int)std::floor(start.y / graph.scale)].get();
		Node *last = graph.grid[(int)std::floor(end.x / graph.scale)]
						  [(int)std::floor(end.y / graph.scale)].get();

		std::map<Node*, Node*> pathHistory;
		std::map<Node*, float> costHistory;
		std::vector<vec2> path;

		std::priority_queue<Node*, std::vector<Node*>, std::less<Node*>> frontier;

		first->priority = 0;
		frontier.emplace(first);

		pathHistory.emplace(first, first);
		costHistory.emplace(first, 0.f);

		Node *current = nullptr;

		while (!frontier.empty())
		{
			current = frontier.top();
			frontier.pop();
			if (current->position == last->position)
				break;

			for (auto neighbor : graph.getNeighbors(current))
			{
				float cost = costHistory[current] + 1;
				if (!(costHistory.find(neighbor) != costHistory.end()) ||
					(cost < costHistory[neighbor]))
				{
					costHistory[neighbor] = cost;
					pathHistory[neighbor] = current;
					neighbor->priority = cost + heuristic(neighbor, last);
					frontier.emplace(neighbor);
				}
			}
		}

		if (current != nullptr)
		{
			while (current->position != first->position)
			{
				path.push_back({current->position.x, current->position.y});
				current = pathHistory[current];
			}
		}

		std::reverse(path.begin(), path.end());
		return path;
	}
	// calculate the manhattan distance between two nodes
	float heuristic(Node* a, Node* b) 
	{
		return abs(a->position.x - b->position.x) +
			   abs(a->position.y - b->position.y);
	}
}