#version 330

//geometry buffer
uniform sampler2D u_g_position;
uniform sampler2D u_g_diffuse;
uniform sampler2D u_g_normal;
uniform sampler2D u_g_specular;

uniform vec2 u_screen_size;
uniform vec3 u_cam_pos;

//point light properties
uniform vec3 u_light_color;
uniform vec3 u_light_position;
uniform float u_light_const_attenuation;
uniform float u_light_linear_attenuation;
uniform float u_light_quadratic_attenuation;

layout (location = 5) out vec4 o_light_color;

void main()
{	
	vec2 geo_texcoord = gl_FragCoord.xy / u_screen_size;
	vec3 position = texture2D(u_g_position, geo_texcoord).rgb;	
	vec3 normal = texture(u_g_normal, geo_texcoord).rgb;
	vec3 diffuse_color = texture2D(u_g_diffuse, geo_texcoord).rgb;
	vec3 specular_color = texture(u_g_specular, geo_texcoord).rgb;
	float specular_power = texture(u_g_specular, geo_texcoord).a;
	
	vec3 to_eye = normalize(u_cam_pos - position);	
	vec3 light_dir = position - u_light_position;
	vec3 reflection = normalize(reflect(light_dir, normal));
	float distance = length(light_dir);
	float attenuation = max(1.0, u_light_const_attenuation + u_light_linear_attenuation * distance + u_light_quadratic_attenuation * distance * distance);
	
	//diffuse
	diffuse_color = diffuse_color * u_light_color * max(dot(-normalize(light_dir), normal), 0.0);
	
	//specular
	float specular_factor = max(dot(to_eye, reflection), 0.0);
	specular_factor = pow(specular_factor, specular_power);
	specular_color = specular_color * u_light_color * specular_factor;

	//o_light_color = vec4(diffuse_color, 1.0);
	//o_light_color = vec4(specular_color, 1.0);
	//o_light_color = vec4((diffuse_color + specular_color), 1.0);
	//o_light_color = vec4(distance, distance, distance, 1.0);
	//o_light_color = vec4((diffuse_color + specular_color) / attenuation, 1.0);
	//o_light_color = vec4(1/attenuation, 1/attenuation, 1/attenuation, 1.0);
	//o_light_color = vec4(attenuation, attenuation, attenuation, 1.0);
	o_light_color = vec4(1, 1, 1, 1.0);
}