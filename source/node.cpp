#include "node.hpp"

Node::Node(int x, int y) 
{ 
	this->position.x = x;
	this->position.y = y;
	this->priority = 0;
	this->blocked = false;
}

bool Node::operator<(Node a) 
{ 
	return (this->priority < a.priority);
}