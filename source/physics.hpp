#pragma once

#include "ecs.hpp"
#include "common.hpp"
#include "subject.hpp"
#include "observer.hpp"

// For objects with physics (gravity, etc.)
struct Physics
{
    vec2 forces = {0.f, 0.f};
    float mass = 1.f;
    bool enabled = false;
    float groundLevel = 0.f;
    vec2 restitution = {1.f, 1.f};
    float squash = 1.f;
    vec2 originalScale = {1.f, 1.f};
};

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem : Subject
{
public:
	void step(float elapsed_ms, vec2 window_size_in_game_units, vec2 map_size_in_game_units);

	void registerObserver(Observer* observer);
	void removeObserver(Observer* observer);
	void notifyObservers(Event event);
	void observersSize();

	// Stucture to store collision information
	struct Collision
	{
		entt::entity firstEntity; // the first object involved in the collision
		entt::entity
			secondEntity;     // the second object involved in the collision
		vec2 uncollideMotion; // the motion to add to the first object so it no
							  // longer collides with the second object
		Collision(entt::entity& first, entt::entity& second,
				  vec2 uncollideMotion) :
			firstEntity(first),
			secondEntity(second), uncollideMotion(uncollideMotion)
		{
		}
	};
};
