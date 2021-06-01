#include "ui.hpp"
#include "render_components.hpp"
#include "render.hpp"
#include "debug.hpp"
#include "player.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

struct UIVertex
{
	float x;
	float y;
	float uvX;
	float uvY;
};

//struct Framebuffer
//{
//	GLResource<FRAME_BUFFER> glFramebuffer;
//	GLResource<RENDER_BUFFER> glRenderBuffer;
//	GLResource<TEXTURE> glTexture;
//
//	void bind() { glBindFramebuffer(GL_FRAMEBUFFER, glFramebuffer); }
//
//	void unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
//};

float remapDepth(float depth) { return depth / 1000.0f; }

//void generateFramebuffer(Framebuffer& framebuffer, int width, int height)
//{
//	// Generate framebuffer
//	glGenFramebuffers(1, framebuffer.glFramebuffer.data());
//	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.glFramebuffer);
//
//	// Generate screen texture
//	glGenTextures(1, framebuffer.glTexture.data());
//	glBindTexture(GL_TEXTURE_2D, framebuffer.glTexture);
//
//	// Generate the screen texture to back the frame buffer
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
//				 GL_UNSIGNED_BYTE, 0);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	// Link the screen texture as the framebuffer's colour attachement
//	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
//						 framebuffer.glTexture, 0);
//
//	// Generate a render buffer with a depth buffer for the framebuffer
//	glGenRenderbuffers(1, framebuffer.glRenderBuffer.data());
//	glBindRenderbuffer(GL_RENDERBUFFER, framebuffer.glRenderBuffer);
//	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
//	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
//							  GL_RENDERBUFFER, framebuffer.glRenderBuffer);
//
//	glBindFramebuffer(GL_FRAMEBUFFER,
//					  0); // Unbind the framebuffer to avoid bugs
//}

struct PanelData
{
	glm::vec2 offset;
	glm::vec2 size;
	glm::vec2 cornerSize;
};
// 9 sections, 2 triangles each, 3 indices per triangle
constexpr int NUM_NINE_SLICE_INDICES = 9 * 3 * 2;

struct FontData
{
	std::map<char, UI::Character> characters{};
	uint32_t lineHeight;
	uint32_t baseFontSize;
};

struct ClickableData
{
	glm::vec2 position;
	glm::vec2 size;
	bool isClicked{false};
};

class UIResources
{
public: // General
#ifndef NDEBUG
	bool debugMode{true};
#else
	bool debugMode{false};
#endif
	Framebuffer uiFramebuffer{};
	glm::mat4x4 uiMatrix;

public: // Quad Mesh
	GLResource<VERTEX_ARRAY> glVAO;
	GLResource<BUFFER> glVertexBuffer;

public: // Nine Slice Mesh
	GLResource<VERTEX_ARRAY> nineSliceGLVAO;
	GLResource<BUFFER> nineSliceGLVBO;
	GLResource<BUFFER> nineSliceGLIBO;

public: // Materials
	Effect blitMaterial{};

	Effect overdrawMaterial{};
	GLint overdrawMaterial_SourceTextureLoc;
	GLint overdrawMaterial_UITextureLoc;

	Effect textMaterial{};
	GLint textMaterial_MatrixUniform;
	GLint textMaterial_DepthUniform;
	GLint textMaterial_ColorUniform;

	Effect uiElementMaterial{};
	GLint uiElementMaterial_MatrixUniform;
	GLint uiElementMaterial_DepthUniform;
	GLint uiElementMaterial_ColorUniform;

public: // Panels
	Texture panelTexture;
	PanelData panelStyleData;

public: // Fonts
	std::map<UI::Font, FontData> fontData;
	UI::Font nextFontDataID{
		1}; // Start at 1 for easier debugging (0 is invalid)
	UI::Font currentFont;
	float currentFontSize;

public: // Clickable
	std::map<UI::ClickableID, ClickableData> clickables{};
	UI::ClickableID nextClickableID{
		1}; // Start at 1 for easier debugging (0 is invalid)
	glm::ivec2 lastMousePos{-1, -1};
	bool lastWasMouseClicking{false};
};
UIResources _resources;
glm::ivec2 _screenSize;

