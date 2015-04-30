#version 330

layout (location = 0) out vec4 color; // for debug

in vec2 v_uv;
uniform sampler2D u_diffuse_texture;

void main()
{
	float scale = 5;

	//color = vec4(gl_FragCoord.z / scale, gl_FragCoord.z / scale, gl_FragCoord.z / scale, 1);
	
	color = texture2D(u_diffuse_texture, v_uv); // display diffuse
}