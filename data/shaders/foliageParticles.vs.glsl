#version 330

// Input attributes
layout(location = 1) in vec3 i_position;
layout(location = 2) in vec2 i_uv;
layout(location = 3) in vec4 i_instanceData;

// Varyings
out vec2 v_uv;

// Uniforms
uniform mat3 u_projection;
uniform float u_time; // in seconds

void main()
{
	vec2 instancePos = i_instanceData.xy;
	float depth = i_instanceData.z;
	float instanceSize = i_instanceData.w;

	mat3 transformMat = mat3(
		instanceSize,0,0,
		0,instanceSize,0,
		instancePos.x,instancePos.y,1
	);

	v_uv = i_uv;

	// Skew
	float totalSeconds = u_time;
	float animRate = 1000;
	float skewAmount = 15;
	float skewXOffset = sin(instancePos.x / 10000.0) * 17.0;
	float skewYOffset = sin(instancePos.y / 10000.0) * 19.0;
	float skew = sin((totalSeconds) / animRate + skewXOffset + skewYOffset) * skewAmount;
	float skewT = 1 - i_position.y;

	vec3 pos = transformMat * vec3(i_position.x, i_position.y, 1.0);
	pos.x = pos.x + skewT * skew;

	// Projection
	vec3 projPos = u_projection * pos;

	gl_Position = vec4(projPos.xy, i_position.z + depth / 1000.0, 1.0);
}
