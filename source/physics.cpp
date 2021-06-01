// internal
#include "physics.hpp"
#include "debug.hpp"
#include "player.hpp"
#include "building.hpp"
#include "bone.hpp"
#include "ball.hpp"
#include "enemy.hpp"
#include "socks.hpp"
#include "cloud.hpp"
#include "background.hpp"

#include <string>
#include <iostream>

// Returns the local bounding coordinates scaled by the current size of the
// entity
vec2 get_bounding_box(const Motion& motion)
{
	// fabs is to avoid negative scale due to the facing direction.
	return {abs(motion.scale.x), abs(motion.scale.y)};
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding
// boxes and sees if the center point of either object is inside the other's
// bounding-box-circle. You don't need to try to use this technique.
vec2 collides(const Motion& motion1, const Motion& motion2)
{
	vec2 boundingBox1 = get_bounding_box(motion1);
	vec2 boundingBox2 = get_bounding_box(motion2);

	bool overlapsVert = false;
	bool overlapsHoriz = false;

	// this is the amount that entity 1 needs to add to its' motion to no longer
	// be colliding with entity 2
	vec2 uncollideMotion = vec2(0, 0);

	// 4 cases: motion1 is bottom left, bottom right, top left, or top right of
	// motion2
	if (motion1.position.x < motion2.position.x)
	{
		// 1 is left of 2
		if (motion1.position.x + boundingBox1.x / 2.0f >
			motion2.position.x - boundingBox2.x / 2.0f)
		{
			overlapsHoriz = true; // The right edge of 1's box is further right
								  // than the left edge of 2's box
			uncollideMotion.x = motion2.position.x - boundingBox2.x / 2.0f -
								(motion1.position.x + boundingBox1.x / 2.0f);
		}

		if (motion1.position.y < motion2.position.y)
		{
			// 1 is above 2
			if (motion1.position.y + boundingBox1.y / 2.0f >
				motion2.position.y - boundingBox2.y / 2.0f)
			{
				overlapsVert = true;
				uncollideMotion.y =
					motion2.position.y - boundingBox2.y / 2.0f -
					(motion1.position.y + boundingBox1.y / 2.0f);
			}
		}
		else
		{
			// 1 is below 2
			if (motion1.position.y - boundingBox1.y / 2.0f <
				motion2.position.y + boundingBox2.y / 2.0f)
			{
				overlapsVert = true;
				uncollideMotion.y =
					motion2.position.y + boundingBox2.y / 2.0f -
					(motion1.position.y - boundingBox1.y / 2.0f);
				/*motion2.position.y - boundingBox2.y / 2.0f -
				(motion1.position.y - boundingBox1.y / 2.0f);*/
			}
		}
	}
	else
	{
		// 1 is right of 2
		if (motion1.position.x - boundingBox1.x / 2.0f <
			motion2.position.x + boundingBox2.x / 2.0f)
		{
			overlapsHoriz = true;
			uncollideMotion.x = motion2.position.x + boundingBox2.x / 2.0f -
								(motion1.position.x - boundingBox1.x / 2.0f);
		}

		if (motion1.position.y < motion2.position.y)
		{
			// 1 is above 2
			if (motion1.position.y + boundingBox1.y / 2.0f >
				motion2.position.y - boundingBox2.y / 2.0f)
			{
				overlapsVert = true;
				uncollideMotion.y =
					motion2.position.y - boundingBox2.y / 2.0f -
					(motion1.position.y + boundingBox1.y / 2.0f);
			}
		}
		else
		{
			// 1 is below 2
			if (motion1.position.y - boundingBox1.y / 2.0f <
				motion2.position.y + boundingBox2.y / 2.0f)
			{
				overlapsVert = true;
				uncollideMotion.y =
					motion2.position.y + boundingBox2.y / 2.0f -
					(motion1.position.y - boundingBox1.y / 2.0f);
				/*motion2.position.y - boundingBox2.y / 2.0f -
				(motion1.position.y - boundingBox1.y / 2.0f);*/
			}
		}
	}

	return uncollideMotion;
}

vec2 vertex_collides(vec3 vertexPosition, const Motion& motion)
{
    vec2 boundingBox = get_bounding_box(motion);

    bool overlapsVert = false;
    bool overlapsHoriz = false;

    // this is the amount that entity 1 needs to add to its' motion to no longer
    // be colliding with entity 2
    vec2 uncollideMotion = vec2(0, 0);

    // 4 cases: motion1 is bottom left, bottom right, top left, or top right of
    // motion2
    if (vertexPosition.x < motion.position.x)
    {
        // 1 is left of 2
        if (vertexPosition.x > motion.position.x - boundingBox.x / 2.0f)
        {
            overlapsHoriz = true; // The vertex is further right than the left edge of motion's box
            uncollideMotion.x = motion.position.x - boundingBox.x / 2.0f - vertexPosition.x;
        }

        if (vertexPosition.y < motion.position.y)
        {
            // 1 is above 2
            if (vertexPosition.y > motion.position.y - boundingBox.y / 2.0f)
            {
                overlapsVert = true;
                uncollideMotion.y = motion.position.y - boundingBox.y / 2.0f - vertexPosition.y;
            }
        }
        else
        {
            // 1 is below 2
            if (vertexPosition.y < motion.position.y + boundingBox.y / 2.0f)
            {
                overlapsVert = true;
                uncollideMotion.y = motion.position.y + boundingBox.y / 2.0f - vertexPosition.y;
            }
        }
    }
    else
    {
        // 1 is right of 2
        if (vertexPosition.x <
            motion.position.x + boundingBox.x / 2.0f)
        {
            overlapsHoriz = true;
            uncollideMotion.x = motion.position.x + boundingBox.x / 2.0f - vertexPosition.x;
        }

        if (vertexPosition.y < motion.position.y)
        {
            // 1 is above 2
            if (vertexPosition.y > motion.position.y - boundingBox.y / 2.0f)
            {
                overlapsVert = true;
                uncollideMotion.y = motion.position.y - boundingBox.y / 2.0f - vertexPosition.y;
            }
        }
        else
        {
            // 1 is below 2
            if (vertexPosition.y <
                motion.position.y + boundingBox.y / 2.0f)
            {
                overlapsVert = true;
                uncollideMotion.y = motion.position.y + boundingBox.y / 2.0f - vertexPosition.y;
            }
        }
    }
    return uncollideMotion;
}

bool intersecting(vec2 p00, vec2 p01, vec2 p10, vec2 p11)
{
    // first line segment is between p00 and p01:
    // parametric equation: p00 + t(p01 - p00), 0 <= t <= 1
    
    // second line segment is between p10 and p11:
    // parametric equation: p10 + s(p11 - p10), 0 <= s <= 1
    
    bool t_intersect = false;
    bool s_intersect = false;
    
    // if intersection point falls within 0 <= t <= 1 AND 0 <= s <= 1, the line segments intersect
    // note: calculate numerators and denominators of t and s separately to avoid dividing by 0
    float t_numerator = (p00.x - p10.x) * (p10.y - p11.y) - (p00.y - p10.y) * (p10.x - p11.x);
    float t_denominator = (p00.x - p01.x) * (p10.y - p11.y) - (p00.y - p01.y) * (p10.x - p11.x);
    
    float s_numerator = (p01.x - p00.x) * (p00.y - p10.y) - (p01.y - p00.y) * (p00.x - p10.x);
    float s_denominator = t_denominator;
    
    // check 0 <= t <= 1
    if (t_numerator >= 0 && t_numerator <= t_denominator)
        t_intersect = true;
    
    // check 0 <= s <= 1
    if (s_numerator >= 0 && s_numerator <= s_denominator)
        s_intersect = true;
    
    return t_intersect && s_intersect;
}

void PhysicsSystem::step(float elapsed_ms, vec2 window_size_in_game_units, vec2 map_size_in_game_units)
{
	auto motionView = _ecs.view<Motion>();

	// Move entities based on how much time has passed, this is to (partially)
	// avoid having entities move at different speed based on the machine.
	for (auto& entity : motionView)
	{
		auto& motion = _ecs.get<Motion>(entity);
		float step_seconds = 1.0f * (elapsed_ms / 1000.f);
        
        // entities with physics-based animation
        if (_ecs.has<Physics>(entity))
        {
            auto& physics = _ecs.get<Physics>(entity);
            if (physics.enabled)
            {
                motion.velocity += physics.forces / physics.mass * step_seconds;    // update velocity using a = F/m
                
                // update squash if it is a ball
                if (_ecs.has<Ball>(entity))
                    physics.squash = 0.5f * sinf(normalize(motion.velocity).y) + 1.f;
            }
            else
            {
                physics.squash = 1.f;
            }
            
            motion.scale.y = physics.originalScale.y * physics.squash;
            
            // bouncing on ground: ball loses energy when it hits the ground and bounces back up
            if (motion.position.y + motion.scale.y / 2.f >= physics.groundLevel && physics.enabled)
            {
                motion.velocity = {physics.restitution.x * motion.velocity.x, physics.restitution.y * -motion.velocity.y};
                
                // once bounces get really small, stop
                if (abs(motion.velocity.x) < 50.f && abs(motion.velocity.y) < 50.f)
                {
                    motion.velocity = {0.f, 0.f};
                    physics.enabled = false;
                }
            }
        }
        motion.position += motion.velocity * step_seconds;
        
        if (motion.velocity.x > 0 && motion.scale.x > 0)
            motion.scale.x *= -1;
        if (motion.velocity.x < 0 && motion.scale.x < 0)
            motion.scale.x *= -1;

		vec2 boundingBox = get_bounding_box(motion);
		if (!_ecs.has<Background>(entity))
		{
			if (motion.position.x - boundingBox.x / 2 < 0.0f)
			{
				motion.position.x = boundingBox.x / 2;
			}
			if (motion.position.x + boundingBox.x / 2 >
				map_size_in_game_units.x)
			{
				motion.position.x =
					map_size_in_game_units.x - boundingBox.x / 2;
			}
			if (motion.position.y - boundingBox.y / 2 < 0.0f)
			{
				motion.position.y = boundingBox.y / 2;
			}
			if (motion.position.y + boundingBox.y / 2 >
				map_size_in_game_units.y)
			{
				motion.position.y =
					map_size_in_game_units.y - boundingBox.y / 2;
			}
        }
	}
    
	if (DebugSystem::in_debug_mode)
	{
		// Visualization for debugging the position and scale of objects
		for (auto& entity : motionView)
		{
			auto& motion = _ecs.get<Motion>(entity);
			DebugSystem::createBox(motion.position, motion.scale);
		}

		// Visualization of player collision meshes
		auto players = _ecs.view<Player, Motion>();
		for (auto [entity, player, motion] : players.each())
		{
			const auto& mesh = player.collisionMesh;
			const size_t triCount = mesh.vertex_indices.size() / 3;
			for (size_t i = 0; i < triCount; ++i)
			{
				mat3 S{
					{motion.scale.x, 0, 0}, {0, motion.scale.y, 0}, {0, 0, 1}};
				const auto& v0 =
					glm::vec2{
						mesh.vertices[mesh.vertex_indices[i * 3 + 0]].position *
						S} +
					motion.position;
				const auto& v1 =
					glm::vec2{
						mesh.vertices[mesh.vertex_indices[i * 3 + 1]].position *
						S} +
					motion.position;
				const auto& v2 =
					glm::vec2{
						mesh.vertices[mesh.vertex_indices[i * 3 + 2]].position *
						S} +
					motion.position;

				DebugSystem::createTriangle(v0, v1, v2, Colors::Blue);
			}
		}
	}

	// Check for collisions between all moving entities
	for (unsigned int i = 0; i < motionView.size(); i++)
	{
		entt::entity entity_i = motionView[i];
		Motion& motion_i = _ecs.get<Motion>(entity_i);
		if (_ecs.has<Cloud>(entity_i) || _ecs.has<Background>(entity_i))
			continue;
		for (unsigned int j = i + 1; j < motionView.size(); j++)
		{
			entt::entity entity_j = motionView[j];
            
            if (_ecs.has<Background>(entity_i) || _ecs.has<Background>(entity_j) || _ecs.has<Cloud>(entity_i) || _ecs.has<Cloud>(entity_j))
                continue;
			Motion& motion_j = _ecs.get<Motion>(entity_j);
			vec2 uncollideMotion = collides(motion_i, motion_j);
			if (uncollideMotion.x != 0 && uncollideMotion.y != 0)
			{
                bool vertexInsideBox = false;
                bool edgeIntersectsBoundingBox = false;
                
                if (_ecs.has<Player>(entity_i) || _ecs.has<Player>(entity_j))
                {
                    entt::entity player;
                    entt::entity otherEntity;
                    Motion otherMotion;
                    if (_ecs.has<Player>(entity_i))
                    {
                        player = entity_i;
                        otherMotion = motion_j;
                        otherEntity = entity_j;
                    }
                    else
                    {
                        player = entity_j;
                        otherMotion = motion_i;
                        otherEntity = entity_i;
                    }
                    
                    bool isTreeColliding = false;
                    if (_ecs.has<Building>(otherEntity) && _ecs.get<Building>(otherEntity).isTree)
                        isTreeColliding = true;
                    
                    if (!isTreeColliding)
                    {
                        auto& playerMesh = _ecs.get<Player>(player).collisionMesh;
                        auto& playerMotion = _ecs.get<Motion>(player);
                        Transform transform;
                        transform.translate(playerMotion.position);
                        transform.rotate(playerMotion.angle);
                        transform.scale(playerMotion.scale);
                
                        for (auto &vertex : playerMesh.vertices)
                        {
                            vec3 position = vec3(vertex.position.x, vertex.position.y, 1.0f);
                            position = transform.mat * position;
                            vec2 uncollideMotionVertex = vertex_collides(position, otherMotion);
                            if (uncollideMotionVertex.x != 0 && uncollideMotionVertex.y != 0)
                            {
                                uncollideMotion = uncollideMotionVertex;
                                vertexInsideBox = true;
                                break;
                            }
                        }
                        
                        // check if any edges intersect if no vertices are inside the other bounding box
                        if (!vertexInsideBox)
                        {
                            vec2 otherBoundingBox = get_bounding_box(otherMotion);
                            for (int k = 0; k < playerMesh.vertex_indices.size(); k++)
                            {
                                // vertex positions of the other's bounding box, starting from top left in CCW order
                                vec2 boundingBoxVertexPosition1 = otherMotion.position - vec2(otherBoundingBox.x / 2.f, otherBoundingBox.y / 2.f);
                                vec2 boundingBoxVertexPosition2 = otherMotion.position + vec2(-otherBoundingBox.x / 2.f, otherBoundingBox.y / 2.f);
                                vec2 boundingBoxVertexPosition3 = otherMotion.position + vec2(otherBoundingBox.x / 2.f, otherBoundingBox.y / 2.f);
                                vec2 boundingBoxVertexPosition4 = otherMotion.position + vec2(otherBoundingBox.x / 2.f, -otherBoundingBox.y / 2.f);
                                
                                std::vector<vec2> boundingBoxVertices = std::vector<vec2>();
                                boundingBoxVertices.push_back(boundingBoxVertexPosition1);
                                boundingBoxVertices.push_back(boundingBoxVertexPosition2);
                                boundingBoxVertices.push_back(boundingBoxVertexPosition3);
                                boundingBoxVertices.push_back(boundingBoxVertexPosition4);
                                
                                for (int m = 0; m < boundingBoxVertices.size(); m++)
                                {
                                    // check if the edge between vertex k and k+1 intersects with any of the 4 bounding box edges
                                    
                                    // get the endpoints of the edge in world coordinates
                                    vec3 v1 = vec3(playerMesh.vertices[playerMesh.vertex_indices[k]].position.x, playerMesh.vertices[playerMesh.vertex_indices[k]].position.y, 1.0f);
                                    v1 = transform.mat * v1;
                                    
                                    vec3 v2 = vec3(playerMesh.vertices[playerMesh.vertex_indices[(k+1) % playerMesh.vertex_indices.size()]].position.x, playerMesh.vertices[playerMesh.vertex_indices[(k+1) % playerMesh.vertex_indices.size()]].position.y, 1.0f);
                                    v2 = transform.mat * v2;
                                    
                                    vec2 p00 = vec2(v1.x, v1.y);
                                    vec2 p01 = vec2(v2.x, v2.y);
                                    vec2 p10 = boundingBoxVertices[m];
                                    vec2 p11 = boundingBoxVertices[(m+1) % boundingBoxVertices.size()];
                                    
                                    bool isIntersecting = intersecting(p00, p01, p10, p11);
                                    if (isIntersecting)
                                    {
                                        edgeIntersectsBoundingBox = true;
                                        vec2 initialDirection = {0.f, 0.f};
                                        if (playerMotion.velocity == vec2(0.f, 0.f))
                                        {
                                            if (m == 0)
                                                initialDirection = {1.f, 0.f};
                                            else if (m == 1)
                                                initialDirection = {0.f, -1.f};
                                            else if (m == 2)
                                                initialDirection = {-1.f, 0.f};
                                            else
                                                initialDirection = {0.f, 1.f};
                                        }
                                        else
                                            initialDirection = normalize(playerMotion.velocity);
                                        vec2 uncollideMotionEdge = -initialDirection;

                                        // calculate uncollide motion
                                        while (isIntersecting)
                                        {
                                            uncollideMotionEdge -= initialDirection;
                                            isIntersecting = intersecting(p00 + uncollideMotionEdge, p01 + uncollideMotionEdge, p10, p11);
                                        }

                                        uncollideMotion = uncollideMotionEdge;
                                        break;
                                    }
                                }
                                
                                // if any edge of the player mesh intersects the bounding box of the other entity, there is a collision
                                if (edgeIntersectsBoundingBox)
                                    break;
                            }
                        }
                        if (!edgeIntersectsBoundingBox && !vertexInsideBox)
                            continue;   // not a collision, so skip
                    }
                }
                
				// Create a collision event
				// Note, we are abusing the ECS system a bit in that we
				// potentially insert muliple collisions for the same entity,
				// hence, emplace_with_duplicates
				_ecs.emplace<Collision>(_ecs.create(), entity_i, entity_j,
										uncollideMotion);
				_ecs.emplace<Collision>(_ecs.create(), entity_j, entity_i,
										-uncollideMotion);

				// If collision is a bone, increase bone count
				if ((_ecs.has<Bone>(entity_j) || _ecs.has<Bone>(entity_i)) &&
					(_ecs.has<Player>(entity_i) || _ecs.has<Player>(entity_j)))
				{
                    Interaction playerInteraction;
                    Interaction boneInteraction;
                    if (_ecs.has<Player>(entity_i))
                    {
                        playerInteraction = _ecs.get<Interaction>(entity_i);
                        boneInteraction = _ecs.get<Interaction>(entity_j);
                    }
                    else
                    {
                        playerInteraction = _ecs.get<Interaction>(entity_j);
                        boneInteraction = _ecs.get<Interaction>(entity_i);
                    }
                    
                    // only increase the bone count if the player has pressed 'E' and it is not under a dirt mound
                    if (playerInteraction.canInteract && boneInteraction.canInteract && boneInteraction.timer >= boneInteraction.interactionDelay)
                        notifyObservers(BONE_COUNT_INCREASE);
				}
				// If enemy picks up ball, notify decisiontree
				if ((_ecs.has<Ball>(entity_j) || _ecs.has<Ball>(entity_i)) &&
					(_ecs.has<Enemy>(entity_i) || _ecs.has<Enemy>(entity_j)))
				{
                    Physics ballPhysics;
                    if (_ecs.has<Ball>(entity_i))
                        ballPhysics = _ecs.get<Physics>(entity_i);
                    else
                        ballPhysics = _ecs.get<Physics>(entity_j);
                    if (!ballPhysics.enabled)    // only pick up if at rest
                        notifyObservers(BALL_PICKED_UP);
				}
				// If player picks up ball, increase ball count
				if ((_ecs.has<Ball>(entity_j) || _ecs.has<Ball>(entity_i)) &&
					(_ecs.has<Player>(entity_i) || _ecs.has<Player>(entity_j)))
				{
                    Interaction playerInteraction;
                    if (_ecs.has<Player>(entity_i))
                        playerInteraction = _ecs.get<Interaction>(entity_i);
                    else
                        playerInteraction = _ecs.get<Interaction>(entity_j);
                    
                    // only increase the ball count if the player has pressed 'E'
                    if (playerInteraction.canInteract)
					notifyObservers(BALL_PICKED_UP_BY_PLAYER);
				}
				// If enemy collides with player, take away bone
				if ((_ecs.has<Enemy>(entity_j) || _ecs.has<Enemy>(entity_i)) &&
					(_ecs.has<Player>(entity_i) || _ecs.has<Player>(entity_j)))
				{
					if (_ecs.get<Enemy>(entity_i).takenBone == false)
					{
						notifyObservers(BONE_COUNT_DECREASE);
					}
				}
				// If player picks up socks, notify observers
				if ((_ecs.has<Socks>(entity_j) || _ecs.has<Socks>(entity_i)) &&
					(_ecs.has<Player>(entity_i) || _ecs.has<Player>(entity_j)))
				{
					Interaction playerInteraction;
					if (_ecs.has<Player>(entity_i))
						playerInteraction = _ecs.get<Interaction>(entity_i);
					else
						playerInteraction = _ecs.get<Interaction>(entity_j);

					// notify observers when the player has picked up socks
					// (player pressed 'E')
					if (playerInteraction.canInteract)
						notifyObservers(SOCKS_PICKED_UP);
				}
			}
		}
	}
}

void PhysicsSystem::notifyObservers(Event event)
{
	for (Observer* observer : observers)
	{
		observer->update(event);
	}
}

void PhysicsSystem::registerObserver(Observer* observer)
{
	observers.push_back(observer);
}

void PhysicsSystem::removeObserver(Observer* observer)
{
	auto iterator = std::find(observers.begin(), observers.end(), observer);

	if (iterator != observers.end())
	{
		observers.erase(iterator);
	}
}

void PhysicsSystem::observersSize() { std::cout << observers.size(); }
