#version 330

// Input attributes
layout(location = 0) in vec4 in_positionAndUV;

// Passed to fragment shader
out vec2 v_uv;

void main()
{
	vec2 pos = in_positionAndUV.xy;
	vec2 uv = in_positionAndUV.zw;
	v_uv = uv;
	gl_Position = vec4(pos.xy, 0, 1);
}