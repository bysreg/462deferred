#version 330

//geometry buffer
uniform sampler2D u_g_position;
uniform sampler2D u_g_diffuse;
uniform sampler2D u_g_normal;
uniform sampler2D u_g_specular;

uniform vec2 u_screen_size;
uniform vec3 u_cam_pos;

//directional light properties
uniform vec3 u_light_direction;
uniform vec3 u_light_color;

out vec4 o_light_color;

void main()
{	
	vec2 geo_texcoord = gl_FragCoord.xy / u_screen_size;	
	vec3 position = texture2D(u_g_position, geo_texcoord).rgb;
	vec3 normal = texture(u_g_normal, geo_texcoord).rgb;
	vec3 diffuse_color = texture2D(u_g_diffuse, geo_texcoord).rgb;
	vec3 specular_color = texture(u_g_specular, geo_texcoord).rgb;
	float specular_power = texture(u_g_specular, geo_texcoord).a;
	vec3 to_eye = normalize(u_cam_pos - position);
	vec3 reflection = normalize(reflect(u_light_direction, normal));	
	
	diffuse_color = diffuse_color * u_light_color * max(dot(-normalize(u_light_direction), normal), 0.0);
	float specular_factor = max(dot(to_eye, reflection), 0.0);
	specular_factor = pow(specular_factor, specular_power);
	specular_color = specular_color * u_light_color * specular_factor;
	
	o_light_color = vec4(specular_color + diffuse_color, 1.0);
}