// internal
#include "render.hpp"
#include "render_components.hpp"
#include "player.hpp"
#include "cloud.hpp"
#include "debug.hpp"
#include "waves.hpp"
#include "particle.hpp"

#include "ui.hpp"

#include <iostream>

void RenderSystem::applyLightEffect(const GLResource<TEXTURE>& sourceTexture)
{
	glUseProgram(lightMaterial.program);       // Bind Program
	glBindVertexArray(screen_sprite.mesh.vao); // Bind VAO

	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(&window, &w, &h);
	ping.bind();
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();

	// Disable alpha channel for mapping the screen texture onto the real screen
	glDisable(
		GL_BLEND); // we have a single texture without transparency. Areas with
				   // alpha <1 cab arise around the texture transparency
				   // boundary, enabling blending would make them visible.
	glDisable(GL_DEPTH_TEST);

	glBindBuffer(GL_ARRAY_BUFFER, screen_sprite.mesh.vbo);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		screen_sprite.mesh.ibo); // Note, GL_ELEMENT_ARRAY_BUFFER associates
								 // indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();

	// Set the vertex position and vertex texture coordinates (both stored in
	// the same VBO)
	GLint in_position_loc =
		glGetAttribLocation(lightMaterial.program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
						  sizeof(TexturedVertex), (void*)0);
	GLint in_texcoord_loc =
		glGetAttribLocation(lightMaterial.program, "in_texcoord");
	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(
		in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
		(void*)sizeof(
			vec3)); // note the stride to skip the preceeding vertex position
	gl_has_errors();

	// Set clock
	GLuint time_uloc = glGetUniformLocation(lightMaterial.program, "time");
	glUniform1f(time_uloc, static_cast<float>(glfwGetTime() * 10.0f));
	gl_has_errors();

	// Flashlight/night effects
	auto player = _ecs.view<Player>()[0];
	auto playerNight = _ecs.get<Night>(player);

	GLuint night_timer_uloc =
		glGetUniformLocation(lightMaterial.program, "night_timer");
	glUniform1f(night_timer_uloc, playerNight.night_timer);

	GLuint flashlight_on_uloc =
		glGetUniformLocation(lightMaterial.program, "flashlight_on");
	glUniform1i(flashlight_on_uloc, playerNight.flashlight_on);

	GLuint is_night_uloc =
		glGetUniformLocation(lightMaterial.program, "is_night");
	glUniform1i(is_night_uloc, playerNight.is_night);

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sourceTexture);

	// Draw
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

	glBindTexture(GL_TEXTURE_2D, 0); // Unbind Texture
	glBindVertexArray(0);            // Unbind VAO
	glUseProgram(0);                 // Unbind Program

	gl_has_errors();
}

void generateFramebuffer(Framebuffer& framebuffer, int width, int height)
{
	// Generate framebuffer
	glGenFramebuffers(1, framebuffer.glFramebuffer.data());
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.glFramebuffer);

	// Generate screen texture
	glGenTextures(1, framebuffer.glTexture.data());
	glBindTexture(GL_TEXTURE_2D, framebuffer.glTexture);

	// Generate the screen texture to back the frame buffer
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
				 GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// Link the screen texture as the framebuffer's colour attachement
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
						 framebuffer.glTexture, 0);

	// Generate a render buffer with a depth buffer for the framebuffer
	glGenRenderbuffers(1, framebuffer.glRenderBuffer.data());
	glBindRenderbuffer(GL_RENDERBUFFER, framebuffer.glRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
							  GL_RENDERBUFFER, framebuffer.glRenderBuffer);

	glBindFramebuffer(GL_FRAMEBUFFER,
					  0); // Unbind the framebuffer to avoid bugs
}

