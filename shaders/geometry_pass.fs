#version 330

in vec3 v_posP;
in vec2 v_uv;
in vec3 v_normalL;
in vec3 v_normalW;
in vec3 v_posW;

uniform sampler2D u_diffuse_texture;
uniform float u_specular_power;
uniform vec3 u_ambient;
uniform vec3 u_diffuse;
uniform vec3 u_specular;

//geometry buffer
layout (location = 0) out vec3 o_posW;
layout (location = 1) out vec3 o_diffuse;
layout (location = 2) out vec3 o_normalW;
layout (location = 3) out vec3 o_uv;
layout (location = 4) out vec4 o_specular;

void main()
{		
	vec4 texture_color = texture2D(u_diffuse_texture, v_uv);
	o_diffuse = u_diffuse * texture_color.xyz; // display texture color multiplied with material diffuse color
	o_uv = vec3(v_uv, 0.0); // display uv
	o_normalW = normalize(v_normalW); // display world normal
	o_posW = v_posW; // display world position	
	o_specular = vec4(u_specular * texture_color.xyz, u_specular_power);
}