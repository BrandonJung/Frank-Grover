#version 330 

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Passed to fragment shader
out vec2 texcoord;

// Application data
uniform mat3 transform;
uniform mat3 projection;
uniform float u_depth;
uniform int u_frame_num;
uniform float u_frame_width;
uniform float u_frame_height;
uniform float u_curr_anim;

void main()
{
	texcoord = in_texcoord;
    texcoord.x += u_frame_width * u_frame_num;
    texcoord.y += u_frame_height * u_curr_anim;
    
	vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z + u_depth, 1.0);
}
