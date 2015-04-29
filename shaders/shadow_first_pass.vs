#version 330

in vec3 a_posL; // local pos

out vec3 v_posW;

uniform mat4 u_proj_view_world;

void main()
{
	gl_Position = (u_proj_view_world * vec4(a_posL, 1.0));
}