void generateOpenGLResources()
{
	// Blit Material
	_resources.blitMaterial.load_from_file("data/shaders/ui/blit.vs.glsl",
										   "data/shaders/ui/blit.fs.glsl");

	// Overdraw Material
	_resources.overdrawMaterial.load_from_file(
		"data/shaders/ui/blit.vs.glsl", "data/shaders/ui/uiOverdraw.fs.glsl");
	_resources.overdrawMaterial_SourceTextureLoc = glGetUniformLocation(
		_resources.overdrawMaterial.program, "u_sourceTexture");
	_resources.overdrawMaterial_UITextureLoc = glGetUniformLocation(
		_resources.overdrawMaterial.program, "u_uiTexture");

	// Text Material
	_resources.textMaterial.load_from_file("data/shaders/ui/uiElement.vs.glsl",
										   "data/shaders/ui/text.fs.glsl");
	_resources.textMaterial_MatrixUniform =
		glGetUniformLocation(_resources.textMaterial.program, "u_transform");
	_resources.textMaterial_DepthUniform =
		glGetUniformLocation(_resources.textMaterial.program, "u_depth");
	_resources.textMaterial_ColorUniform =
		glGetUniformLocation(_resources.textMaterial.program, "u_color");

	// UI Element Material
	_resources.uiElementMaterial.load_from_file(
		"data/shaders/ui/uiElement.vs.glsl",
		"data/shaders/ui/uiElement.fs.glsl");
	_resources.uiElementMaterial_MatrixUniform = glGetUniformLocation(
		_resources.uiElementMaterial.program, "u_transform");
	_resources.uiElementMaterial_DepthUniform =
		glGetUniformLocation(_resources.textMaterial.program, "u_depth");
	_resources.uiElementMaterial_ColorUniform =
		glGetUniformLocation(_resources.uiElementMaterial.program, "u_color");

	// Panel Textures
	_resources.panelTexture.load_from_file("data/textures/ui/panels.png");
	_resources.panelStyleData = {
		{0, 0},            // offset
		{100 / 225.0f, 1}, // size
		{0.25f, 0.25f},    // corner size
	};

	// Generate Quad Mesh
	glGenVertexArrays(1, _resources.glVAO.data());
	glGenBuffers(1, _resources.glVertexBuffer.data());
	glBindVertexArray(_resources.glVAO);
	glBindBuffer(GL_ARRAY_BUFFER, _resources.glVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(UIVertex) * 6, NULL,
				 // Dynamic draw because we will be abusing this poor buffer
				 GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0); // position & uv
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(UIVertex),
						  (GLvoid*)0);

	// Generate NineSlice Mesh
	glGenVertexArrays(1, _resources.nineSliceGLVAO.data());
	glGenBuffers(1, _resources.nineSliceGLVBO.data());
	glBindVertexArray(_resources.nineSliceGLVAO);
	glBindBuffer(GL_ARRAY_BUFFER, _resources.nineSliceGLVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(UIVertex) * 16, NULL,
				 // Dynamic draw because we will be abusing this poor buffer
				 GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0); // position & uv
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(UIVertex),
						  (GLvoid*)0);
	glGenBuffers(1, _resources.nineSliceGLIBO.data());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _resources.nineSliceGLIBO);
	// See: <repo>/docs/UI_9SliceMeshReference.png to understand vertex indices
	uint16_t nineSliceIndices[NUM_NINE_SLICE_INDICES] = {
		0,  1,  5,  5,  4,  0,  // A
		1,  2,  6,  6,  5,  1,  // B
		2,  3,  7,  7,  6,  2,  // C
		4,  5,  9,  9,  8,  4,  // D
		5,  6,  10, 10, 9,  5,  // E
		6,  7,  11, 11, 10, 6,  // F
		8,  9,  13, 13, 12, 8,  // G
		9,  10, 14, 14, 13, 9,  // H
		10, 11, 15, 15, 14, 10, // I
	};
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				 sizeof(uint16_t) * NUM_NINE_SLICE_INDICES, &nineSliceIndices,
				 GL_STATIC_DRAW);

	// Unbind to prevent bugs
	glBindVertexArray(0);
}

void activateTextMaterial(glm::vec4 color)
{
	// Setup Material
	_resources.uiFramebuffer.bind();
	glUseProgram(_resources.textMaterial.program); // Bind Program
	glActiveTexture(GL_TEXTURE0);                  // Bind texture
	gl_has_errors();

	// Enable blending
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Update the UI matrix
	glUniformMatrix4fv(_resources.textMaterial_MatrixUniform, 1, GL_FALSE,
					   &_resources.uiMatrix[0][0]);

	// Update the color uniform
	glUniform4f(_resources.textMaterial_ColorUniform, color.r, color.g, color.b,
				color.a);
}

