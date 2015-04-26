#version 330

in vec3 v_posP;
in vec2 v_uv;
in vec3 v_normalL;
in vec3 v_normalW;
in vec3 v_posW;

//geometry buffer
uniform sampler2D u_g_position;
uniform sampler2D u_g_diffuse;
uniform sampler2D u_g_normal;
uniform sampler2D u_g_specular;

uniform vec2 u_screen_size;

//directional light properties
uniform vec3 u_light_direction;
uniform vec3 u_light_color;

out vec4 o_light_color;

void main()
{	
	vec2 geo_texcoord = gl_FragCoord.xy / u_screen_size;
	vec3 position = texture2D(u_g_position, geo_texcoord).xyz;	
	vec3 normal = texture(u_g_normal, geo_texcoord).xyz;
	vec3 color = texture2D(u_g_diffuse, geo_texcoord).xyz;
	vec3 specular = texture(u_g_specular, geo_texcoord).xyz;
	o_light_color += u_light_color * max(dot(-normalize(u_light_direction), normal), 0.0) * color;	
}