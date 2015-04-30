#version 330

precision mediump float;

in vec3 v_posP;
in vec2 v_uv;
in vec3 v_normalL;
in vec3 v_normalW;
in vec3 v_posW;

uniform sampler2D u_diffuse_texture;

out vec4 o_color;

void main()
{	
	//o_color = vec4(v_posP.z / 20.0); // display depth, divided by 20 so that it will display more progressive 		
	//o_color = vec4(v_posW, 1.0); // display world position	
	o_color = texture2D(u_diffuse_texture, v_uv); // display diffuse
	//o_color = vec4(1, 0, 0, 1);
	//o_color = vec4(v_normalW, 1.0); // display world normal
	//o_color = vec4(v_uv, 0.0, 1.0); // display uv
}