#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	vec4 sample = texture(sampler0, vec2(texcoord.x, texcoord.y));

	// Alpha clipping
	if (sample.a <= 0.1)
		discard;

	color = vec4(fcolor, 1.0) * sample;
}
