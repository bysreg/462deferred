attribute vec3 a_posL; // local pos
attribute vec2 a_uv;

varying vec4 v_posP;
varying vec2 v_uv;

uniform mat4 u_world;
uniform mat4 u_proj_view;

void main()
{
	vec4 posW = vec4(a_posL, 1.0);
	v_posP = u_proj_view * u_world * posW;
	gl_Position = v_posP;
}