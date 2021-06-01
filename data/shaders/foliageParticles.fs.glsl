#version 330

// From vertex shader
in vec2 v_uv;

// Application data
uniform sampler2D u_sampler0;
// uniform float u_time; // in seconds

// Output color
layout(location = 0) out vec4 o_color;

void main()
{
	vec4 sample = texture(u_sampler0, v_uv);

	// Alpha clipping
	if (sample.a <= 0.1)
		discard;

	o_color = sample;
}
