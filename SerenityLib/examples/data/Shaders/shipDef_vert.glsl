USE_LOG_DEPTH_VERTEX

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 normal;
layout (location = 3) in vec4 binormal;
layout (location = 4) in vec4 tangent;
layout (location = 5) in vec4 BoneIDs;
layout (location = 6) in vec4 Weights;

uniform mat4 Model;
uniform mat3 NormalMatrix;
uniform int AnimationPlaying;
uniform mat4 gBones[100];

varying vec2 UV;

varying vec3 Normals;
varying vec3 WorldPosition;
varying mat3 TBN;

flat varying vec3 CamPosition;
flat varying vec3 CamRealPosition;
varying vec3 TangentCameraPos;
varying vec3 TangentFragPos;

void main(){
    mat4 ModelMatrix = Model;
    ModelMatrix[3][0] -= CameraRealPosition.x;
    ModelMatrix[3][1] -= CameraRealPosition.y;
    ModelMatrix[3][2] -= CameraRealPosition.z;
    mat4 BoneTransform = mat4(1.0);
    if(AnimationPlaying == 1.0){
        BoneTransform  = gBones[int(BoneIDs.x)] * Weights.x;
        BoneTransform += gBones[int(BoneIDs.y)] * Weights.y;
        BoneTransform += gBones[int(BoneIDs.z)] * Weights.z;
        BoneTransform += gBones[int(BoneIDs.w)] * Weights.w;
    }
    vec4 PosTrans      =   BoneTransform * vec4(position,     1.0);
    vec3 NormalTrans   =  (BoneTransform * vec4(normal.zyx,   0.0)).xyz;
    vec3 BinormalTrans =  (BoneTransform * vec4(binormal.zyx, 0.0)).xyz;
    vec3 TangentTrans  =  (BoneTransform * vec4(tangent.zyx,  0.0)).xyz;

           Normals = NormalMatrix * NormalTrans;
    vec3 Binormals = NormalMatrix * BinormalTrans;
    vec3  Tangents = NormalMatrix * TangentTrans;
    TBN = mat3(Tangents,Binormals,Normals);

    vec4 worldPos = (ModelMatrix * PosTrans);

    gl_Position = CameraViewProj * worldPos;
    WorldPosition = worldPos.xyz;

    CamPosition = CameraPosition;
    CamRealPosition = CameraRealPosition;
    TangentCameraPos = TBN * CameraPosition;
    TangentFragPos = TBN * WorldPosition;

    UV = uv;
};