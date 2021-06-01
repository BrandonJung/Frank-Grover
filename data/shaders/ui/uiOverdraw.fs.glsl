#version 330

// Application data
uniform sampler2D u_sourceTexture;
uniform sampler2D u_uiTexture;

// From vertex shader
in vec2 v_uv;

// Output color
layout(location = 0) out vec4 out_color;

void main()
{
	vec4 uiSample = texture(u_uiTexture, v_uv);
	vec4 sourceSample = texture(u_sourceTexture, v_uv);
	vec4 merged = sourceSample * (1 - uiSample.a) + uiSample.a * uiSample;
	out_color = vec4(merged.xyz, 1.0);
}
