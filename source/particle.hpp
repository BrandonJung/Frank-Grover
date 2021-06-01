#pragma once

#include "ecs.hpp"
#include "common.hpp"
#include "render_components.hpp"

struct DirtParticle
{
	static entt::entity create(vec2 position, vec2 velocity, vec2 scale);
};

struct GPUParticleData
{
	float x;
	float y;
	float size;
	float depth;
};

struct DirtParticleSystem
{
	// Particles
	GLResource<BUFFER> buffer;
	uint32_t bufferCount;

	// Dirt Particle Mesh
	ShadedMesh dirtMesh;

	static DirtParticleSystem create();
	void updateBuffer();
	void draw(const mat3& projection);
};

void updateDirtParticles(float deltaTime);

struct FoliageParticle
{
	vec2 position;
	float depth;
	vec2 size;

	static entt::entity create(vec2 position, vec2 size, float depth);
};

struct GPUFoliageData
{
	float x;
	float y;
	float z; // depth
	float size;
};

struct FoliageParticleSystem
{
	// Particles
	GLResource<BUFFER> buffer;
	uint32_t count;

	// Dirt Particle Mesh
	ShadedMesh foliageMesh;

	static FoliageParticleSystem create();
	void updateBuffer();
	void draw(const mat3& projection, float totalTimeMS);
};