void getLinePoints(vec2 start, vec2 end, float width, vec2& p1, vec2& p2,
				   vec2& p3, vec2& p4)
{
	float x = end.x - start.x, y = end.y - start.y;
	vec2 normal = glm::normalize(vec2{y, -x}) * width;
	p1 = start + normal;
	p2 = end + normal;
	p3 = end - normal;
	p4 = start - normal;
}

void RenderSystem::drawLine(const DebugLine& line, const mat3& projection)
{
	vec2 p1, p2, p3, p4;
	getLinePoints(line.start, line.end, line.width, p1, p2, p3, p4);

	std::string key = "line";
	ShadedMesh& texmesh = cache_resource(key);
	if (texmesh.effect.program.resource == 0)
	{
		// First time rendering a line, so generate the mesh
		texmesh.mesh.vertices.emplace_back<ColoredVertex>({});
		texmesh.mesh.vertices.emplace_back<ColoredVertex>({});
		texmesh.mesh.vertices.emplace_back<ColoredVertex>({});
		texmesh.mesh.vertices.emplace_back<ColoredVertex>({});

		texmesh.mesh.vertex_indices.push_back(0);
		texmesh.mesh.vertex_indices.push_back(1);
		texmesh.mesh.vertex_indices.push_back(2);
		texmesh.mesh.vertex_indices.push_back(0);
		texmesh.mesh.vertex_indices.push_back(2);
		texmesh.mesh.vertex_indices.push_back(3);

		RenderSystem::createColoredMesh(texmesh, "colored_mesh");
	}

	texmesh.mesh.vertices[0] = {{p1.x, p1.y, line.depth}, line.color};
	texmesh.mesh.vertices[1] = {{p2.x, p2.y, line.depth}, line.color};
	texmesh.mesh.vertices[2] = {{p3.x, p3.y, line.depth}, line.color};
	texmesh.mesh.vertices[3] = {{p4.x, p4.y, line.depth}, line.color};

	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform{};

	// Setting shaders
	glUseProgram(texmesh.effect.program);
	glBindVertexArray(texmesh.mesh.vao);
	gl_has_errors();

	// Enabling alpha channel for textures
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	gl_has_errors();

	GLint transform_uloc =
		glGetUniformLocation(texmesh.effect.program, "transform");
	GLint projection_uloc =
		glGetUniformLocation(texmesh.effect.program, "projection");
	gl_has_errors();

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, texmesh.mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER,
				 sizeof(ColoredVertex) * texmesh.mesh.vertices.size(),
				 texmesh.mesh.vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texmesh.mesh.ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	GLint in_position_loc =
		glGetAttribLocation(texmesh.effect.program, "in_position");
	GLint in_color_loc =
		glGetAttribLocation(texmesh.effect.program, "in_color");
	if (in_color_loc >= 0)
	{
		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex),
							  reinterpret_cast<void*>(0));
		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex),
							  reinterpret_cast<void*>(sizeof(vec3)));
	}
	else
	{
		throw std::runtime_error("This type of entity is not yet supported");
	}
	gl_has_errors();

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(texmesh.effect.program, "fcolor");
	glUniform3fv(color_uloc, 1, (float*)&texmesh.texture.color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();
	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform.mat);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	glBindVertexArray(0);
}

