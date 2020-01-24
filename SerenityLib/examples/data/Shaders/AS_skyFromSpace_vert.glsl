USE_LOG_DEPTH_VERTEX

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 normal;
layout (location = 3) in vec4 binormal;
layout (location = 4) in vec4 tangent;
layout (location = 5) in vec4 BoneIDs;
layout (location = 6) in vec4 Weights;

uniform mat4 Model;
uniform int nSamples;

uniform vec4 VertDataMisc1;
uniform vec4 VertDataMisc2;
uniform vec4 VertDataMisc3;
uniform vec4 VertDataScale;
uniform vec4 VertDataRadius;
uniform vec4 VertDatafK;

varying vec3 c0;
varying vec3 c1;
varying vec3 v3Direction;
varying vec3 v3LightPosition;
varying vec3 WorldPosition;
varying float Depth;

float scale(float fCos){
    float x = 1.0 - fCos;
    return VertDataScale.y * exp(-0.00287 + x * (0.459 + x * (3.83 + x * (-6.80 + x * 5.25))));
}
float getNearIntersection(vec3 v3Pos, vec3 v3Ray, float fDistance2, float fRadius2){
    float B = 2.0 * dot(v3Pos, v3Ray);
    float C = fDistance2 - fRadius2;
    float fDet = max(0.0, B * B - 4.0 * C);
    return 0.5 * (-B - sqrt(fDet));
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

    float fNear = getNearIntersection(VertDataMisc1.xyz, v3Ray, VertDataMisc2.y,VertDataRadius.y);

    vec3 v3Start = VertDataMisc1.xyz + v3Ray * fNear;
    fFar -= fNear;

    float fStartAngle = dot(v3Ray, v3Start) / VertDataRadius.x;
    float fStartDepth = exp(-1.0 / VertDataScale.y);
    float fStartOffset = fStartDepth * scale(fStartAngle);
    Depth = clamp(fStartOffset,0.0,1.0);

    float fSampleLength = fFar / VertDataScale.w;
    float fScaledLength = fSampleLength * VertDataScale.x;
    vec3 v3SampleRay = v3Ray * fSampleLength;
    vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

    vec3 v3FrontColor = vec3(0);
    for(int i = 0; i < nSamples; ++i) {
        float fHeight = length(v3SamplePoint);
        float fDepth = exp(VertDataScale.z * (VertDataRadius.z - fHeight));
        float fLightAngle = dot(v3LightDir, v3SamplePoint) / fHeight;
        float fCameraAngle = 1.0f;
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

    WorldPosition = (ModelClone * vec4(position, 1.0)).xyz;
}