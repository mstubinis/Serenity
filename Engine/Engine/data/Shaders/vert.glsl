#version 120

attribute vec3 position;
attribute vec2 uv;
attribute vec3 normal;
attribute vec3 binormal;
attribute vec3 tangent;
attribute vec4 BoneIDs;
attribute vec4 Weights;

uniform mat4 VP;
uniform mat4 Model;
uniform mat3 NormalMatrix;
uniform int AnimationPlaying;
uniform mat4 gBones[200];

varying vec2 UV;

varying vec3 Normals;
varying vec3 Binormals;
varying vec3 Tangents;
varying vec3 WorldPosition;

varying float logz_f;
varying float FC_2_f;
uniform float fcoeff;

void main(void){
    mat4 BoneTransform = mat4(1.0);
    if(AnimationPlaying == 1.0){
        BoneTransform  = gBones[int(BoneIDs.x)] * Weights.x;
        BoneTransform += gBones[int(BoneIDs.y)] * Weights.y;
        BoneTransform += gBones[int(BoneIDs.z)] * Weights.z;
        BoneTransform += gBones[int(BoneIDs.w)] * Weights.w;
    }
    vec4 PosL = BoneTransform * vec4(position, 1.0);
    vec4 NormalL = BoneTransform * vec4(normal, 0.0);
    vec4 BinormalL = BoneTransform * vec4(binormal, 0.0);
    vec4 TangentL = BoneTransform * vec4(tangent, 0.0);

    mat4 MVP = VP * Model;

    gl_Position = MVP * PosL;
    
    Normals = (NormalMatrix * NormalL).xyz;
    Binormals = (NormalMatrix * BinormalL).xyz;
    Tangents = (NormalMatrix * TangentL).xyz;
    
    WorldPosition = (Model * PosL).xyz;

    UV = uv;
    logz_f = 1.0 + gl_Position.w;
    gl_Position.z = (log2(max(1e-6, logz_f)) * fcoeff - 1.0) * gl_Position.w;
    FC_2_f = fcoeff * 0.5;
}