void RenderSystem::drawTexturedMesh(entt::entity entity, const mat3& projection)
{
	auto& motion = _ecs.get<Motion>(entity);
	auto& texmesh = *_ecs.get<ShadedMeshRef>(entity).reference_to_cache;
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(motion.position);
	transform.rotate(motion.angle);
	if (_ecs.has<Animator>(entity))
	{
		auto& animator = _ecs.get<Animator>(entity);
		transform.scale(
			vec2(motion.scale.x * animator.max_num_frames,
				 motion.scale.y * (float)animator.animations.size()));
	}
	else
		transform.scale(motion.scale);

	// Setting shaders
	glUseProgram(texmesh.effect.program);
	glBindVertexArray(texmesh.mesh.vao);
	gl_has_errors();

	// Enabling alpha channel for textures
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	gl_has_errors();

	GLint transform_uloc =
		glGetUniformLocation(texmesh.effect.program, "transform");
	GLint projection_uloc =
		glGetUniformLocation(texmesh.effect.program, "projection");
	GLint depth_uloc = glGetUniformLocation(texmesh.effect.program, "u_depth");

	// Animation info
	GLint frame_uloc =
		glGetUniformLocation(texmesh.effect.program, "u_frame_num"); // CHANGED
	GLint frame_width_uloc =
		glGetUniformLocation(texmesh.effect.program, "u_frame_width");
	GLint frame_height_uloc =
		glGetUniformLocation(texmesh.effect.program, "u_frame_height");
	GLint curr_anim_uloc =
		glGetUniformLocation(texmesh.effect.program, "u_curr_anim");
	gl_has_errors();

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, texmesh.mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texmesh.mesh.ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	GLint in_position_loc =
		glGetAttribLocation(texmesh.effect.program, "in_position");
	GLint in_texcoord_loc =
		glGetAttribLocation(texmesh.effect.program, "in_texcoord");
	GLint in_color_loc =
		glGetAttribLocation(texmesh.effect.program, "in_color");
	if (in_texcoord_loc >= 0)
	{
		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex),
							  reinterpret_cast<void*>(0));
		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex),
							  reinterpret_cast<void*>(
								  sizeof(vec3))); // note the stride to skip the
												  // preceeding vertex position
		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texmesh.texture.texture_id);
	}
	else if (in_color_loc >= 0)
	{
		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex),
							  reinterpret_cast<void*>(0));
		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex),
							  reinterpret_cast<void*>(sizeof(vec3)));
	}
	else
	{
		throw std::runtime_error("This type of entity is not yet supported");
	}
	gl_has_errors();

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(texmesh.effect.program, "fcolor");
	glUniform3fv(color_uloc, 1, (float*)&texmesh.texture.color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();
	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	// Setting uniform values to the currently bound program
	glUniform1f(depth_uloc, motion.depth / 1000.0f);
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform.mat);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);
	if (_ecs.has<Animator>(entity))
	{
		auto& animator = _ecs.get<Animator>(entity);
		glUniform1i(frame_uloc, animator.frame);
		glUniform1f(frame_width_uloc, 1 / animator.max_num_frames);
		glUniform1f(frame_height_uloc, 1 / (float)animator.animations.size());
		glUniform1f(curr_anim_uloc, (float)animator.curr_animation);
	}
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);

	glBindVertexArray(0);
	glDisable(GL_DEPTH_TEST);
}

void RenderSystem::drawTexturedMesh(vec3 position, float angle, vec2 scale,
									const ShadedMesh& texmesh,
									const mat3& projection)
{
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(vec2{position.x, position.y});
	transform.rotate(angle);
	transform.scale(scale);

	// Setting shaders
	glUseProgram(texmesh.effect.program);
	glBindVertexArray(texmesh.mesh.vao);
	gl_has_errors();

	// Enabling alpha channel for textures
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	gl_has_errors();

	GLint transform_uloc =
		glGetUniformLocation(texmesh.effect.program, "transform");
	GLint projection_uloc =
		glGetUniformLocation(texmesh.effect.program, "projection");
	GLint depth_uloc = glGetUniformLocation(texmesh.effect.program, "u_depth");

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, texmesh.mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texmesh.mesh.ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	GLint in_position_loc =
		glGetAttribLocation(texmesh.effect.program, "in_position");
	GLint in_texcoord_loc =
		glGetAttribLocation(texmesh.effect.program, "in_texcoord");
	GLint in_color_loc =
		glGetAttribLocation(texmesh.effect.program, "in_color");
	if (in_texcoord_loc >= 0)
	{
		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex),
							  reinterpret_cast<void*>(0));
		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex),
							  reinterpret_cast<void*>(
								  sizeof(vec3))); // note the stride to skip the
												  // preceeding vertex position
		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texmesh.texture.texture_id);
	}
	gl_has_errors();

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(texmesh.effect.program, "fcolor");
	glUniform3fv(color_uloc, 1, (float*)&texmesh.texture.color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();
	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	// Setting uniform values to the currently bound program
	glUniform1f(depth_uloc, position.z / 1000.0f);
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform.mat);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);

	glBindVertexArray(0);
	glDisable(GL_DEPTH_TEST);
}

