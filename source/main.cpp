
#define GL3W_IMPLEMENTATION
#include <gl3w/gl3w.h>

// stlib
#include <chrono>
#include <iostream>

// internal
#include "ecs.hpp"
#include "common.hpp"
#include "world.hpp"
#include "render.hpp"
#include "physics.hpp"
#include "ai.hpp"
#include "debug.hpp"
#include "ui.hpp"
#include "pathfinder.hpp"
#include "decision_tree.hpp"

using Clock = std::chrono::high_resolution_clock;

const ivec2 window_size_in_px = {1200, 800};
const vec2 window_size_in_game_units = {3600, 2400};
const vec2 map_size_in_game_units = {10000, 6000};
// resolution should be coarse to ensure efficient pathfinding
const int gridResolution = 300;
int currentFrame = 0;
// Note, here the window will show a width x height part of the game world,
// measured in px. You could also define a window to show 1.5 x 1 part of your
// game world, where the aspect ratio depends on your window size.

vec2 gameToPixelUnits(const vec2& v)
{
	return {v.x * window_size_in_px.x / window_size_in_game_units.x,
			v.y * window_size_in_px.y / window_size_in_game_units.y};
}
vec2 pixelToGameUnits(const vec2& v)
{
	return {v.x * window_size_in_game_units.x /
				static_cast<float>(window_size_in_px.x),
			v.y * window_size_in_game_units.y /
				static_cast<float>(window_size_in_px.y)};
}

struct Description
{
	std::string name;
	Description(const char* str) : name(str){};
};

// EnTT implementation
entt::registry _ecs{};

// Entry point
int main()
{
	Logger::info("Rebuild UBC - Version ", 3);

	// Initialize the main systems
	WorldSystem world(window_size_in_px, window_size_in_game_units);
	RenderSystem renderer(*world.window);
	PhysicsSystem physics;
	AISystem ai;
	DecisionTree decisionTree;
	UI::initUI(window_size_in_px);
	world.loadUI(window_size_in_px);

	//// Set all states to default
	world.restart(false);
	// dimensions of the grid should be the total width/height of the game map
	ai.gameGrid.initGrid(map_size_in_game_units.x, map_size_in_game_units.y,
						 gridResolution);

	// debug runtime of pathfinder
	/*auto timeBeforePath = Clock::now();
	std::vector<vec2> path = Pathfinder::searchPath(ai.gameGrid, {500, 500},
	{700, 700}); std::cout << "time path took: "
			  << static_cast<float>(
					 (std::chrono::duration_cast<std::chrono::microseconds>(
						  Clock::now() - timeBeforePath))
						 .count()) /
					 1000.f
	<< std::endl;*/

	auto startTime = Clock::now();
	auto t = startTime;

	physics.registerObserver(&world);
	physics.registerObserver(&decisionTree);
	// Variable timestep loop
	while (!world.is_over())
	{
		// Processes system messages, if this wasn't present the window would
		// become unresponsive
		glfwPollEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			static_cast<float>(
				(std::chrono::duration_cast<std::chrono::microseconds>(now - t))
					.count()) /
			1000.f;
		float total_ms =
			static_cast<float>(
				(std::chrono::duration_cast<std::chrono::microseconds>(
					 now - startTime))
					.count()) /
			1000.f;
		t = now;

		DebugSystem::clearDebugComponents(elapsed_ms);
		ai.step(elapsed_ms, window_size_in_game_units, currentFrame);
		world.step(elapsed_ms, window_size_in_game_units);
		physics.step(elapsed_ms, window_size_in_game_units,
					 map_size_in_game_units);
		world.handle_collisions();
		decisionTree.step(elapsed_ms, window_size_in_game_units);
		renderer.draw(elapsed_ms, total_ms, window_size_in_game_units); // CHANGED
		currentFrame++;
	}

	return EXIT_SUCCESS;
}
