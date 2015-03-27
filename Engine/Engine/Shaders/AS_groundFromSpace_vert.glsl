#version 330

layout (location=0) in vec3 position;

uniform mat4 MVP;

uniform vec3 v3CameraPos;
uniform vec3 v3LightDir;		// The direction vector to the light source 
uniform vec3 v3InvWavelength;	// 1 / pow(wavelength, 4) for the red, green, and blue channels 
uniform float fCameraHeight2;	// fCameraHeight^2 
uniform float fOuterRadius;		// The outer (atmosphere) radius 
uniform float fOuterRadius2;	// fOuterRadius^2 
uniform float fInnerRadius;		// The inner (planetary) radius 
uniform float fInnerRadius2;	// fInnerRadius^2 
uniform float fKrESun;			// Kr * ESun 
uniform float fKmESun;			// Km * ESun 
uniform float fKr4PI;			// Kr * 4 * PI 
uniform float fKm4PI;			// Km * 4 * PI 
uniform float fScale;			// 1 / (fOuterRadius - fInnerRadius) 
uniform float fScaleDepth;		// The scale depth (i.e. the altitude at which the atmosphere's average density is found) 
uniform float fScaleOverScaleDepth;	// fScale / fScaleDepth 	
const int iSamples = 4; 
const float fInvSamples = 0.25; 

out vec3 c0;
out vec3 c1;

float scale(float fCos)	{	
	float x = 1.0 - fCos;	
	return fScaleDepth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));	
}
float getNearIntersection(vec3 _p, vec3 _r, float _d2, float _r2){
	float B = 2.0 * dot(_p, _r);
	float C = _d2 - _r2;
	float fDet = max(0.0, B*B - 4.0 * C);
	return 0.5 * (-B - sqrt(fDet));
}
void main(void)	{	
	vec3 v3Pos = normalize(position);
	vec3 v3Ray = v3Pos - v3CameraPos;
	float fFar = length(v3Ray);	
	v3Ray /= fFar;	
	
	float fNear = getNearIntersection(v3CameraPos, v3Ray, fCameraHeight2, fOuterRadius2);
	
	vec3 v3Start = v3CameraPos + v3Ray * fNear;	
	fFar -= fNear;	
	float fDepth = exp((fInnerRadius - fOuterRadius) / fScaleDepth);	
	float fCameraAngle = dot(-v3Ray, v3Pos);	
	float fLightAngle = dot(v3LightDir, v3Pos);	
	float fCameraScale = scale(fCameraAngle);	
	float fLightScale = scale(fLightAngle);	
	float fCameraOffset = fDepth*fCameraScale;	
	float fTemp = (fLightScale + fCameraScale);	
	
	float fSampleLength = fFar * fInvSamples;	
	float fScaledLength = fSampleLength * fScale;	
	vec3 v3SampleRay = v3Ray * fSampleLength;	
	vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;	
	
	vec3 v3FrontColor = vec3(0,0,0);	
	vec3 v3Attenuate;
	for(int i = 0; i < iSamples; i++)	{	
		float fHeight = length(v3SamplePoint);	
		float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));	
		float fScatter = fDepth*fTemp - fCameraOffset;	
		v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));	
		v3FrontColor += v3Attenuate * (fDepth * fScaledLength);	
		v3SamplePoint += v3SampleRay;	
	}	
	
	c0 = v3FrontColor * (v3InvWavelength * fKrESun + fKmESun);	
	c1 = v3Attenuate;	
	
	gl_Position = MVP * vec4(position,1);
}