void RenderSystem::drawWaves(entt::entity entity, const mat3& projection,
							 const ShadedMesh& texmesh, float total_ms)
{
	auto& motion = _ecs.get<Motion>(entity);
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(motion.position);
	transform.rotate(motion.angle);
	transform.scale(motion.scale);

	// Setting shaders
	glUseProgram(texmesh.effect.program);
	glBindVertexArray(texmesh.mesh.vao);
	gl_has_errors();

	// Enabling alpha channel for textures
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	gl_has_errors();

	GLint transform_uloc =
		glGetUniformLocation(texmesh.effect.program, "transform");
	GLint projection_uloc =
		glGetUniformLocation(texmesh.effect.program, "projection");
	GLint depth_uloc = glGetUniformLocation(texmesh.effect.program, "u_depth");

	GLint itime_uloc =
		glGetUniformLocation(texmesh.effect.program, "iTime");
	GLint iresolution_uloc = glGetUniformLocation(texmesh.effect.program, "iResolution");
	gl_has_errors();

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, texmesh.mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texmesh.mesh.ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	GLint in_position_loc =
		glGetAttribLocation(texmesh.effect.program, "in_position");
	GLint in_texcoord_loc =
		glGetAttribLocation(texmesh.effect.program, "in_texcoord");
	GLint in_color_loc =
		glGetAttribLocation(texmesh.effect.program, "in_color");
	if (in_texcoord_loc >= 0)
	{
		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex),
							  reinterpret_cast<void*>(0));
		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex),
							  reinterpret_cast<void*>(
								  sizeof(vec3))); // note the stride to skip the
												  // preceeding vertex position
		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texmesh.texture.texture_id);
	}
	else if (in_color_loc >= 0)
	{
		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex),
							  reinterpret_cast<void*>(0));
		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex),
							  reinterpret_cast<void*>(sizeof(vec3)));
	}
	else
	{
		throw std::runtime_error("This type of entity is not yet supported");
	}
	gl_has_errors();

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(texmesh.effect.program, "fcolor");
	glUniform3fv(color_uloc, 1, (float*)&texmesh.texture.color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();
	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	// Setting uniform values to the currently bound program
	int w, h;
	glfwGetFramebufferSize(&window, &w, &h);
	const vec3 iResolution = vec3(w, h, 1.0);
	glUniform1f(itime_uloc, total_ms / 1000.0); // time in seconds
	glUniform3fv(iresolution_uloc, 1,
				 (float*)&iResolution); // the third value is 1.0 for a square pixel aspect ratio, according to the co-founder of Shadertoy on Twitter:
	// https://twitter.com/iquilezles/status/633113177568313344

	glUniform1f(depth_uloc, motion.depth / 1000.0f);
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform.mat);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);

	glBindVertexArray(0);
	glDisable(GL_DEPTH_TEST);
}