void activateUIElementMaterial(glm::vec4 color)
{
	// Setup Material
	_resources.uiFramebuffer.bind();
	glUseProgram(_resources.uiElementMaterial.program); // Bind Program
	glActiveTexture(GL_TEXTURE0);                       // Bind texture
	gl_has_errors();

	// Enable blending
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Update the UI matrix
	glUniformMatrix4fv(_resources.uiElementMaterial_MatrixUniform, 1, GL_FALSE,
					   &_resources.uiMatrix[0][0]);

	// Update the color uniform
	glUniform4f(_resources.uiElementMaterial_ColorUniform, color.r, color.g,
				color.b, color.a);
}

void deactivateMaterial()
{
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind Texture
	glBindVertexArray(0);            // Unbind VAO
	glUseProgram(0);                 // Unbind Program
	_resources.uiFramebuffer.unbind();

	gl_has_errors();
}

void UI::initUI(glm::ivec2 screenSize)
{
	generateOpenGLResources();
	updateUISize(screenSize);
}

void UI::updateUISize(glm::ivec2 screenSize)
{
	_screenSize = screenSize;

	// Create UI Framebuffer
	generateFramebuffer(_resources.uiFramebuffer, _screenSize.x, _screenSize.y);

	// Create the UI Matrix
	_resources.uiMatrix =
		glm::ortho(0.0f, static_cast<float>(_screenSize.x), // left, right
				   0.0f, static_cast<float>(_screenSize.y)  // bottom, top
		);
}

glm::ivec3 UI::worldPosToUIPos(glm::vec2 worldPos, glm::vec2 window_in_game_units)
{
	auto player = _ecs.view<Player>()[0];
	auto& playerMotion = _ecs.get<Motion>(player);
	auto cameraPos = playerMotion.position - (window_in_game_units / 2.f);
	auto pixelPos = gameToPixelUnits(worldPos - cameraPos);
	return {pixelPos.x, _screenSize.y - pixelPos.y, 0};
}

