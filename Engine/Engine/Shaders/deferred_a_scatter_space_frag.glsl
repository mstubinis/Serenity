#version 330

uniform float g;
uniform float g2;

in vec3 c0;
in vec3 c1;
in vec3 v3Direction;
in vec3 v3LightPosition;
in float PixelToCamera;

layout (location = 0) out vec4 DiffuseOut; 

float getMiePhase(float fCos, float fCos2, float g, float g2){
	return 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos2) / pow(abs(1.0 + g2 - 2.0*g*fCos), 1.5);
}
float getRayleighPhase(float fCos2){
	//return 1.0;
	return 0.75 + 0.75*fCos2;
}
void main(){
	float fCos = dot(v3LightPosition, v3Direction) / length(v3Direction);
	float fCos2 = fCos*fCos;
	float fMiePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos2) / pow(1.0 + g2 - 2.0*g*fCos, 1.5);
	float fRayPhase = 0.75 + (0.75*fCos2);

	DiffuseOut.rgb = (fRayPhase * c0) + (fMiePhase * c1);
	DiffuseOut.a = gl_FragColor.b;
}