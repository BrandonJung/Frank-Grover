#include "ai.hpp"
#include "seagull.hpp"
#include "player.hpp"
#include "pathfinder.hpp"
#include "debug.hpp"
#include "rabbit.hpp"
#include "enemy.hpp"
#include "ball.hpp"
#include "litter_quest_girl.hpp"

//bool goingBottomRight = true;

const float minDist = 300.0f;
const float fleeDist = 350.0f;
const float speed = 300.0f;
const float rabbitSpeed = 300.0f;

vec2 currentDestination = {4000, 1500};

void avoidEntityAtPosition(vec2 position)
{
    for (auto seagull : _ecs.view<Seagull>())
    {
        auto& seagullMotion = _ecs.get<Motion>(seagull);
        auto& seagullAI = _ecs.get<AI>(seagull);

        vec2 displacement = seagullMotion.position - position;
        float distSquared = dot(displacement, displacement);

        if (distSquared < pow(minDist, 2.0f))
        {
            seagullAI.fleeing = true;
            seagullAI.collisionPosition = seagullMotion.position;
            vec2 direction = normalize(displacement);
            seagullMotion.velocity = speed * direction;
        }
        else
        {
            if (seagullAI.fleeing)
            {
                vec2 displacementFromOriginal =
                    seagullMotion.position - seagullAI.collisionPosition;
                float distSquaredFromOriginal =
                    dot(displacementFromOriginal, displacementFromOriginal);

                if (distSquaredFromOriginal > pow(fleeDist, 2.0f))
                {
                    seagullAI.fleeing = false;
                    seagullMotion.velocity = vec2(0.f, 0.f);
                }
            }
        }
    }
}

// function moves the entity on its path with the given speed
void traversePath(entt::entity traverser, float speed) 
{
	auto& motion = _ecs.get<Motion>(traverser);
	auto& pathFinder = _ecs.get<PathFinder>(traverser);

	if (pathFinder.path.size() > 0 && pathFinder.pathIndex <= pathFinder.path.size())
	{
		// first step
		if (pathFinder.pathIndex == 0)
		{
			vec2 nextStep = pathFinder.path[pathFinder.pathIndex];
			pathFinder.pathIndex++;

			vec2 displacement = nextStep - motion.position;
			float total = abs(displacement.x) + abs(displacement.y);
			float xFraction = displacement.x / total;
			float yFraction = displacement.y / total;
			motion.velocity.x = xFraction * speed;
			motion.velocity.y = yFraction * speed;
		}

		// end of path
		else if (pathFinder.pathIndex == pathFinder.path.size())
		{
			motion.velocity = {0, 0};
			pathFinder.pathIndex = 0;
			pathFinder.pathInitialized = false;
			pathFinder.path.clear();
		}

		else
		{
			// check if entity has reached the previous waypoint
			vec2 prevStep =	pathFinder.path[pathFinder.pathIndex - 1];
			vec2 displacement = prevStep - motion.position;
			float distanceFromWaypoint = dot(displacement, displacement);
			if (distanceFromWaypoint < pow(50, 2))
			{
				vec2 nextStep =	pathFinder.path[pathFinder.pathIndex];
				pathFinder.pathIndex++;
				vec2 displacement = nextStep - motion.position;
				float total = abs(displacement.x) + abs(displacement.y);
				float xFraction = displacement.x / total;
				float yFraction = displacement.y / total;
				motion.velocity.x = xFraction * speed;
				motion.velocity.y = yFraction * speed;
			}
		}
	}
}