void UI::clear()
{
	_resources.uiFramebuffer.bind();
	glViewport(0, 0, _screenSize.x, _screenSize.y);
	glClearColor(0, 0, 0, 0);
	glClearDepth(1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	_resources.uiFramebuffer.unbind();
}

void UI::drawComponents(glm::vec2 window_in_game_units)
{
	// Draw all panels
	{
		auto view = _ecs.view<UIPanel>();
		for (const auto& e : view)
		{
			const auto& panel = _ecs.get<UIPanel>(e);
			UI::drawPanel(panel.position, panel.size, panel.color,
						  panel.cornerRadius);

			if (_resources.debugMode && DebugSystem::in_debug_mode)
			{
			}
		}
	}
	// Draw all labels
	{
		auto view = _ecs.view<UILabel>();
		for (const auto& e : view)
		{
			const auto& label = _ecs.get<UILabel>(e);
			UI::setFont(label.font, label.fontSize);
			UI::drawText(label.text, label.position, label.size, label.color);

			if (_resources.debugMode && DebugSystem::in_debug_mode)
			{
			}
		}
	}
	// Draw all images
	{
		auto view = _ecs.view<UIImage>();
		for (const auto& e : view)
		{
			const auto& image = _ecs.get<UIImage>(e);
			UI::drawImage(*image.texture, image.position, image.size,
						  image.angle, image.color);

			if (_resources.debugMode && DebugSystem::in_debug_mode)
			{
				// Draw the pivot
				// Draw the bounding box
			}
		}
	}

	// Draw debug helpers
	if (DebugSystem::in_debug_mode)
	{
		DebugSystem::renderDebugEntityIDs(window_in_game_units);
	}
}

void UI::drawPanel(glm::vec3 position, glm::vec2 size, glm::vec4 color,
				   float cornerSize)
{
	activateUIElementMaterial(color);
	glBindVertexArray(_resources.nineSliceGLVAO); // Bind VAO

	const auto& panel = _resources.panelStyleData;

	// Position Variables
	float px = position.x;
	float py = position.y;
	float pw = size.x;
	float ph = size.y;
	float pcw = cornerSize;
	float pch = cornerSize * (static_cast<float>(_screenSize.y) / static_cast<float>(_screenSize.x));

	// Texture (UV) Variables
	float tx = panel.offset.x;
	float ty = panel.offset.y;
	float tw = panel.size.x;
	float th = panel.size.y;
	float tcw = panel.cornerSize.x;
	float tch = panel.cornerSize.y;

	// Update VertexBuffer
	// See: <repo>/docs/UI_9SliceMeshReference.png to understand vertex indices
	UIVertex vertices[16] = {
		{px, py, tx, ty},                                             // 0
		{px + pcw, py, tx + tcw, ty},                                 // 1
		{px + pw - pcw, py, tx + tw - tcw, ty},                       // 2
		{px + pw, py, tx + tw, ty},                                   // 3
		{px, py + pch, tx, ty + tch},                                 // 4
		{px + pcw, py + pch, tx + tcw, ty + tch},                     // 5
		{px + pw - pcw, py + pch, tx + tw - tcw, ty + tch},           // 6
		{px + pw, py + pch, tx + tw, ty + tch},                       // 7
		{px, py + ph - pch, tx, ty + th - tch},                       // 8
		{px + pcw, py + ph - pch, tx + tcw, ty + th - tch},           // 9
		{px + pw - pcw, py + ph - pch, tx + tw - tcw, ty + th - tch}, // 10
		{px + pw, py + ph - pch, tx + tw, ty + th - tch},             // 11
		{px, py + ph, tx, ty + th},                                   // 12
		{px + pcw, py + ph, tx + tcw, ty + th},                       // 13
		{px + pw - pcw, py + ph, tx + tw - tcw, ty + th},             // 14
		{px + pw, py + ph, tx + tw, ty + th}                          // 15
	};
	glBindBuffer(GL_ARRAY_BUFFER, _resources.nineSliceGLVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);

	// Draw
	glUniform1f(_resources.uiElementMaterial_DepthUniform,
				remapDepth(position.z));
	glBindTexture(GL_TEXTURE_2D, _resources.panelTexture.texture_id);
	glDrawElements(GL_TRIANGLES, NUM_NINE_SLICE_INDICES, GL_UNSIGNED_SHORT,
				   nullptr);

	deactivateMaterial();
}

UI::Font UI::loadFont(const std::string& fontPath)
{
	FontData font{};
	FT_Library freetypeLibrary{nullptr};
	FT_Face face{nullptr};
	try
	{
		if (FT_Init_FreeType(&freetypeLibrary))
		{
			throw std::runtime_error("Could not initialize FreeType Library!");
		}

		// Load font
		if (FT_New_Face(freetypeLibrary,
						fontPath.c_str(), // Font path
						0, // Font index if multiple fonts in the file
						&face))
		{
			std::string errMessage = "Could not load font: [" + fontPath + "]";
			std::cout << errMessage << std::endl;
			throw std::runtime_error(errMessage);
		}

		// Could use FT_Set_Char_Size for DPI aware scaling
		font.baseFontSize = 48;
		font.lineHeight = 64;
		FT_Set_Pixel_Sizes(face,
						   0,                // Font width, 0 = automatic
						   font.baseFontSize // Font size to extract
		);

		// Could use FT_Get_Char_Index to convert Unicode to Glyph indexes. For
		// now we'll just load the first 128 glyphs. This should cover basic
		// ASCII usage

		// Load font characters
		// Disable OpenGL byte-alignment restriction as some characters will
		// have odd size textures
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		for (unsigned char c = 0; c < 128; c++)
		{
			// Load character glyphs (and render them)
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				std::cout << "ERROR: Failed to load Glyph " << std::to_string(c)
						  << std::endl;
				continue;
			}

			UI::Character character{};

			// Generate an OpenGL texture to represent the character
			glGenTextures(1, character.glTexture.data());
			glBindTexture(GL_TEXTURE_2D, character.glTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width,
						 face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
						 face->glyph->bitmap.buffer);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// Store character for later use
			character.size =
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
			character.bearing =
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
			character.advance =
				(face->glyph->advance.x >>
				 6); // Advance is specified in 1/64th pixels. Bitshift by 6 to
					 // get value in pixels (2^6 = 64)
			font.characters.insert(
				std::pair<char, UI::Character>(c, std::move(character)));
		}

		if (face != nullptr)
			FT_Done_Face(face);
		if (freetypeLibrary != nullptr)
			FT_Done_FreeType(freetypeLibrary);
	}
	catch (std::exception ex)
	{
		if (face != nullptr)
			FT_Done_Face(face);
		if (freetypeLibrary != nullptr)
			FT_Done_FreeType(freetypeLibrary);
		throw;
	}

	// Add font to lookup and return the key
	_resources.fontData.insert({_resources.nextFontDataID, std::move(font)});
	return _resources.nextFontDataID++;
}

