#pragma once

#include "common.hpp"

struct DebugLine
{
	vec2 start;
	vec2 end;
	float width{1};
	vec3 color{1, 1, 1};
	float depth{-0.1f};
	float timeToLive{0};

	DebugLine(vec2 start, vec2 end, float timeToLive = 0) :
		start(start), end(end), timeToLive(timeToLive)
	{
	}
	DebugLine(vec2 start, vec2 end, float width, vec3 color,
			  float depth = -0.1f, float timeToLive = 0) :
		start(start),
		end(end), width(width), color(color), depth(depth),
		timeToLive(timeToLive)
	{
	}
};

namespace DebugSystem
{
	extern bool in_debug_mode;

	// Draw a line between any two world positions
	void createLine(
		vec2 start, // The start position of the line (in world coordinates)
		vec2 end,   // The end position of the line (in world coordinates)
		vec3 color =
			Colors::Red, // RGB color of the line. (defaults to Colors::Red)
		float width = 3, // The width of the line. (defaults to 3)
		// How long should the line exist (in seconds)
		// The default is 0 seconds, which removes the line at the
		// end of the frame.
		float duration = 0,
		float depth =
			-0.1f // The z-depth to render the line at. (defaults to -0.1f)
	);

	// Draw a wireframe triangle given three world positions
	void createTriangle(
		vec2 v0, // One of the triangles vertices
		vec2 v1, // One of the triangles vertices
		vec2 v2, // One of the triangles vertices
		vec3 color =
			Colors::Red, // RGB color of the triangle. (defaults to Colors::Red)
		float width = 3, // The width of the line. (defaults to 3)
		// How long should the line exist (in seconds)
		// The default is 0 seconds, which removes the line at the
		// end of the frame.
		float duration = 0,
		float depth =
			-0.1f // The z-depth to render the line at. (defaults to -0.1f)
	);

	void createBox(vec2 position, vec2 size);

	// Removes all debugging graphics in ECS, called at every iteration of the
	// game loop
	void clearDebugComponents(float elapsed_ms);

	void renderDebugEntityIDs(vec2 window_in_game_units);
}; // namespace DebugSystem
