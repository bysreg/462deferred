#version 330

layout (location = 0) out vec4 o_color;

in vec2 v_uv;
uniform sampler2D u_shadow_map;

void main()
{
	float depth = texture(u_shadow_map, v_uv).x;
	depth = 1.0 - (1.0 - depth) * 25.0;
	o_color = vec4(depth);
	//o_color = vec4(1, 0, 0, 1);
}