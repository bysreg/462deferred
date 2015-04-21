#version 330

precision mediump float;

in vec4 v_color;
in vec4 v_posP;
in vec2 v_uv;
in vec3 v_normalL;
in vec4 v_posW;

uniform sampler2D u_diffuse_texture;

out vec4 o_color;

void main()
{	
	//o_color = vec4(v_posP.z / 20.0); // display depth, divided by 20 so that it will display more progressive 
	//o_color = texture2D(u_diffuse_texture, v_uv); // display diffuse
	//o_color = vec4(v_uv, 0.0, 1.0); // display uv
	//o_color = vec4(v_normalL, 1.0); // display normal
	o_color = vec4(v_posW.xyz, 1.0); // display world position	
}