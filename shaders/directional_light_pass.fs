#version 330

precision mediump float;

in vec3 v_posP;
in vec2 v_uv;
in vec3 v_normalL;
in vec3 v_normalW;
in vec3 v_posW;

//geometry buffer
uniform sampler2D u_g_position;
uniform sampler2D u_g_color;
uniform sampler2D u_g_normal;
uniform sampler2D u_g_uv;

out vec4 light_color;

void main()
{	
	o_diffuse = texture2D(u_diffuse_texture, v_uv).xyz; // display diffuse
	o_uv = vec3(v_uv, 0.0); // display uv
	o_normalW = normalize(v_normalW); // display world normal
	o_posW = v_posW; // display world position	
}