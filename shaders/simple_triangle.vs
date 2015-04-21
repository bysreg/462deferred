#version 330

in vec3 a_posL; // local pos
in vec2 a_uv;
in vec3 a_normalL;

out vec4 v_posP;
out vec2 v_uv;
out vec3 v_normalL;
out vec4 v_posW;

uniform mat4 u_world;
uniform mat4 u_proj_view;

void main()
{
	v_uv = a_uv;
	v_normalL = a_normalL;
	
	v_posW = u_world * vec4(a_posL, 1.0);
	v_posP = u_proj_view * v_posW;
	gl_Position = v_posP;
}