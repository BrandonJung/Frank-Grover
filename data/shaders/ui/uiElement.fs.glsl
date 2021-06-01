#version 330

// Application data
uniform sampler2D u_texture;
uniform vec4 u_color;

// From vertex shader
in vec2 v_uv;

// Output color
layout(location = 0) out vec4 out_color;

void main()
{
	vec4 sample = texture(u_texture, vec2(v_uv.x, 1 - v_uv.y));

	// Alpha clipping
	if (sample.a <= 0.15)
		discard;

	out_color = u_color * sample;
}
