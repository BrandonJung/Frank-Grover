// Header
#include "particle.hpp"
#include "render.hpp"

entt::entity FoliageParticle::create(vec2 position, vec2 size, float depth)
{
	// Reserve en entity
	auto entity = _ecs.create();

	auto& particle = _ecs.emplace<FoliageParticle>(entity, FoliageParticle{});
	particle.position = position;
	particle.depth = depth;
	particle.size = size;

	return entity;
}

FoliageParticleSystem FoliageParticleSystem::create()
{
	FoliageParticleSystem dps{};
	glGenBuffers(1, &dps.buffer.resource);
	gl_has_errors();

	RenderSystem::createSprite(dps.foliageMesh, textures_path("grass.png"),
							   "foliageParticles");

	return dps;
}

void FoliageParticleSystem::updateBuffer()
{
	auto view = _ecs.view<FoliageParticle>();
	std::vector<GPUFoliageData> data(1000);

	// Recount particles as we update buffer data
	count = 0;
	for (auto [e, particle] : view.each())
	{
		GPUFoliageData pd;
		pd.x = particle.position.x;
		pd.y = particle.position.y;
		pd.z = particle.depth;
		pd.size = particle.size.x;
		data[count] = pd;
		++count;
	}

	// Update buffer data
	glBindBuffer(GL_ARRAY_BUFFER, buffer.resource);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GPUParticleData) * count, data.data(),
				 GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind buffer
}

void FoliageParticleSystem::draw(const mat3& projection, float totalTimeMS)
{
	updateBuffer();

	if (count < 1)
		return; // Early return as there are no particles
	// Logger::info("Drawing foliage ", bufferCount);

	// Bind material/buffers & draw

	// Setting shaders
	glUseProgram(foliageMesh.effect.program);
	gl_has_errors();

	// Enabling alpha channel for textures
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	gl_has_errors();

	// Setting vertex and index buffers
	glBindVertexArray(foliageMesh.mesh.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, foliageMesh.mesh.ibo);
	glBindBuffer(GL_ARRAY_BUFFER, foliageMesh.mesh.vbo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	GLint projection_uloc =
		glGetUniformLocation(foliageMesh.effect.program, "u_projection");
	GLint time_uloc =
		glGetUniformLocation(foliageMesh.effect.program, "u_time");
	GLint in_position_loc =
		glGetAttribLocation(foliageMesh.effect.program, "i_position");
	GLint in_uv_loc = glGetAttribLocation(foliageMesh.effect.program, "i_uv");

	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
						  sizeof(TexturedVertex), reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(in_uv_loc);
	glVertexAttribPointer(
		in_uv_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
		reinterpret_cast<void*>(sizeof(vec3))); // note the stride to skip the
												// preceeding vertex position

	glVertexAttribDivisor(in_position_loc, 0);
	glVertexAttribDivisor(in_uv_loc, 0);
	// Logger::info("Bound per vertex attributes ", in_position_loc, " ",
	// in_uv_loc);
	gl_has_errors();

	GLint instance_data_uloc =
		glGetAttribLocation(foliageMesh.effect.program, "i_instanceData");
	glBindBuffer(GL_ARRAY_BUFFER, this->buffer.resource);
	glEnableVertexAttribArray(instance_data_uloc);
	glVertexAttribPointer(instance_data_uloc, 4, GL_FLOAT, GL_FALSE,
						  sizeof(vec4), reinterpret_cast<void*>(0));
	glVertexAttribDivisor(instance_data_uloc, 1);
	// Logger::info("Bound per instance attributes ", instance_data_uloc);
	gl_has_errors();

	// Enabling and binding texture to slot 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, foliageMesh.texture.texture_id);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	// Logger::info("Getting size!");
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();
	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	// Setting uniform values to the currently bound program
	// Logger::info("Setting uniforms!");
	glUniform1f(time_uloc, totalTimeMS);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	// Logger::info("Drawing foliage!");
	glDrawElementsInstanced(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT,
							nullptr, this->count);

	glBindVertexArray(0);
	glDisable(GL_DEPTH_TEST);
}