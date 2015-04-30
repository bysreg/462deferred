#version 330

in vec3 a_posL; // local pos
in vec2 a_uv;

out vec2 v_uv;

uniform mat4 u_proj_view_world;

void main()
{
	v_uv = a_uv;
	//gl_Position = (u_proj_view_world * vec4(a_posL, 1.0));
	gl_Position = vec4(a_posL, 1.0);
}