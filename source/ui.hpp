#pragma once

#include <glm/glm.hpp>

#include <freetype/config/ftheader.h>
#include FT_FREETYPE_H

#include <gl3w/gl3w.h>

#include <string>
#include <map>

#include "render_components.hpp"

namespace UI
{
	// Source: https://learnopengl.com/In-Practice/Text-Rendering
	struct Character
	{
		GLResource<TEXTURE> glTexture; // ID handle of the glyph texture
		glm::ivec2 size;               // Size of glyph
		glm::ivec2 bearing;   // Offset from baseline to left/top of glyph
		unsigned int advance; // Offset to advance to next glyph
	};

	using Font = uint32_t;

	void initUI(glm::ivec2 screenSize);
	void update(glm::ivec2 mousePos, bool isMouseClicking);
	void updateUISize(glm::ivec2 screenSize);
	glm::ivec3 worldPosToUIPos(glm::vec2 worldPos, glm::vec2 window_in_game_units);

	void clear();
	void drawComponents(glm::vec2 window_in_game_units);

	void drawPanel(glm::vec3 position, glm::vec2 size, glm::vec4 color,
				   float cornerSize = 0);

	Font loadFont(const std::string& fontPath);
	void setFont(Font font, float fontSize);

	void drawText(std::string text, glm::vec3 position, glm::vec4 color);
	void drawText(std::string text, glm::vec3 position, glm::vec2 containerSize,
				  glm::vec4 color);
	vec2 getMaxLineWidthAndNumLines(std::string text, glm::vec2 containerSize,
									Font font, float fontSize);

	void drawImage(const Texture& texture, glm::vec3 position, glm::vec2 size,
				   float angleInDegrees, glm::vec4 color);

	using ClickableID = uint32_t;
	bool isClickableClicked(ClickableID clickableID);
	void updateClickable(ClickableID clickableID, glm::vec2 position,
						 glm::vec2 size);
	struct Clickable
	{
		ClickableID id{0};

		Clickable() {}
		Clickable(ClickableID id) : id(id) {}

		inline bool isClicked() { return UI::isClickableClicked(id); }
		inline void update(glm::vec2 position, glm::vec2 size)
		{
			UI::updateClickable(id, position, size);
		}
	};
	Clickable createClickable(glm::vec2 position, glm::vec2 size);

	void drawOverTexture(const GLResource<TEXTURE>& sourceTexture);
	void debugFlushToScreen();
} // namespace UI

struct UILabel
{
	// The font for the label
	UI::Font font;
	// The font size for the label
	float fontSize;
	// The label text
	std::string text;
	// The position of the label in screen coordinates (pixels)
	// The depth component should be in the range -1000 to 1000
	glm::vec3 position;
	// The max size of the label in screen coordinates (pixels)
	glm::vec2 size;
	// The RGBA color of the label text
	glm::vec4 color;
};

struct UIPanel
{
	// The position of the panel in screen coordinates (pixels)
	// The depth component should be in the range -1000 to 1000
	glm::vec3 position;
	// The size of the panel in screen coordinates (pixels)
	glm::vec2 size;
	// The RGBA color of the panel
	glm::vec4 color;
	// The radius of the panel corner
	// Should be in the range of 0 (square) to (size / 2) fully rounded
	float cornerRadius{0};
};

struct UIImage
{
	// A reference to the image to draw
	Texture* texture;
	// The position of the image in screen coordinates (pixels)
	// The depth component should be in the range -1000 to 1000
	glm::vec3 position;
	// The size of the image in screen coordinates (pixels)
	glm::vec2 size;
	// The RGBA color to tint the image
	glm::vec4 color;
	// The angle to rotate the image (in radians)
	float angle;
};
