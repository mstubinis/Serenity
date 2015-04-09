#version 330

uniform float g;
uniform float g2;
uniform float fExposure;

in vec3 c0;
in vec3 c1;
in vec3 v3Direction;
in vec3 v3LightPosition;
in float Depth;
in float outerRadius;
in float cameraHeight;

layout(location=0)out vec4 DiffuseOut; 
layout(location=1)out vec4 NormalOut; 

void main(){
	float fCos = dot(v3LightPosition, v3Direction) / length(v3Direction);
	float fRayleighPhase = 0.75 * (1.0 + (fCos*fCos));
	float fMiePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos*fCos) / pow(1.0 + g2 - 2.0*g*fCos, 1.5);
	
	float sun = 2.0*((1.0 - 0.2) / (2.0 + 0.2)) * (1.0 + fCos*fCos) / pow(1.0 + 0.2 - 2.0*(-0.2)*fCos, 1.0);
	
	vec4 f4Ambient = (sun * Depth )*vec4(0.05, 0.05, 0.1,1.0);
	
	vec4 f4Color = (fRayleighPhase * vec4(c0,1) + fMiePhase * vec4(c1,1))+f4Ambient;
	vec4 HDR = 1.0 - exp(f4Color * -fExposure);
	float nightmult = clamp(max(HDR.x, max(HDR.y, HDR.z))*1.5,0.0,1.0);

	DiffuseOut = vec4(HDR.xyz,nightmult+(outerRadius - cameraHeight));
	NormalOut = vec4(0,0,0,1);
}