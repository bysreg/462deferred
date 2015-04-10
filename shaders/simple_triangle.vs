attribute vec3 a_posL; // local pos
attribute vec4 a_color;

varying vec4 v_color;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	vec4 posW = vec4(a_posL, 1.0);
	gl_Position = proj * view * world * posW; // TODO : move somewhere

	v_color = a_color;		
}