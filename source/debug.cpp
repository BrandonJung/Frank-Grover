// Header
#include "ecs.hpp"

#include "debug.hpp"
#include "render.hpp"
#include "ui.hpp"
#include "player.hpp"

#include <cmath>
#include <iostream>

#include "render_components.hpp"

namespace DebugSystem
{
	void createLine(vec2 start, vec2 end, vec3 color, float width,
					float duration, float depth)
	{
		_ecs.emplace<DebugLine>(_ecs.create(), start, end, width, color, depth,
								duration);
	}

	void createTriangle(vec2 v0, vec2 v1, vec2 v2, vec3 color, float width,
						float duration, float depth)
	{
		createLine(v0, v1, color, width, duration, depth);
		createLine(v1, v2, color, width, duration, depth);
		createLine(v2, v0, color, width, duration, depth);
	}

	void createBox(vec2 position, vec2 size)
	{
		// position is the center of the box
		// size is the width, height of the box

		float lineWidth = 5.0f;

		// Bottom Line
		DebugSystem::createLine(                                    //
			{position.x - size.x / 2.f, position.y + size.y / 2.f}, // start
			{position.x + size.x / 2.f, position.y + size.y / 2.f}, // end
			Colors::Red, lineWidth);

		// Top Line
		DebugSystem::createLine(                                    //
			{position.x - size.x / 2.f, position.y - size.y / 2.f}, // start
			{position.x + size.x / 2.f, position.y - size.y / 2.f}, // end
			Colors::Red, lineWidth);

		// Left Line
		DebugSystem::createLine(                                    //
			{position.x - size.x / 2.f, position.y - size.y / 2.f}, // start
			{position.x - size.x / 2.f, position.y + size.y / 2.f}, // end
			Colors::Red, lineWidth);

		// Right Line
		DebugSystem::createLine(                                    //
			{position.x + size.x / 2.f, position.y - size.y / 2.f}, // start
			{position.x + size.x / 2.f, position.y + size.y / 2.f}, // end
			Colors::Red, lineWidth);
	}

	// create an X at the position
	void createX(vec2 position) { 
		DebugSystem::createLine(position - vec2(50.0, 50.0),
								position + vec2(50.0, 50.0));
		DebugSystem::createLine(position + vec2(50.0, -50.0),
								position + vec2(-50.0, 50.0));
	}

	void clearDebugComponents(float elapsed_ms)
	{
		// Clear old debugging visualizations
		auto debugEntities = _ecs.view<DebugComponent>();
		_ecs.destroy(debugEntities.begin(), debugEntities.end());

		// Summon death and purge the debug world of lines that are too old
		auto debugLines = _ecs.view<DebugLine>();
		for (const auto& e : debugLines)
		{
			auto& l = _ecs.get<DebugLine>(e);
			l.timeToLive -= elapsed_ms / 1000.0f;
			if (l.timeToLive <= 0)
				_ecs.destroy(e);
		}
	}

	void DebugSystem::renderDebugEntityIDs(vec2 window_in_game_units)
	{
		auto view = _ecs.view<Motion>();
		for (auto [entity, motion] : view.each())
		{
			if (_ecs.has<DebugComponent>(entity) || _ecs.has<DebugLine>(entity))
				continue; // We don't print IDs for debug entities

			UI::drawText(
				std::to_string(static_cast<uint32_t>(entity)),
				UI::worldPosToUIPos(motion.position, window_in_game_units),
				{0, 0, 0, 1});
		}
	}

	bool in_debug_mode = false;
} // namespace DebugSystem
