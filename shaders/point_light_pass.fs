#version 330

//geometry buffer
uniform sampler2D u_g_position;
uniform sampler2D u_g_diffuse;
uniform sampler2D u_g_normal;
uniform sampler2D u_g_specular;

uniform vec2 u_screen_size;

//point light properties
uniform vec3 u_light_color;
uniform vec3 u_light_position;
uniform float u_light_const_attenuation;
uniform float u_light_linear_attenuation;
uniform float u_light_quadratic_attenuation;

uniform float u_a;

out vec4 o_light_color;

void main()
{	
	vec2 geo_texcoord = gl_FragCoord.xy / u_screen_size;
	vec3 position = texture2D(u_g_position, geo_texcoord).xyz;	
	vec3 normal = texture(u_g_normal, geo_texcoord).xyz;
	vec3 color = texture2D(u_g_diffuse, geo_texcoord).xyz;
	vec3 specular = texture(u_g_specular, geo_texcoord).xyz;
	vec3 light_dir = position - u_light_position;
	float distance = length(light_dir);
	float attenuation = max(1.0, u_light_const_attenuation + u_light_linear_attenuation * distance + u_light_quadratic_attenuation * distance * distance);
	
	vec3 calc_light_color = u_light_color * max(dot(-normalize(light_dir), normal), 0.0);
	calc_light_color = calc_light_color / attenuation;
	
	o_light_color = vec4(calc_light_color, 1.0);
}