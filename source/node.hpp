#pragma once

#include <vector>
#include "common.hpp"

class Node
{
public:
	vec2 position;
	float priority;
	bool blocked;
	Node(int x, int y);
	bool operator <(Node a);
};
