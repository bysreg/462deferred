attribute vec3 a_posL; // local pos
attribute vec4 a_color;

varying vec4 v_color;
varying vec4 a_posP;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	vec4 posW = vec4(a_posL, 1.0);
	a_posP = proj * view * world * posW;
	gl_Position = a_posP;
	
	v_color = a_color;			
}