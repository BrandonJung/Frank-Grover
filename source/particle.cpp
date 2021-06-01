// Header
#include "particle.hpp"
#include "render.hpp"

entt::entity DirtParticle::create(vec2 position, vec2 velocity, vec2 scale)
{
	// Reserve en entity
	auto entity = _ecs.create();

	// Initialize the position, scale, and physics components
	auto& motion = _ecs.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = velocity;
	motion.position = position;
	motion.scale = scale;
	motion.depth = -100;

	_ecs.emplace<DirtParticle>(entity, DirtParticle{});

	return entity;
}

void updateDirtParticles(float deltaTime)
{
	const float shrinkSpeed = 60.0f;
	const vec2 gravity = vec2{0, 981.f / 2.f};
	auto particles = _ecs.view<DirtParticle, Motion>();
	for (auto [e, motion] : particles.each())
	{
		motion.scale.x -= deltaTime * shrinkSpeed;

		if(motion.scale.x < 1)
		{
			_ecs.destroy(e);
			continue;
		} 
		motion.position += motion.velocity * deltaTime;
		motion.velocity += gravity * deltaTime;
	}
}

DirtParticleSystem DirtParticleSystem::create()
{
	DirtParticleSystem dps{};
	glGenBuffers(1, &dps.buffer.resource);
	gl_has_errors();

	RenderSystem::createSprite(dps.dirtMesh, textures_path("dirt_particle.png"),
							   "dirtParticles");

	return dps;
}

void DirtParticleSystem::updateBuffer()
{
	auto view = _ecs.view<DirtParticle, Motion>();
	std::vector<GPUParticleData> data(1000);

	// Recount particles as we update buffer data
	bufferCount = 0;
	for (auto [e, motion] : view.each())
	{
		GPUParticleData pd;
		pd.x = motion.position.x;
		pd.y = motion.position.y;
		pd.size = motion.scale.x;
		pd.depth = motion.depth;
		data[bufferCount] = pd;
		++bufferCount;
	}

	// Update buffer data
	glBindBuffer(GL_ARRAY_BUFFER, buffer.resource);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GPUParticleData) * bufferCount,
				 data.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind buffer
}

void DirtParticleSystem::draw(const mat3& projection)
{
	updateBuffer();

	if (bufferCount < 1)
		return; // Early return as there are no particles
	// Logger::info("Drawing dirt ", bufferCount);

	// Bind material/buffers & draw

	// Setting shaders
	glUseProgram(dirtMesh.effect.program);
	gl_has_errors();

	// Enabling alpha channel for textures
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	gl_has_errors();

	// Setting vertex and index buffers
	glBindVertexArray(dirtMesh.mesh.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dirtMesh.mesh.ibo);
	glBindBuffer(GL_ARRAY_BUFFER, dirtMesh.mesh.vbo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	GLint projection_uloc =
		glGetUniformLocation(dirtMesh.effect.program, "u_projection");
	GLint in_position_loc =
		glGetAttribLocation(dirtMesh.effect.program, "i_position");
	GLint in_uv_loc = glGetAttribLocation(dirtMesh.effect.program, "i_uv");

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
	// Logger::info("Bound per vertex attributes ", in_position_loc, " ", in_uv_loc);
	gl_has_errors();

	GLint instance_data_uloc =
		glGetAttribLocation(dirtMesh.effect.program, "i_instanceData");
	glBindBuffer(GL_ARRAY_BUFFER, this->buffer.resource);
	glEnableVertexAttribArray(instance_data_uloc);
	glVertexAttribPointer(instance_data_uloc, 4, GL_FLOAT, GL_FALSE,
						  sizeof(vec4), reinterpret_cast<void*>(0));
	glVertexAttribDivisor(instance_data_uloc, 1);
	// Logger::info("Bound per instance attributes ", instance_data_uloc);
	gl_has_errors();

	// Enabling and binding texture to slot 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, dirtMesh.texture.texture_id);
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
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	// Logger::info("Drawing dirt!");
	glDrawElementsInstanced(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT,
							nullptr, this->bufferCount);

	glBindVertexArray(0);
	glDisable(GL_DEPTH_TEST);
}