precision mediump float;

varying vec4 v_color;
varying vec4 a_posP;

void main()
{	
	gl_FragColor = vec4(a_posP.z / 20.0); // display depth, 
	//gl_FragColor = v_color;
	//gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);	
}