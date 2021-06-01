#version 330

// Input attributes
layout(location = 1) in vec3 i_position;
layout(location = 2) in vec2 i_uv;
layout(location = 3) in vec4 i_instanceData;

// Varyings
out vec2 v_uv;

// Uniforms
uniform mat3 u_projection;

void main()
{
	vec2 instancePos = i_instanceData.xy;
	float instanceSize = i_instanceData.z;
	float depth = i_instanceData.w;
	mat3 transformMat = mat3(
		instanceSize,0,0,
		0,instanceSize,0,
		instancePos.x,instancePos.y,1
	);

	v_uv = i_uv;

	vec3 pos = u_projection * transformMat * vec3(i_position.xy, 1.0);
	gl_Position = vec4(pos.xy, i_position.z + depth / 1000.0, 1.0);
}
