#version 330

layout (location=0) in vec3 position;

uniform mat4 MVP;

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

out vec3 c0;
out vec3 c1;
out vec3 v3Direction;
out vec3 v3LightPosition;

float scale(float fCos){
	float x = 1.0 - fCos;
	return fScaleDepth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));
}
float getNearIntersection(vec3 v3Pos, vec3 v3Ray, float fDistance2, float fRadius2){
	float B = 2.0 * dot(v3Pos, v3Ray);
	float C = fDistance2 - fRadius2;
	float fDet = max(0.0, B*B - 4.0 * C);
	return 0.5 * (-B - sqrt(fDet));
}
void main(){
	vec3 v3Pos = position;
	vec3 v3Ray = v3Pos - v3CameraPos;
	float fFar = length(v3Ray);
	v3Ray /= fFar;

    float fNear = getNearIntersection(v3CameraPos, v3Ray, fCameraHeight2,fOuterRadius2);

    vec3 v3Start = v3CameraPos + v3Ray * fNear;
    fFar -= fNear;

    float fStartAngle = dot(v3Ray, v3Start) / fOuterRadius;
    float fStartDepth = exp(-(1.0 / fScaleDepth));
    float fStartOffset = fStartDepth * scale(fStartAngle);

    float fSampleLength = fFar / fSamples;
    float fScaledLength = fSampleLength * fScale;
    vec3 v3SampleRay = v3Ray * fSampleLength;
    vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

    vec3 v3FrontColor = vec3(0,0,0);
    for(int i = 0; i < nSamples; i++) {
        float fHeight = length(v3SamplePoint);
        float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));
        float fLightAngle = dot(v3LightDir, v3SamplePoint) / fHeight;
        float fCameraAngle = 1.0f;
        float fScatter = (fStartOffset + fDepth * (scale(fLightAngle) - scale(fCameraAngle)));

        vec3 v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));
        v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
        v3SamplePoint += v3SampleRay;
    }
    // Finally, scale the Mie and Rayleigh colors
	gl_Position = MVP * vec4(position, 1.0);
	v3LightPosition = v3LightDir;
	v3Direction = v3CameraPos - v3Pos;
	c0 = v3FrontColor * (v3InvWavelength * fKrESun);
    c1 = v3FrontColor * fKmESun;
}