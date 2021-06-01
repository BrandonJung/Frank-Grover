#version 330

// Application data
uniform sampler2D u_sourceTexture;

// From vertex shader
in vec2 v_uv;

// Output color
layout(location = 0) out vec4 out_color;

void main()
{
	out_color = texture(u_sourceTexture, v_uv);
}
