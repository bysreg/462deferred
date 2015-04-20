attribute vec3 a_posL; // local pos
attribute vec2 a_uv;
attribute vec3 a_normalL;

varying vec4 v_posP;
varying vec2 v_uv;
varying vec3 v_normalL;

uniform mat4 u_world;
uniform mat4 u_proj_view;

void main()
{
	v_uv = a_uv;
	v_normalL = a_normalL;
	
	vec4 posW = vec4(a_posL, 1.0);
	v_posP = u_proj_view * u_world * posW;
	gl_Position = v_posP;
}