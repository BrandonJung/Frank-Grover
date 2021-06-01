#pragma once

#include "ecs.hpp"
#include "common.hpp"
#include "debug.hpp"
#include "render_components.hpp"
#include "particle.hpp"

struct InstancedMesh;
struct ShadedMesh;

// OpenGL utilities
void gl_has_errors();

struct Framebuffer
{
    GLResource<FRAME_BUFFER> glFramebuffer;
    GLResource<RENDER_BUFFER> glRenderBuffer;
    GLResource<TEXTURE> glTexture;

    inline void bind() { glBindFramebuffer(GL_FRAMEBUFFER, glFramebuffer); }

    inline void unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
};

void generateFramebuffer(Framebuffer& framebuffer, int width, int height);

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem
{
public:
	// Initialize the window
	RenderSystem(GLFWwindow& window);

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw(float elapsed_ms, float total_ms,
			  vec2 window_size_in_game_units); // CHANGED (elapsed_ms)

	// Expose the creating of visual representations to other systems   //
	// CHANGED (animator)
	static void createSprite(ShadedMesh& mesh_container,
							 std::string texture_path, std::string shader_name,
							 Animator& animator);
	static void createSprite(ShadedMesh& sprite, std::string texture_path,
							 std::string shader_name);
	static void createColoredMesh(ShadedMesh& mesh_container,
								  std::string shader_name);
    void applyLightEffect(const GLResource<TEXTURE>& sourceTexture);

private:
	// Initialize the screeen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the
	// water shader
	void initScreenTexture();

	// Internal drawing functions for each entity type
	void drawLine(const DebugLine& line, const mat3& projection);
	void drawTexturedMesh(entt::entity entity, const mat3& projection);
	void drawTexturedMesh(vec3 position, float angle, vec2 scale,
						  const ShadedMesh& texmesh, const mat3& projection);
	void drawWaves(entt::entity entity, const mat3& projection,
				   const ShadedMesh& texmesh, float total_ms);
	void drawToScreen();

	// Window handle
	GLFWwindow& window;
    
    Framebuffer ping;
    Effect lightMaterial;
	DirtParticleSystem dirtParticleSystem;
	FoliageParticleSystem foliageParticleSystem;

	// Screen texture handles
	GLuint frame_buffer;
	ShadedMesh screen_sprite;
	GLResource<RENDER_BUFFER> depth_render_buffer_id;
	entt::entity screen_state_entity;
};
