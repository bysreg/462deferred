precision mediump float;

varying vec4 v_color;
varying vec4 v_posP;
varying vec2 v_uv;

uniform sampler2D u_diffuse_texture;

void main()
{	
	gl_FragColor = vec4(v_posP.z / 20.0); // display depth, 
	//gl_FragColor = texture2D(u_diffuse_texture, );
}