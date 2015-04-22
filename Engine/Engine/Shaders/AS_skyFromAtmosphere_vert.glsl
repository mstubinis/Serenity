#version 120

attribute vec3 position;

uniform mat4 MVP1;
uniform mat4 World1;

uniform int nSamples;
uniform float fSamples;
uniform vec3 v3CameraPos;           // The camera's current position
uniform vec3 v3LightDir;            // Direction vector to the light source
uniform vec3 v3InvWavelength;       // 1 / pow(wavelength, 4) for RGB
uniform float fCameraHeight;        // The camera's current height

uniform float fCameraHeight2;       // fCameraHeight^2
uniform float fOuterRadius;         // The outer (atmosphere) radius
uniform float fOuterRadius2;        // fOuterRadius^2

uniform float fInnerRadius;         // The inner (planetary) radius
uniform float fInnerRadius2;        // fInnerRadius^2
uniform float fKrESun;              // Kr * ESun

uniform float fKmESun;              // Km * ESun
uniform float fKr4PI;               // Kr * 4 * PI
uniform float fKm4PI;               // Km * 4 * PI

uniform float fScaleDepth;

uniform float fScale;               // 1 / (fOuterRadius - fInnerRadius)
uniform float fScaleOverScaleDepth; // fScale / fScaleDepth

varying vec3 c0;
varying vec3 c1;
varying vec3 v3Direction;
varying vec3 v3LightPosition;
varying vec3 WorldPosition;

varying float Depth;
varying float cameraHeight;
varying float outerRadius;

float scale(float fCos){
	float x = 1.0 - fCos;
	return fScaleDepth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));
}
void main(){
	vec3 v3Pos = position * vec3(fOuterRadius);
	vec3 v3Ray = v3Pos - v3CameraPos;
	float fFar = length(v3Ray);
	v3Ray /= fFar;

	vec3 v3Start = v3CameraPos;
	float fHeight = length(v3Start);
	float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fCameraHeight));
	float fStartAngle = dot(v3Ray, v3Start) / fHeight;
	float fStartOffset = fDepth*scale(fStartAngle);

	float fStartDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fCameraHeight));
	Depth = clamp(fStartDepth*scale(fStartAngle),0.0,1.0);

    float fSampleLength = fFar / fSamples;
    float fScaledLength = fSampleLength * fScale;
    vec3 v3SampleRay = v3Ray * fSampleLength;
    vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

    vec3 v3FrontColor = vec3(0);
	vec3 v3Attenuate = vec3(0);
    for(int i = 0; i < nSamples; i++) {
        float fHeight = length(v3SamplePoint);
        float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));
        float fLightAngle = dot(v3LightDir, v3SamplePoint) / fHeight;
        float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;
        float fScatter = (fStartOffset + fDepth * (scale(fLightAngle) - scale(fCameraAngle)));

        vec3 v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));
        v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
        v3SamplePoint += v3SampleRay;
    }
	gl_Position = MVP1 * vec4(position, 1.0);
	gl_TexCoord[6] = MVP1 * vec4(position, 1.0);

	v3LightPosition = v3LightDir;
	v3Direction = v3CameraPos - v3Pos;

	c0 = v3FrontColor * (v3InvWavelength * fKrESun);
    c1 = v3FrontColor * fKmESun;

	cameraHeight = fCameraHeight;
	outerRadius = fOuterRadius;

	WorldPosition = (World1 * vec4(position,1.0)).xyz;
}