#version 330

// Application data
uniform mat4 u_transform;
uniform float u_depth;

// Input attributes
layout(location = 0) in vec4 in_positionAndUV;

// Passed to fragment shader
out vec2 v_uv;

void main()
{
	vec2 pos = in_positionAndUV.xy;
	vec2 uv = in_positionAndUV.zw;

	v_uv = uv;
	gl_Position = u_transform * vec4(pos.xy, u_depth, 1);
}