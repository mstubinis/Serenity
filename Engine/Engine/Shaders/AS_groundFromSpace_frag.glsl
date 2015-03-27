#version 330

uniform float fExposure;

in vec3 c0;
in vec3 c1;

void main(){
	gl_FragColor.rgb = 1.0 - exp( -fExposure * (c0 * c1) );
	gl_FragColor.a = 1.0;
}