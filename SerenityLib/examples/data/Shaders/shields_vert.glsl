USE_LOG_DEPTH_VERTEX
  
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 normal;

uniform mat4 Model;
uniform mat3 NormalMatrix;

varying vec2 UV;

varying vec3 Normals;
varying vec3 WorldPosition;
varying mat3 TBN;

flat varying vec3 CamPosition;
flat varying vec3 CamRealPosition;
varying vec3 TangentCameraPos;
varying vec3 TangentFragPos;

uniform float uv_scale;

void main(){
    mat4 ModelMatrix = Model;
    ModelMatrix[3][0] -= CameraRealPosition.x;
    ModelMatrix[3][1] -= CameraRealPosition.y;
    ModelMatrix[3][2] -= CameraRealPosition.z;

    vec4 PosTrans      =  vec4(position,     1.0);
    vec3 NormalTrans   =  vec4(normal.zyx,   0.0).xyz;

            Normals = NormalMatrix * NormalTrans;
    TBN = mat3(Normals,Normals,Normals);

    vec4 worldPos = (ModelMatrix * PosTrans);

    gl_Position = CameraViewProj * worldPos;
    WorldPosition = worldPos.xyz;

    CamPosition = CameraPosition;
    CamRealPosition = CameraRealPosition;
    TangentCameraPos = TBN * CameraPosition;
    TangentFragPos = TBN * WorldPosition;

    UV = uv * uv_scale;
}