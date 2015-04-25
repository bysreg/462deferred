#version 330

precision mediump float;

in vec3 v_posP;
in vec2 v_uv;
in vec3 v_normalL;
in vec3 v_normalW;
in vec3 v_posW;

uniform sampler2D u_diffuse_texture;
uniform float u_specular_power;

//geometry buffer
layout (location = 0) out vec3 o_posW;
layout (location = 1) out vec4 o_diffuse;
layout (location = 2) out vec3 o_normalW;
layout (location = 3) out vec3 o_uv;

void main()
{	
	o_diffuse = vec4(texture2D(u_diffuse_texture, v_uv).xyz, u_specular_power); // display diffuse and specular power
	o_uv = vec3(v_uv, 0.0); // display uv
	o_normalW = normalize(v_normalW); // display world normal
	o_posW = v_posW; // display world position	
}