// Draw the intermediate texture to the screen, with some distortion to simulate
// water
void RenderSystem::drawToScreen()
{
	// Setting shaders
	glUseProgram(screen_sprite.effect.program);
	glBindVertexArray(screen_sprite.mesh.vao);
	gl_has_errors();

	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(&window, &w, &h);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();

	// Disable alpha channel for mapping the screen texture onto the real screen
	glDisable(
		GL_BLEND); // we have a single texture without transparency. Areas with
				   // alpha <1 can arise around the texture transparency
				   // boundary, enabling blending would make them visible.
	glDisable(GL_DEPTH_TEST);

	glBindBuffer(GL_ARRAY_BUFFER, screen_sprite.mesh.vbo);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		screen_sprite.mesh.ibo); // Note, GL_ELEMENT_ARRAY_BUFFER associates
								 // indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();

	// Draw the screen texture on the quad geometry
	gl_has_errors();

	// Set the vertex position and vertex texture coordinates (both stored in
	// the same VBO)
	GLint in_position_loc =
		glGetAttribLocation(screen_sprite.effect.program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
						  sizeof(TexturedVertex), (void*)0);
	GLint in_texcoord_loc =
		glGetAttribLocation(screen_sprite.effect.program, "in_texcoord");
	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(
		in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
		(void*)sizeof(
			vec3)); // note the stride to skip the preceeding vertex position
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ping.glTexture);

	// Draw
	glDrawElements(
		GL_TRIANGLES, 6, GL_UNSIGNED_SHORT,
		nullptr); // two triangles = 6 vertices; nullptr indicates that there is
				  // no offset from the bound index buffer
	glBindVertexArray(0);
	gl_has_errors();
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw(float elapsed_ms, float total_ms, vec2 window_size_in_game_units)
{
	// Getting size of window
	ivec2 frame_buffer_size; // in pixels
	glfwGetFramebufferSize(&window, &frame_buffer_size.x, &frame_buffer_size.y);

	// Draw the UI
	UI::clear(); // Clear the UI to be redrawn each frame
	UI::drawComponents(
		window_size_in_game_units); // Draw all images, labels, panels

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();

	// Clearing backbuffer
	glViewport(0, 0, frame_buffer_size.x, frame_buffer_size.y);
	glDepthRange(0.00001, 10);
	glClearColor(0, 1, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();

	// Fake projection matrix, scales with respect to window coordinates
	auto player = _ecs.view<Player>()[0];
	auto& playerMotion = _ecs.get<Motion>(player);
	float left = 0.f;
	float top = 0.f;
	float right = window_size_in_game_units.x;
	float bottom = window_size_in_game_units.y;

	// convert the coordinate frame of the player's position to camera
	// coordinate frame
	float playerX =
		(playerMotion.position.x) - (window_size_in_game_units.x / 2);
	float playerY =
		(playerMotion.position.y) - (window_size_in_game_units.y / 2);
	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	// add the player's translate to camera translate
	float tx = (-(right + left) / (right - left));
	float ty = (-(top + bottom) / (top - bottom));
	mat3 translate_2D{
		{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {-sx * playerX, -sy * playerY, 1.f}};
	mat3 projection{{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
	mat3 projection_2D = translate_2D * projection;

	// update frame for sprite animation - see definition of Animator struct in
	// render_components.hpp
	auto animatorView = _ecs.view<Animator>();
	for (entt::entity entity : animatorView)
	{
		auto& animator = _ecs.get<Animator>(entity);

		animator.animations[animator.curr_animation][2] += elapsed_ms;

		if (animator.animations[animator.curr_animation][2] >=
			animator.animations[animator.curr_animation][1])
		{
			animator.animations[animator.curr_animation][2] = 0.f;
			animator.frame =
				(animator.frame + 1) %
				(int)animator.animations[animator.curr_animation][0];
		}
	}

	//    // Update night timer
	//    auto& playerNight = _ecs.get<Night>(player);
	//
	//    if (playerNight.is_night == 0)
	//        playerNight.day_timer += elapsed_ms;
	//    else
	//        playerNight.night_timer += elapsed_ms;
	//
	//    if (playerNight.day_timer >= playerNight.day_duration)
	//    {
	//        playerNight.is_night = 1;
	//        playerNight.day_timer = 0.f;
	//        if (!playerNight.firstNightHasHappened)
	//        {
	//            playerNight.firstNightHasHappened = true;
	//
	//        }
	//    }
	//
	//    if (playerNight.night_timer >= playerNight.night_duration)
	//    {
	//        playerNight.is_night = 0;
	//        playerNight.night_timer = 0.f;
	//        playerNight.flashlight_on = 0;
	//    }

	// Draw all textured meshes that have a position and size component
	auto view = _ecs.view<Motion, ShadedMeshRef>();
	for (entt::entity entity : view)
	{
		// Note, its not very efficient to access elements indirectly via the
		// entity albeit iterating through all Sprites in sequence
		drawTexturedMesh(entity, projection_2D);
		gl_has_errors();
	}

	std::string wavesKey = "waves";
	ShadedMesh& wavesResource = cache_resource(wavesKey);
	if (wavesResource.effect.program.resource == 0)
	{
		wavesResource = ShadedMesh();
		RenderSystem::createSprite(wavesResource, textures_path("cloud.png"),
								   "waves"); // use a dummy texture we won't use, empty string?
	}

	auto wavesView = _ecs.view<Waves>();
	for (entt::entity entity : wavesView)
	{
		drawWaves(entity, projection_2D, wavesResource, total_ms);
		gl_has_errors();
	}
	// Draw all dirt particles
	dirtParticleSystem.draw(projection_2D);

	// Draw all dirt particles
	foliageParticleSystem.draw(projection_2D, total_ms);

	// Draw all debug lines
	auto debugLines = _ecs.view<DebugLine>();
	for (const auto& e : debugLines)
	{
		const auto& line = _ecs.get<DebugLine>(e);
		drawLine(line, projection_2D);
	}

	// Draw clouds (parallax)
	std::string cloudKey = "cloud";
	ShadedMesh& cloudResource = cache_resource(cloudKey);
	if (cloudResource.effect.program.resource == 0)
	{
		cloudResource = ShadedMesh();
		RenderSystem::createSprite(cloudResource, textures_path("cloud.png"),
								   "textured");
	}

	auto clouds = _ecs.view<Cloud, Motion>();
	for (const auto& e : clouds)
	{
		const auto& cloud = _ecs.get<Cloud>(e);
		const auto& cloudMotion = _ecs.get<Motion>(e);
		float parallaxAmount = 1 + cloud.parallaxAmount;
		mat3 cloudTranslateMatrix{{1.f, 0.f, 0.f},
								  {0.f, 1.f, 0.f},
								  {parallaxAmount * -sx * playerX,
								   parallaxAmount * -sy * playerY, 1.f}};
		mat3 cloudProj = cloudTranslateMatrix * projection;
		drawTexturedMesh(vec3{cloudMotion.position.x, cloudMotion.position.y,
							  cloudMotion.depth},
						 cloudMotion.angle,
						 {cloudMotion.scale.x * cloudResource.texture.size.x,
						  cloudMotion.scale.y * cloudResource.texture.size.y},
						 cloudResource, cloudProj);
	}

	applyLightEffect(screen_sprite.texture.texture_id);

	// Truely render to the screen
	drawToScreen();

	// Overlay the UI
	UI::drawOverTexture(ping.glTexture);

	// flicker-free display with a double buffer
	glfwSwapBuffers(&window);
}
void gl_has_errors()
{
	GLenum error = glGetError();

	if (error == GL_NO_ERROR)
		return;

	const char* error_str = "";
	while (error != GL_NO_ERROR)
	{
		switch (error)
		{
		case GL_INVALID_OPERATION:
			error_str = "INVALID_OPERATION";
			break;
		case GL_INVALID_ENUM:
			error_str = "INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			error_str = "INVALID_VALUE";
			break;
		case GL_OUT_OF_MEMORY:
			error_str = "OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			error_str = "INVALID_FRAMEBUFFER_OPERATION";
			break;
		}

		std::cerr << "OpenGL:" << error_str << std::endl;
		error = glGetError();
	}
	throw std::runtime_error("last OpenGL error:" + std::string(error_str));
}
