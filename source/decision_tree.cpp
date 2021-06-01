#include "decision_tree.hpp"
#include "player.hpp"
#include "enemy.hpp"
#include "ball.hpp"

#include <iostream>
#include <string>
#include <random>

// C++ random number generator (copied from world.hpp)
std::default_random_engine rng;
std::uniform_real_distribution<float> uniform_dist; // number between 0..1
float ENEMY_SPEED = 250.0;
const int PATROL_TIME = 200;
const int IDLE_TIME = 50; // these two could be randomized later

float ballPickUpRadius = 500.0;

int playerBoneCount = 0;
bool playerHasBall = false;
bool enemyChasingBall = false;

void DecisionTree::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	// if within bounds, change state to WANTS_BONE
	if (_ecs.view<Enemy>().size() == 0)
		return;
	updateState();
	act();
}

void DecisionTree::updateState(void)
{
	entt::entity player = _ecs.view<Player>()[0];
	auto& playerMotion = _ecs.get<Motion>(player);
	entt::entity e = _ecs.view<Enemy>()[0];
	auto& enemyMotion = _ecs.get<Motion>(e);
	auto& enemy = _ecs.get<Enemy>(e);

	auto motion = _ecs.view<Motion>();
	
	for (auto ball : _ecs.view<Ball>())
	{
		auto& ballMotion = _ecs.get<Motion>(ball);
		vec2 distance = ballMotion.position - enemyMotion.position;
		if (dot(distance, distance) < pow(ballPickUpRadius, 2.0f))
		{
			changeState(WANTS_BALL);
			enemyChasingBall = true;
			motionDirection = ballMotion.position - enemyMotion.position;
		}
	}
	if (!enemyChasingBall)
	{
		
		if (insidePatrolBoundaries(playerMotion.position) &&
			playerBoneCount > 0 && !enemy.takenBone)
		{
			changeState(WANTS_BONE);
			motionDirection = playerMotion.position - enemyMotion.position;
		}
		else if (!insidePatrolBoundaries(enemyMotion.position) ||
				 (enemy.takenBone && state == WANTS_BONE))
		{
			if (getPositiveOrNegativeFactor() == 1) // 50% chance to start
											   // patrolling or idling
			{
				changeState(PATROL);
				timer = PATROL_TIME;
			}
			else
			{
				changeState(IDLE);
				timer = IDLE_TIME;
			}
		}
		else if (state == IDLE && timer == 0)
		{
			changeState(PATROL);
			timer = PATROL_TIME;
		}
		else if ((state == PATROL && timer == 0))
		{
			changeState(IDLE);
			timer = IDLE_TIME;
		}
		// otherwise don't update state	
	}
}

void DecisionTree::changeState(EnemyState newState)
{
	//std::cout << "State change from " << state << " to " << newState << std::endl;
	state = newState;
}

int DecisionTree::getPositiveOrNegativeFactor(void)
{
	float x = uniform_dist(rng);
	if (x < 0.5)
	{
		return -1;
	}
	else
	{
		return 1;
	}
}

bool DecisionTree::insidePatrolBoundaries(vec2 position)
{
	return insideXPatrolBoundaries(position) &&
		   insideYPatrolBoundaries(position);
}

bool DecisionTree::insideXPatrolBoundaries(vec2 position)
{
	return position.x > lowerBounds.x && position.y > lowerBounds.y;
}

bool DecisionTree::insideYPatrolBoundaries(vec2 position)
{
	return position.x < upperBounds.x && position.y < upperBounds.y;
}

void DecisionTree::updateVelocityFromDirection()
{
	entt::entity enemy = _ecs.view<Enemy>()[0];
	auto& enemyMotion = _ecs.get<Motion>(enemy);
	float total = abs(motionDirection.x) + abs(motionDirection.y);
	float xFraction = motionDirection.x / total;
	float yFraction = motionDirection.y / total;
	enemyMotion.velocity =
		vec2(xFraction * ENEMY_SPEED, yFraction * ENEMY_SPEED);
}

void DecisionTree::act(void)
{
	entt::entity enemy = _ecs.view<Enemy>()[0];
	auto& enemyMotion = _ecs.get<Motion>(enemy);
	switch (state)
	{
	case IDLE:
		// decrement timer
		timer--;
		enemyMotion.velocity = vec2(0, 0);
		break;
	case PATROL:
		if (timer % PATROL_TIME == 0) // reduce the value we mod by to increase
									  // the frequency of changing directions
		{
			float x = uniform_dist(rng);
			float y = uniform_dist(rng);
			motionDirection = vec2(getPositiveOrNegativeFactor() * x,
								   getPositiveOrNegativeFactor() * y);
			updateVelocityFromDirection();
		}
		if (!insideXPatrolBoundaries(enemyMotion.position))
		{
			enemyMotion.velocity.x *= -1;
		}
		if (!insideYPatrolBoundaries(enemyMotion.position))
		{
			enemyMotion.velocity.y *= -1;
		}
		timer--;
		break;
	case WANTS_BONE:
		updateVelocityFromDirection();
		break;
	case WANTS_BALL:
		updateVelocityFromDirection();
		break;
	default:
		break;
	}
}

void DecisionTree::update(Event event)
{
	switch (event)
	{
	case BONE_COUNT_INCREASE:
		playerBoneCount += 1;
		ENEMY_SPEED = 250 + playerBoneCount * 7;
		// 490 is 10,000 - 3600 / 13
		// 275 is 6000 - 2400 / 13
		upperBounds[0] = 3600 + (490 * playerBoneCount);
		upperBounds[1] = 2400 + (275 * playerBoneCount);
		// 150 is 2000/13
		// 75 is 2000/13
		lowerBounds[0] = 2000 - (150 * playerBoneCount);
		lowerBounds[1] = 1000 - (75 * playerBoneCount);
		break;
	case BONE_COUNT_DECREASE:
		if (playerBoneCount > 0)
		{
			playerBoneCount -= 1;
		}
		break;
	case BALL_PICKED_UP:
		enemyChasingBall = false;
		changeState(PATROL);
		timer = PATROL_TIME;
		break;
	case BALL_DROPPED:
		playerHasBall = false;
		break;
	case BALL_PICKED_UP_BY_PLAYER:
		break;
	}
}
