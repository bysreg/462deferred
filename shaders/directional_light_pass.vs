#version 330

in vec3 a_posL; // local pos
in vec2 a_uv;
in vec3 a_normalL;

out vec3 v_posP;
out vec2 v_uv;
out vec3 v_normalL;
out vec3 v_normalW;
out vec3 v_posW;

uniform mat4 u_world;
uniform mat4 u_proj_view;

void main()
{
	gl_Position = vec4(a_posL, 1.0);
}