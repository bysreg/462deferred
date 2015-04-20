precision mediump float;

varying vec4 v_color;
varying vec4 v_posP;
varying vec2 v_uv;
varying vec3 v_normalL;
varying vec4 v_posW;

uniform sampler2D u_diffuse_texture;

void main()
{	
	//gl_FragColor = vec4(v_posP.z / 20.0); // display depth, divided by 20 so that it will display more progressive 
	//gl_FragColor = texture2D(u_diffuse_texture, v_uv); // display diffuse
	//gl_FragColor = vec4(v_uv, 0.0, 1.0); // display uv
	//gl_FragColor = vec4(v_normalL, 1.0); // display normal
	gl_FragColor = vec4(v_posW.xyz, 1.0); // display world position
}