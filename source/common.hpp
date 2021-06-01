#pragma once

// stlib
#include <string>
#include <tuple>
#include <vector>
#include <stdexcept>
#include <memory>

// glfw (OpenGL)
#define NOMINMAX
#include <gl3w/gl3w.h>
#include <GLFW/glfw3.h>

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>            // vec2
#include <glm/ext/vector_int2.hpp> // ivec2
#include <glm/vec3.hpp>            // vec3
#include <glm/mat3x3.hpp>          // mat3
using namespace glm;
static const float PI = 3.14159265359f;

#include "logger.hpp"

// Helper functions to convert between pixels and game units
vec2 gameToPixelUnits(const vec2& v);
vec2 pixelToGameUnits(const vec2& v);

// Simple utility functions to avoid mistyping directory name
inline std::string data_path() { return "data"; };
inline std::string shader_path(const std::string& name)
{
	return data_path() + "/shaders/" + name;
};
inline std::string textures_path(const std::string& name)
{
	return data_path() + "/textures/" + name;
};
inline std::string audio_path(const std::string& name)
{
	return data_path() + "/audio/" + name;
};
inline std::string levels_path(const std::string& name)
{
	return data_path() + "/levels/" + name;
};
inline std::string scripts_path(const std::string& name)
{
	return data_path() + "/scripts/" + name;
};
inline std::string meshes_path(const std::string& name)
{
	return data_path() + "/meshes/" + name;
};

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
struct Transform
{
	mat3 mat = {{1.f, 0.f, 0.f},
				{0.f, 1.f, 0.f},
				{0.f, 0.f, 1.f}}; // start with the identity
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
};

// All data relevant to the shape and motion of entities
struct Motion
{
	vec2 position = {0, 0};
	float depth = 0;
	float angle = 0;
	vec2 velocity = {0, 0};
	vec2 scale = {10, 10};
};

struct Interaction
{
    bool canInteract = false;
    float timer = 0.f;
    float interactionDelay = 1000.f;
};

namespace Colors
{
	constexpr const vec4 Red{1, 0, 0, 1};
	constexpr const vec4 Yellow{1, 1, 0, 1};
	constexpr const vec4 Green{0, 1, 0, 1};
	constexpr const vec4 Teal{0, 1, 1, 1};
	constexpr const vec4 Blue{0, 0, 1, 1};
	constexpr const vec4 Magenta{1, 0, 1, 1};
	constexpr const vec4 White{1, 1, 1, 1};
	constexpr const vec4 Black{0, 0, 0, 1};
}; // namespace Colors
