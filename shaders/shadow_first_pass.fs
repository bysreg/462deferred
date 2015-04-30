#version 330

layout (location = 0) out vec4 o_color; // for debug

in vec2 v_uv;
uniform sampler2D u_diffuse_texture;
uniform sampler2D u_shadow_map;

void main()
{
	float scale = 5;

	//o_color = vec4(gl_FragCoord.z / scale, gl_FragCoord.z / scale, gl_FragCoord.z / scale, 1);
	
	o_color = texture2D(u_diffuse_texture, v_uv); // display diffuse

}