void AISystem::step(float elapsed_ms, vec2 window_size_in_game_units, int currentFrame)
{
    (void)elapsed_ms; // placeholder to silence unused warning until implemented
    (void)window_size_in_game_units; // placeholder to silence unused warning until implemented
    
    auto player = _ecs.view<Player>()[0];
	auto& playerMotion = _ecs.get<Motion>(player);
	
	auto rabbit = _ecs.view<Rabbit>()[0];
	auto& rabbitMotion = _ecs.get<Motion>(rabbit);
	auto litterQuestGirl = _ecs.view<LitterQuestGirl>()[0];

	avoidEntityAtPosition(playerMotion.position);
	avoidEntityAtPosition(rabbitMotion.position);

	if (_ecs.view<Enemy>().size() > 0)
	{
		auto enemy = _ecs.view<Enemy>()[0];
		auto& enemyMotion = _ecs.get<Motion>(enemy);
		avoidEntityAtPosition(enemyMotion.position);
	}

    auto ballView = _ecs.view<Ball>();
    for (auto ball : ballView)
    {
        auto ballMotion = _ecs.get<Motion>(ball);
        avoidEntityAtPosition(ballMotion.position);
    }

	auto& rabbitPathfinder = _ecs.get<PathFinder>(rabbit);
	auto& litterQuestGirlMotion = _ecs.get<Motion>(litterQuestGirl);

	if (!rabbitPathfinder.pathInitialized)
	{
		/*rabbitPathfinder.path = Pathfinder::searchPath(gameGrid, rabbitMotion.position,
									  litterQuestGirlMotion.position);*/
		rabbitPathfinder.path = Pathfinder::searchPathBetweenEntities(
			gameGrid, rabbit, litterQuestGirl);
        rabbitPathfinder.pathInitialized = true;
		//std::cout << "Computed a new path\n";
    } 

	// test for pathfinder
	//auto& enemyPathfinder = _ecs.get<PathFinder>(enemy);
	//if (!enemyPathfinder.pathInitialized)
	//{
	//	enemyPathfinder.path = Pathfinder::searchPathBetweenEntities(
	//		gameGrid, enemy, player);
	//	//enemyPathfinder.pathInitialized = true;
	//	//std::cout << "Computed a new path\n";
	//} 

	if (DebugSystem::in_debug_mode)
	{
		if (rabbitPathfinder.path.size() > 1)
		{
			for (int i = 0; i < rabbitPathfinder.path.size() - 2; i++)
			{
				DebugSystem::createLine(rabbitPathfinder.path[i],
										rabbitPathfinder.path[i + 1]);
			}
		}

		/*if (enemyPathfinder.path.size() > 1)
		{
			for (int i = 0; i < enemyPathfinder.path.size() - 2; i++)
			{
				DebugSystem::createLine(enemyPathfinder.path[i],
										enemyPathfinder.path[i + 1]);
			}
		}*/
	}

    // update rabbit velocity
	traversePath(rabbit, rabbitSpeed);
	//if (rabbitPathfinder.path.size() > 0 && rabbitPathfinder.pathIndex <= rabbitPathfinder.path.size())
	//{
	//	if (rabbitPathfinder.pathIndex == 0)
	//	{
	//		vec2 nextStep = rabbitPathfinder.path[rabbitPathfinder.pathIndex];
	//		rabbitPathfinder.pathIndex++;

	//		vec2 displacement = nextStep - rabbitMotion.position;
	//		float total = abs(displacement.x) + abs(displacement.y);
	//		float xFraction = displacement.x / total;
	//		float yFraction = displacement.y / total;
	//		rabbitMotion.velocity.x = xFraction * rabbitSpeed;
	//		rabbitMotion.velocity.y = yFraction * rabbitSpeed;
 //       }

	//	else if (rabbitPathfinder.pathIndex == rabbitPathfinder.path.size())
	//	{
	//		rabbitMotion.velocity = {0, 0};
	//		rabbitPathfinder.pathIndex = 0;
	//		rabbitPathfinder.pathInitialized = false;
	//		rabbitPathfinder.path.clear();
	//		std::cout << "rabbitPathfinder.pathIndex reset to 0\n";

	//	}
 //       else
	//	{
	//		// check if rabbit has reached the previous waypoint
	//		vec2 prevStep =	rabbitPathfinder.path[rabbitPathfinder.pathIndex - 1];
	//		vec2 displacement = prevStep - rabbitMotion.position;
	//		float distanceFromWaypoint = dot(displacement, displacement);
	//		if (distanceFromWaypoint < pow(50, 2))
	//		{
	//			vec2 nextStep =
	//				rabbitPathfinder.path[rabbitPathfinder.pathIndex];
	//			rabbitPathfinder.pathIndex++;
	//			vec2 displacement = nextStep - rabbitMotion.position;
	//			float total = abs(displacement.x) + abs(displacement.y);
	//			float xFraction = displacement.x / total;
	//			float yFraction = displacement.y / total;
	//			rabbitMotion.velocity.x = xFraction * rabbitSpeed;
	//			rabbitMotion.velocity.y = yFraction * rabbitSpeed;
	//		}			
 //       }
 //   }
}