void UI::setFont(Font font, float fontSize)
{
	// Check that the font is valid
	if (_resources.fontData.find(font) == _resources.fontData.end())
	{
		throw std::runtime_error("Font " + std::to_string(font) +
								 " is not loaded!");
	}

	// Check font size
	assert(fontSize > 0);

	_resources.currentFont = font;
	_resources.currentFontSize = fontSize;
}

FontData& getFontData(UI::Font font)
{
	assert(font != 0);
	return _resources.fontData.at(_resources.currentFont);
}

float calculateNextWordSize(const std::string& text, const FontData& font,
							size_t index, size_t& endIndex, float scale,
							char& wrapChar)
{
	wrapChar = '\0';
	float size{0};
	size_t i{0};
	// Note: Subtract 1 from text size to ignore the null terminator character
	// '\0'
	for (i = index; i < text.size() - 1; ++i)
	{
		const char& c = text[i];

		// Break on whitespace characters
		if (isspace(c))
		{
			wrapChar = c;
			endIndex = i;
			return size;
		}

		const auto& charInfo = font.characters.at(c);

		size += charInfo.advance * scale;
	}

	endIndex = i + 1;
	return size;
}

void drawCharacter(char c, const FontData& font, glm::vec2& cursorPos,
				   float scale, float depth)
{
	const auto& charInfo = font.characters.at(c);
	// Only draw the character if it has a size
	if (charInfo.size.x > 0 && charInfo.size.y > 0)
	{
		// Update VertexBuffer
		float x = cursorPos.x + charInfo.bearing.x * scale;
		float y = cursorPos.y - (charInfo.size.y - charInfo.bearing.y) * scale;
		float w = charInfo.size.x * scale;
		float h = charInfo.size.y * scale;

		UIVertex vertices[6] = {
			{x, y, 0.0f, 0.0f},         {x + w, y, 1.0f, 0.0f},
			{x + w, y + h, 1.0f, 1.0f}, {x + w, y + h, 1.0f, 1.0f},
			{x, y + h, 0.0f, 1.0f},     {x, y, 0.0f, 0.0f}};
		glBindBuffer(GL_ARRAY_BUFFER, _resources.glVertexBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);

		// Draw
		glUniform1f(_resources.textMaterial_DepthUniform, remapDepth(depth));
		glBindTexture(GL_TEXTURE_2D, charInfo.glTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	// Advance character position
	cursorPos.x += charInfo.advance * scale;
}

void UI::drawText(std::string text, glm::vec3 position, glm::vec4 color)
{
	activateTextMaterial(color);
	glBindVertexArray(_resources.glVAO); // Bind VAO

	const auto& font = getFontData(_resources.currentFont);
	float scale = _resources.currentFontSize / font.baseFontSize;
	for (const char& c : text)
	{
		const auto& charInfo = font.characters.at(c);
		// Only draw the character if it has a size
		if (charInfo.size.x > 0 && charInfo.size.y > 0)
		{
			// Update VertexBuffer
			float x = position.x + charInfo.bearing.x * scale;
			float y =
				position.y - (charInfo.size.y - charInfo.bearing.y) * scale;
			float w = charInfo.size.x * scale;
			float h = charInfo.size.y * scale;

			UIVertex vertices[6] = {
				{x, y, 0.0f, 0.0f},         {x + w, y, 1.0f, 0.0f},
				{x + w, y + h, 1.0f, 1.0f}, {x + w, y + h, 1.0f, 1.0f},
				{x, y + h, 0.0f, 1.0f},     {x, y, 0.0f, 0.0f}};
			glBindBuffer(GL_ARRAY_BUFFER, _resources.glVertexBuffer);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);

			// Draw
			glUniform1f(_resources.textMaterial_DepthUniform,
						remapDepth(position.z));
			glBindTexture(GL_TEXTURE_2D, charInfo.glTexture);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		// Advance character position
		position.x += charInfo.advance * scale;
	}

	deactivateMaterial();
}

void UI::drawText(std::string text, glm::vec3 position, glm::vec2 containerSize,
				  glm::vec4 color)
{
	if (text.size() == 0)
		return;

	activateTextMaterial(color);
	glBindVertexArray(_resources.glVAO); // Bind VAO

	glm::vec2 cursorPos{position.x, position.y + containerSize.y -
										_resources.currentFontSize};
	const auto& font = getFontData(_resources.currentFont);
	float scale = _resources.currentFontSize / font.baseFontSize;
	for (size_t i = 0; i < text.size() - 1; ++i)
	{
		// Check for out of room
		if (cursorPos.y < position.y)
		{
			// Can't fit any more text, so we're done
			break;
		}

		// This next bit of code calculates the size of the next word
		// Also this: https://www.youtube.com/watch?v=Dpp3quce1Vo
		char wrapChar;
		size_t endIndex = 0;
		float nextWordSize =
			calculateNextWordSize(text, font, i, endIndex, scale, wrapChar);
		size_t wordCharCount = endIndex - i;

		float maxLineWidth = containerSize.x;
		float remainingSpace = (position.x + containerSize.x) - cursorPos.x;

		// Single character may be newline
		if (text[i] == '\n')
		{
			cursorPos.x = position.x;
			cursorPos.y -= font.lineHeight * scale;
			continue;
		}

		if (nextWordSize > maxLineWidth)
		{
			// No line could render this beast of a word. Just get after it,
			// until we can't. Then hyphenate the rest of this bad boy
			for (size_t j = i; j < endIndex; ++j)
			{
				char c = text[j];
				drawCharacter(c, font, cursorPos, scale, position.z);

				// Check for line wrap
				if (cursorPos.x > position.x + containerSize.x)
				{
					// Wrap line
					drawCharacter('-', font, cursorPos, scale, position.z);
					cursorPos.x = position.x;
					cursorPos.y -= font.lineHeight * scale;
				}

				// Check for out of room
				if (cursorPos.y < position.y)
				{
					// Can't fit any more text, so we're done
					break;
				}
			}
		}
		else if (nextWordSize > remainingSpace)
		{
			// The line be long, but not too long. Render it on the next
			// line

			// Wrap line
			cursorPos.x = position.x;
			cursorPos.y -= font.lineHeight * scale;

			// Check for not out of room
			if (cursorPos.y >= position.y)
			{
				// Render word
				for (size_t j = i; j < endIndex; ++j)
				{
					char c = text[j];
					drawCharacter(c, font, cursorPos, scale, position.z);
				}
				i = endIndex;
			}
		}
		else
		{
			// This word fits without issue
			// "I know words. I have the best words"
			//  - Oxford Dictionary, 1884
			for (size_t j = i; j < endIndex; ++j)
			{
				char c = text[j];
				drawCharacter(c, font, cursorPos, scale, position.z);
			}
		}

		// Render the wrap char (if needed)
		if (wrapChar == '\n')
		{
			// Wrap line
			cursorPos.x = position.x;
			cursorPos.y -= font.lineHeight * scale;
		}
		else if (wrapChar == '\0')
		{
			// Do nothing
		}
		else
			drawCharacter(wrapChar, font, cursorPos, scale, position.z);

		i = endIndex;
	}

	deactivateMaterial();
}

void advanceCursor(char c, const FontData& font, glm::vec2& cursorPos,
				   float scale)
{
	const auto& charInfo = font.characters.at(c);
	// Advance character position
	cursorPos.x += charInfo.advance * scale;
}

vec2 UI::getMaxLineWidthAndNumLines(std::string text,
									glm::vec2 containerSize, Font font, float fontSize)
{
	UI::setFont(font, fontSize);
	int maxLineWidthSoFar = 0;
	vec2 position = vec2(0, 0);

	glm::vec2 cursorPos{position.x, position.y};
	const auto& fontData = getFontData(_resources.currentFont);
	float lineHeight = _resources.currentFontSize;
	float scale = _resources.currentFontSize / fontData.baseFontSize;

	for (size_t i = 0; i < text.size() - 1; ++i)
	{
		// This next bit of code calculates the size of the next word
		// Also this: https://www.youtube.com/watch?v=Dpp3quce1Vo
		char wrapChar;
		size_t endIndex = 0;
		float nextWordSize =
			calculateNextWordSize(text, fontData, i, endIndex, scale, wrapChar);
		size_t wordCharCount = endIndex - i;


		float maxLineWidth = containerSize.x;
		float remainingSpace = (position.x + containerSize.x) - cursorPos.x;

		// Single character may be newline
		if (text[i] == '\n')
		{
			if (cursorPos.x > maxLineWidthSoFar)
			{
				maxLineWidthSoFar = cursorPos.x;
			}
			cursorPos.x = position.x;
			cursorPos.y -= lineHeight;
			continue;
		}

		if (nextWordSize > maxLineWidth)
		{
			// No line could render this beast of a word. Just get after it,
			// until we can't. Then hyphenate the rest of this bad boy
			maxLineWidthSoFar = maxLineWidth;
			for (size_t j = i; j < endIndex; ++j)
			{
				char c = text[j];
				advanceCursor(c, fontData, cursorPos, scale);
				// Check for line wrap
				if (cursorPos.x > position.x + containerSize.x)
				{
					// Wrap line
					advanceCursor('-', fontData, cursorPos, scale);
					if (cursorPos.x > maxLineWidthSoFar)
					{
						maxLineWidthSoFar = cursorPos.x;
					}
					cursorPos.x = position.x;
					cursorPos.y -= lineHeight;
				}
			}
		}
		else if (nextWordSize > remainingSpace)
		{
			// The line be long, but not too long. Render it on the next
			// line

			// Wrap line
			if (cursorPos.x > maxLineWidthSoFar)
			{
				maxLineWidthSoFar = cursorPos.x;
			}
			cursorPos.x = position.x;
			cursorPos.y -= lineHeight;


		// Render word
		for (size_t j = i; j < endIndex; ++j)
		{
			char c = text[j];
			advanceCursor(c, fontData, cursorPos, scale);
		}
		i = endIndex;
		}
		else
		{
			// This word fits without issue
			// "I know words. I have the best words"
			//  - Oxford Dictionary, 1884
			for (size_t j = i; j < endIndex; ++j)
			{
				char c = text[j];
				advanceCursor(c, fontData, cursorPos, scale);
			}
		}

		// Render the wrap char (if needed)
		if (wrapChar == '\n')
		{
			// Wrap line
			if (cursorPos.x > maxLineWidthSoFar)
			{
				maxLineWidthSoFar = cursorPos.x;
			}
			cursorPos.x = position.x;
			cursorPos.y -= lineHeight;
		}
		else if (wrapChar == '\0')
		{
			// Do nothing
		}
		else
			advanceCursor(wrapChar, fontData, cursorPos, scale);

		i = endIndex;
	}
	if (cursorPos.x > maxLineWidthSoFar)
	{
		maxLineWidthSoFar = cursorPos.x;
	}
	return vec2(maxLineWidthSoFar, -cursorPos.y + lineHeight);
}

void UI::drawImage(const Texture& texture, glm::vec3 position, glm::vec2 size,
				   float angleInDegrees, glm::vec4 color)
{
	activateUIElementMaterial(color);
	glBindVertexArray(_resources.glVAO); // Bind VAO

	// Update matrix to include rotation
	float c = std::cos(glm::radians(angleInDegrees));
	float s = std::sin(glm::radians(angleInDegrees));
	mat4x4 T = {{1, 0, 0, 0},
				{0, 1, 0, 0},
				{0, 0, 1, 0},
				{position.x, position.y, 0, 1}};
	mat4x4 R = {{c, s, 0, 0}, {-s, c, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
	auto matrix = _resources.uiMatrix * T * R;
	glUniformMatrix4fv(_resources.uiElementMaterial_MatrixUniform, 1, GL_FALSE,
					   &matrix[0][0]);

	// Update VertexBuffer
	float x = 0;
	float y = 0;
	float w = size.x;
	float h = size.y;

	UIVertex vertices[6] = {
		{x, y, 0.0f, 0.0f},         {x + w, y, 1.0f, 0.0f},
		{x + w, y + h, 1.0f, 1.0f}, {x + w, y + h, 1.0f, 1.0f},
		{x, y + h, 0.0f, 1.0f},     {x, y, 0.0f, 0.0f}};
	glBindBuffer(GL_ARRAY_BUFFER, _resources.glVertexBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);

	// Draw
	glUniform1f(_resources.uiElementMaterial_DepthUniform,
				remapDepth(position.z));
	glBindTexture(GL_TEXTURE_2D, texture.texture_id);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	deactivateMaterial();
}

UI::Clickable UI::createClickable(glm::vec2 position, glm::vec2 size)
{
	ClickableData clickableData{position, size};

	// Add clickable to lookup and return the key
	_resources.clickables.insert(
		{_resources.nextClickableID, std::move(clickableData)});
	return _resources.nextClickableID++;
}
void UI::updateClickable(UI::ClickableID clickableID, glm::vec2 position,
						 glm::vec2 size)
{
#ifndef NDEBUG
	if (_resources.clickables.find(clickableID) == _resources.clickables.end())
		throw std::runtime_error("Clickable with ID [" +
								 std::to_string(clickableID) +
								 "] does not exist!");
#endif
	auto& clickable = _resources.clickables.at(clickableID);
	clickable.position = position;
	clickable.size = size;
}
bool UI::isClickableClicked(UI::ClickableID clickableID)
{
#ifndef NDEBUG
	if (_resources.clickables.find(clickableID) == _resources.clickables.end())
		throw std::runtime_error("Clickable with ID [" +
								 std::to_string(clickableID) +
								 "] does not exist!");
#endif
	auto& clickable = _resources.clickables.at(clickableID);
	return clickable.isClicked;
}

void UI::update(glm::ivec2 mousePos, bool isMouseClicking)
{
	// We need to flip the mouse y position, because the original idiot who
	// wrote the code decided the UI starts at the bottom left unlike OpenGL
	// which uses coordinates starting at the top left. P.S. I'm the
	// original idiot, I'll fix it one day... or not... maybe UI should use
	// the bottom left as the origin?
	mousePos = {mousePos.x, _screenSize.y - mousePos.y};

	// Loop through clickables and reset them
	for (auto& clickablePair : _resources.clickables)
	{
		auto& clickable = clickablePair.second;
		clickable.isClicked = false;
	}

	// Check if this is the first frame the mouse is clicking
	if (!_resources.lastWasMouseClicking && isMouseClicking)
	{
		// std::cout << "Player clicked at (" << mousePos.x << "," <<
		// mousePos.y
		//   << ")" << std::endl;

		// Loop through clickables and check if clicked
		for (auto& clickablePair : _resources.clickables)
		{
			auto& clickable = clickablePair.second;
			// Check if mouse is inside the clickable area
			if (mousePos.x >= clickable.position.x &&
				mousePos.x <= clickable.position.x + clickable.size.x &&
				mousePos.y >= clickable.position.y &&
				mousePos.y <= clickable.position.y + clickable.size.y)
			{
				clickable.isClicked = true;
			}
		}
	}

	_resources.lastMousePos = mousePos;
	_resources.lastWasMouseClicking = isMouseClicking;
}

void UI::drawOverTexture(const GLResource<TEXTURE>& sourceTexture)
{
	glUseProgram(_resources.overdrawMaterial.program); // Bind Program
	glBindVertexArray(_resources.glVAO);               // Bind VAO

	UIVertex vertices[6] = {{-1, -1, 0, 0}, {1, -1, 1, 0}, {1, 1, 1, 1},
							{1, 1, 1, 1},   {-1, 1, 0, 1}, {-1, -1, 0, 0}};

	glUniform1i(_resources.overdrawMaterial_SourceTextureLoc, 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, sourceTexture);

	glUniform1i(_resources.overdrawMaterial_UITextureLoc, 1);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, _resources.uiFramebuffer.glTexture);

	// Draw
	glDisable(GL_DEPTH_TEST);
	glBindBuffer(GL_ARRAY_BUFFER, _resources.glVertexBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, 0); // Unbind Texture
	glBindVertexArray(0);            // Unbind VAO
	glUseProgram(0);                 // Unbind Program

	gl_has_errors();
}

void UI::debugFlushToScreen()
{
	glBindFramebuffer(GL_FRAMEBUFFER,
					  0); // Ensure we are rendering to the back buffer
	glUseProgram(_resources.blitMaterial.program); // Bind Program
	glBindVertexArray(_resources.glVAO);           // Bind VAO
	glActiveTexture(GL_TEXTURE0);                  // Bind texture

	UIVertex vertices[6] = {{-1, -1, 0, 0}, {1, -1, 1, 0}, {1, 1, 1, 1},
							{1, 1, 1, 1},   {-1, 1, 0, 1}, {-1, -1, 0, 0}};

	// Draw
	glBindTexture(GL_TEXTURE_2D, _resources.uiFramebuffer.glTexture);
	glBindBuffer(GL_ARRAY_BUFFER, _resources.glVertexBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, 0); // Unbind Texture
	glBindVertexArray(0);            // Unbind VAO
	glUseProgram(0);                 // Unbind Program
}
