#version 330

uniform float g;
uniform float g2;
uniform float fExposure;

in vec3 c0;
in vec3 c1;
in vec3 v3Direction;
in vec3 v3LightPosition;

void main(){
	float fCos = dot(v3LightPosition, v3Direction) / length(v3Direction);	
	float fRayleighPhase = 0.75 * (1.0 + fCos*fCos);	
	float fMiePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos*fCos) / pow(1.0 + g2 - 2.0*g*fCos, 1.5);	
	gl_FragColor.rgb = 1.0 - exp( -fExposure * (fRayleighPhase * c0 + fMiePhase * c1) );
	gl_FragColor.a = 1.0; 
}