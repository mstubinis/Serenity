
USE_LOG_DEPTH_VERTEX

layout (location = 0) in vec3 position;
//layout (location = 1) in float uv;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 normal;
layout (location = 3) in vec4 binormal;
layout (location = 4) in vec4 tangent;
layout (location = 5) in vec4 BoneIDs;
layout (location = 6) in vec4 Weights;

uniform mat4 Model;
uniform int nSamples;

uniform vec4 VertDataMisc1;         //xyz = camPos,                              w = lightDir.x
uniform vec4 VertDataMisc2;         //x = camHeight, y = camHeight2, z = fDepth, w = lightDir.y
uniform vec4 VertDataMisc3;         //xyz = v3InvWaveLength,                     w = lightDir.z
uniform vec4 VertDataScale;         //fScale, fScaledepth, fScaleOverScaleDepth, fSamples
uniform vec4 VertDataRadius;        //out, out2, inn, inn2
uniform vec4 VertDatafK;            //fKrESun, fKmESun, fKr4PI, fKm4PI

varying vec3 c0;
varying vec3 c1;
varying vec3 v3Direction;
varying vec3 v3LightPosition;
varying vec3 WorldPosition;

varying float Depth;
varying float cameraHeight;
varying float outerRadius;
varying float planetRadius;

float scale(float fCos){
    float x = 1.0 - fCos;
    return VertDataScale.y * exp(-0.00287 + x * (0.459 + x * (3.83 + x * (-6.80 + x * 5.25))));
}
void main(){
    mat4 ModelClone = Model;
    ModelClone[3][0] -= CameraRealPosition.x;
    ModelClone[3][1] -= CameraRealPosition.y;
    ModelClone[3][2] -= CameraRealPosition.z;

    vec3 v3Pos = position * vec3(VertDataRadius.x);
    vec3 v3Ray = v3Pos - VertDataMisc1.xyz;
    vec3 v3LightDir = vec3(VertDataMisc1.w,VertDataMisc2.w,VertDataMisc3.w);
    float fFar = length(v3Ray);
    v3Ray /= fFar;

    vec3 v3Start = VertDataMisc1.xyz;
    float fHeight = length(v3Start);
    float fStartAngle = dot(v3Ray, v3Start) / fHeight;
    float fStartAngleScale = scale(fStartAngle);
    float fStartOffset = VertDataMisc2.z * fStartAngleScale;

    Depth = clamp(VertDataMisc2.z * fStartAngleScale,0.0,1.0);

    float fSampleLength = fFar / VertDataScale.w;
    float fScaledLength = fSampleLength * VertDataScale.x;
    vec3 v3SampleRay = v3Ray * fSampleLength;
    vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

    vec3 v3FrontColor = vec3(0.0);
    vec3 v3Attenuate = vec3(0.0);
    for(int i = 0; i < nSamples; ++i) {
        float fHeight = length(v3SamplePoint);
        float fDepth = exp(VertDataScale.z * (VertDataRadius.z - fHeight));
        float fLightAngle = dot(v3LightDir, v3SamplePoint) / fHeight;
        float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;
        float fScatter = (fStartOffset + fDepth * (scale(fLightAngle) - scale(fCameraAngle)));

        vec3 v3Attenuate = exp(-fScatter * (VertDataMisc3.xyz * VertDatafK.z + VertDatafK.w));
        v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
        v3SamplePoint += v3SampleRay;
    }
    gl_Position = CameraViewProj * ModelClone * vec4(position, 1.0);

    v3LightPosition = v3LightDir;
    v3Direction = VertDataMisc1.xyz - v3Pos;

    c0 = v3FrontColor * (VertDataMisc3.xyz * VertDatafK.x);
    c1 = v3FrontColor * VertDatafK.y;

    cameraHeight = VertDataMisc2.x;
    outerRadius = VertDataRadius.x;
    planetRadius = VertDataRadius.z;

    WorldPosition = (ModelClone * vec4(position, 1.0)).xyz;
}