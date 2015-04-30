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

layout (location = 5) out vec4 o_light_color;

//shadow calculation
uniform sampler2D u_shadow_map;
uniform mat4 u_light_pv; // light projection matrix

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
	
	//diffuse
	diffuse_color = diffuse_color * u_light_color * max(dot(-normalize(u_light_direction), normal), 0.0);
	
	//specular
	float specular_factor = max(dot(to_eye, reflection), 0.0);
	specular_factor = pow(specular_factor, specular_power);
	specular_color = specular_color * u_light_color * specular_factor;
	
	//convert v_light_space_pos to NDC
	vec4 v_light_space_pos = u_light_pv * vec4(position, 1.0);
	vec3 ndc_pos = v_light_space_pos.xyz / v_light_space_pos.w;
	
	//convert ndc pos [-1, 1] to texcoord space [0, 1]
	vec2 shadow_map_uv;
	shadow_map_uv.x = 0.5 * ndc_pos.x + 0.5;
	shadow_map_uv.y = 0.5 * ndc_pos.y + 0.5;
	float z = 0.5 * ndc_pos.z + 0.5;
	float shadow_map_depth = texture(u_shadow_map, shadow_map_uv).x;
	
	//compare z and shadow_map_depth, z is the actual pixel depth, and shadow_map_depth contains the nearest depth to the light source
	float shadow_factor = 1.0;
	float bias = 0.001;
	if(shadow_map_depth < z - bias )
	{
		shadow_factor = 0.1;
	}
	
	//o_light_color = vec4(specular_color + diffuse_color, 1.0);
	o_light_color = vec4((specular_color + diffuse_color) * shadow_factor, 1.0);
}