
#include <serenity/resources/Engine_BuiltInShaders.h>
#include <serenity/renderer/particles/ParticleSystem.h>
#include <serenity/resources/material/MaterialEnums.h>
#include <serenity/renderer/opengl/State.h>
#include <serenity/lights/Lights.h>

/*
GLSL Version    OpenGL Version
110              2.0
120              2.1
130              3.0
140              3.1
150              3.2
330              3.3
400              4.0
410              4.1
420              4.2
430              4.3
440              4.4
450              4.5
460              4.6
*/

#pragma region Declarations
std::string Engine::priv::EShaders::conditional_functions;
std::string Engine::priv::EShaders::decal_vertex;
std::string Engine::priv::EShaders::decal_frag;
std::string Engine::priv::EShaders::bullet_physics_vert;
std::string Engine::priv::EShaders::bullet_physcis_frag;
std::string Engine::priv::EShaders::fullscreen_quad_vertex;
std::string Engine::priv::EShaders::vertex_basic;
std::string Engine::priv::EShaders::vertex_2DAPI;
std::string Engine::priv::EShaders::vertex_skybox;
std::string Engine::priv::EShaders::particle_vertex;
std::string Engine::priv::EShaders::lighting_vert;
std::string Engine::priv::EShaders::stencil_passover;
std::string Engine::priv::EShaders::forward_frag;
std::string Engine::priv::EShaders::particle_frag;
std::string Engine::priv::EShaders::deferred_frag;
std::string Engine::priv::EShaders::zprepass_frag;
std::string Engine::priv::EShaders::deferred_frag_hud;
std::string Engine::priv::EShaders::deferred_frag_skybox;
std::string Engine::priv::EShaders::copy_depth_frag;
std::string Engine::priv::EShaders::cubemap_convolude_frag;
std::string Engine::priv::EShaders::cubemap_prefilter_envmap_frag;
std::string Engine::priv::EShaders::brdf_precompute;
std::string Engine::priv::EShaders::blur_frag;
std::string Engine::priv::EShaders::normaless_diffuse_frag;
std::string Engine::priv::EShaders::final_frag;
std::string Engine::priv::EShaders::depth_and_transparency_frag;
std::string Engine::priv::EShaders::lighting_frag;
std::string Engine::priv::EShaders::lighting_frag_gi;
std::string Engine::priv::EShaders::lighting_frag_basic;
std::string Engine::priv::EShaders::lighting_frag_gi_basic;
std::string Engine::priv::EShaders::shadow_depth_vert;
std::string Engine::priv::EShaders::shadow_depth_frag;
#pragma endregion

void Engine::priv::EShaders::init(){

#pragma region Functions

Engine::priv::EShaders::conditional_functions = R"(
vec4 when_eq(vec4 x, vec4 y) { return 1.0 - abs(sign(x - y)); }
vec4 when_neq(vec4 x, vec4 y) { return abs(sign(x - y)); }
vec4 when_gt(vec4 x, vec4 y) { return max(sign(x - y), 0.0); }
vec4 when_lt(vec4 x, vec4 y) { return max(sign(y - x), 0.0); }
vec4 when_ge(vec4 x, vec4 y) { return 1.0 - when_lt(x, y); }
vec4 when_le(vec4 x, vec4 y) { return 1.0 - when_gt(x, y); }
vec4 and(vec4 a, vec4 b) { return a * b; }
vec4 or(vec4 a, vec4 b) { return min(a + b, 1.0); }
//vec4 xor(vec4 a, vec4 b) { return (a + b) % 2.0; } //this is commented out
vec4 not(vec4 a) { return 1.0 - a; }

vec3 when_eq(vec3 x, vec3 y) { return 1.0 - abs(sign(x - y)); }
vec3 when_neq(vec3 x, vec3 y) { return abs(sign(x - y)); }
vec3 when_gt(vec3 x, vec3 y) { return max(sign(x - y), 0.0); }
vec3 when_lt(vec3 x, vec3 y) { return max(sign(y - x), 0.0); }
vec3 when_ge(vec3 x, vec3 y) { return 1.0 - when_lt(x, y); }
vec3 when_le(vec3 x, vec3 y) { return 1.0 - when_gt(x, y); }
vec3 and(vec3 a, vec3 b) { return a * b; }
vec3 or(vec3 a, vec3 b) { return min(a + b, 1.0); }
//vec3 xor(vec3 a, vec3 b) { return (a + b) % 2.0; } //this is commented out
vec3 not(vec3 a) { return 1.0 - a; }

vec2 when_eq(vec2 x, vec2 y) { return 1.0 - abs(sign(x - y)); }
vec2 when_neq(vec2 x, vec2 y) { return abs(sign(x - y)); }
vec2 when_gt(vec2 x, vec2 y) { return max(sign(x - y), 0.0); }
vec2 when_lt(vec2 x, vec2 y) { return max(sign(y - x), 0.0); }
vec2 when_ge(vec2 x, vec2 y) { return 1.0 - when_lt(x, y); }
vec2 when_le(vec2 x, vec2 y) { return 1.0 - when_gt(x, y); }
vec2 and(vec2 a, vec2 b) { return a * b; }
vec2 or(vec2 a, vec2 b) { return min(a + b, 1.0); }
//vec2 xor(vec2 a, vec2 b) { return (a + b) % 2.0; } //this is commented out
vec2 not(vec2 a) { return 1.0 - a; }

float when_eq(float x, float y) { return 1.0 - (abs(sign(x - y))); }
float when_neq(float x, float y) { return abs(sign(x - y)); }
float when_gt(float x, float y) { return max(sign(x - y), 0.0); }
float when_lt(float x, float y) { return max(sign(y - x), 0.0); }
float when_ge(float x, float y) { return 1.0 - when_lt(x, y); }
float when_le(float x, float y) { return 1.0 - when_gt(x, y); }
float and(float a, float b) { return a * b; }
float or(float a, float b) { return min(a + b, 1.0); }
//float xor(float a, float b) { return (a + b) % 2.0; } //this is commented out
float not(float a) { return 1.0 - a; }
int when_ieq(int x, int y) { return 1 - (abs(sign(x - y))); }
int when_ineq(int x, int y) { return abs(sign(x - y)); }
int when_igt(int x, int y) { return max(sign(x - y), 0); }
int when_ilt(int x, int y) { return max(sign(y - x), 0); }
int when_ige(int x, int y) { return 1 - when_ilt(x, y); }
int when_ile(int x, int y) { return 1 - when_igt(x, y); }
int iand(int a, int b) { return a * b; }
int ior(int a, int b) { return min(a + b, 1); }
int inot(int a) { return 1 - a; }
)";
#pragma endregion

#pragma region LightingVertex

// vec2 VertexShaderData; x = outercutoff, y = radius
// float Type; 2.0 = spot light. 1.0 = any other light. 0.0 = fullscreen quad / triangle

Engine::priv::EShaders::lighting_vert = R"(
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

uniform mat4 Model;
uniform mat4 VP;
uniform float Type;
uniform vec3 uSceneAmbientColor;

varying vec2 texcoords;
flat varying vec3 CamRealPosition;
flat varying vec3 SceneAmbientColor;

void main() {
    mat4 ModelClone = Model;
    vec3 ModelSpacePositions = position;
    if(Type == 2.0){
        ModelClone[3][0] -= CameraRealPosition.x;
        ModelClone[3][1] -= CameraRealPosition.y;
        ModelClone[3][2] -= CameraRealPosition.z;
    }else if(Type == 1.0){
        ModelClone[3][0] -= CameraRealPosition.x;
        ModelClone[3][1] -= CameraRealPosition.y;
        ModelClone[3][2] -= CameraRealPosition.z;
    }else if(Type == 0.0){
    }
    texcoords = uv;
    CamRealPosition = CameraRealPosition;
    SceneAmbientColor = uSceneAmbientColor;
    vec4 worldPosition = ModelClone * vec4(ModelSpacePositions, 1.0);
    gl_Position = (VP * worldPosition);
}
)";
#pragma endregion

#pragma region BulletPhysicsVertex
Engine::priv::EShaders::bullet_physics_vert = R"(
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

varying vec3 OutColor;

uniform mat4 Model;
uniform mat4 VP;

void main(){
    gl_Position = VP * Model * vec4(position, 1.0f);
    OutColor = color;
}
)";
#pragma endregion

#pragma region BulletPhysicsFragment
Engine::priv::EShaders::bullet_physcis_frag = R"(
in vec3 OutColor;
void main(){
    gl_FragColor = vec4(OutColor, 1.0);
}
)";
#pragma endregion

#pragma region DecalVertex
//TODO: get rid of the useless info here
Engine::priv::EShaders::decal_vertex = R"(
USE_LOG_DEPTH_VERTEX

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 normal;
layout (location = 3) in vec4 binormal;
layout (location = 4) in vec4 tangent;

uniform mat4 Model;
uniform mat3 NormalMatrix;

varying vec3 Normals;
varying mat3 TBN;

flat varying mat4 WorldMatrix;
flat varying mat4 InvWorldMatrix;
flat varying vec3 CamPosition;
flat varying vec3 CamRealPosition;

varying vec4 VertexPositionsClipSpace;
varying vec4 VertexPositionsViewSpace;
varying vec3 TangentCameraPos;
varying vec3 TangentFragPos;

void main(){
    mat4 ModelMatrix = Model;
    ModelMatrix[3][0] -= CameraRealPosition.x;
    ModelMatrix[3][1] -= CameraRealPosition.y;
    ModelMatrix[3][2] -= CameraRealPosition.z;
    WorldMatrix = ModelMatrix;
    InvWorldMatrix = inverse(ModelMatrix);
    vec4 worldPos = (ModelMatrix * vec4(position, 1.0));
    gl_Position = CameraViewProj * worldPos;
    VertexPositionsViewSpace = CameraView * worldPos;
    VertexPositionsClipSpace = gl_Position;

    vec3 NormalTrans   = vec4(normal.xyz,   0.0).xyz;
    vec3 BinormalTrans = vec4(binormal.xyz, 0.0).xyz;
    vec3 TangentTrans  = vec4(tangent.xyz,  0.0).xyz;

            Normals = NormalMatrix * NormalTrans;
    vec3 Binormals = NormalMatrix * BinormalTrans;
    vec3  Tangents = NormalMatrix * TangentTrans;
    TBN = mat3(Tangents,Binormals,Normals);

    CamPosition = CameraPosition;

    CamRealPosition  = CameraRealPosition;
    TangentCameraPos = TBN * CameraPosition;
    TangentFragPos   = TBN * worldPos.xyz;
}
)";
#pragma endregion

#pragma region ParticleVertex
Engine::priv::EShaders::particle_vertex =
    "USE_LOG_DEPTH_VERTEX\n"

    "layout (location = 0) in vec3 position;\n"
    "layout (location = 1) in vec2 uv;\n"

    "layout (location = 2) in vec4 ParticlePositionAndScaleX;\n"
    "layout (location = 3) in vec2 ParticleScaleYAndRotation;\n"
    "layout (location = 4) in uvec2 ParticleMaterialIndexAndColorPacked;\n"

    "varying vec2 UV;\n"
    "varying vec3 WorldPosition;\n"

    "flat varying uint MaterialIndex;\n"
    "flat varying vec4 ParticleColor;\n"

    "const vec3 ParticlePosition = ParticlePositionAndScaleX.xyz;\n"
    "const float ScaleX = ParticlePositionAndScaleX.w;\n"
    "const float ScaleY = ParticleScaleYAndRotation.x;\n"

    "void main(){\n"
    "    float sine = sin(ParticleScaleYAndRotation.y);\n"
    "    float cose = cos(ParticleScaleYAndRotation.y);\n"

    "    float xPrime = position.x * cose - position.y * sine;\n"
    "    float yPrime = position.x * sine + position.y * cose;\n"

    "    vec3 CameraRight = vec3(CameraView[0][0], CameraView[1][0], CameraView[2][0]);\n"
    "    vec3 CameraUp = vec3(CameraView[0][1], CameraView[1][1], CameraView[2][1]);\n"
    "    vec3 VertexWorldSpace = (ParticlePosition) + CameraRight * xPrime * ScaleX + CameraUp * yPrime * ScaleY;\n"

    "    vec4 worldPos = vec4(VertexWorldSpace, 1.0);\n"
    "    gl_Position = CameraViewProj * worldPos;\n"
    "    WorldPosition = worldPos.xyz;\n"
    "    UV = uv;\n"

    "    MaterialIndex = ParticleMaterialIndexAndColorPacked.x;\n"
#if defined(ENGINE_PARTICLES_HALF_SIZE)
    "    ParticleColor = Unpack16BitUIntTo4ColorFloats(ParticleMaterialIndexAndColorPacked.y);\n"
#else
    "    ParticleColor = Unpack32BitUIntTo4ColorFloats(ParticleMaterialIndexAndColorPacked.y);\n"
#endif
    "}";
#pragma endregion

#pragma region DecalFrag
//TODO: get rid of the useless info here
Engine::priv::EShaders::decal_frag =
"USE_LOG_DEPTH_FRAGMENT\n"
"USE_MAX_MATERIAL_LAYERS_PER_COMPONENT\n"
"USE_MAX_MATERIAL_COMPONENTS\n"
"\n"
"struct InData {\n"
"    vec4  diffuse;\n"
"    vec4  objectColor;\n"
"    vec3  normals;\n"
"    float glow;\n"
"    vec2  uv;\n"
"    float specular;\n"
"    float ao;\n"
"    vec3  worldPosition;\n"
"    float metalness;\n"
"    float smoothness;\n"
"};\n"
"struct Layer {\n"
"    vec4 d0;\n"
"    vec4 d1;\n"//x = blend mode | y = texture enabled? | z = mask enabled? | w = cubemap enabled?
"    vec4 d2;\n"
"    vec4 uvMods;\n"
"    SAMPLER_TYPE_2D texture;\n"
"    SAMPLER_TYPE_2D mask;\n"
"    SAMPLER_TYPE_Cube cubemap;\n"
"};\n"
"struct Component {\n"
"    ivec2 compDat;\n" //x = numLayers, y = componentType
"    Layer layers[MAX_MATERIAL_LAYERS_PER_COMPONENT];\n"
"};\n"
"uniform Component   components[MAX_MATERIAL_COMPONENTS];\n"
"uniform int         numComponents;\n"
"\n"
"\n"
"uniform vec4        MaterialBasePropertiesOne;\n"//x = BaseGlow, y = BaseAO, z = BaseMetalness, w = BaseSmoothness
"uniform vec4        MaterialBasePropertiesTwo;\n"//x = BaseAlpha, y = diffuseModel, z = specularModel, w = UNUSED
"\n"
"uniform int Shadeless;\n"
"\n"
"uniform uint Object_Color;\n"
"uniform vec4 Material_F0AndID;\n"
"uniform uint Gods_Rays_Color;\n"
"\n"
"uniform SAMPLER_TYPE_2D gDepthMap;\n"
"\n"
"varying vec3 Normals;\n"
"varying mat3 TBN;\n"
"flat varying mat4 WorldMatrix;\n"
"flat varying mat4 InvWorldMatrix;\n"
"flat varying vec3 CamPosition;\n"
"flat varying vec3 CamRealPosition;\n"
"varying vec3 TangentCameraPos;\n"
"varying vec3 TangentFragPos;\n"
"varying vec4 VertexPositionsClipSpace;\n"
"varying vec4 VertexPositionsViewSpace;\n"
"\n"
"void main(){\n"
"    vec2 screenPos = gl_FragCoord.xy / ScreenInfo.xy; \n"
"    vec3 WorldPosition = GetWorldPosition(USE_SAMPLER_2D(gDepthMap), screenPos, CameraNear, CameraFar);\n"
"    vec4 ObjectPosition = InvWorldMatrix * vec4(WorldPosition, 1.0);\n"
"    vec3 absData = abs(ObjectPosition.xyz);\n"
"    if ( absData.x > 0.5 || absData.y > 0.5 || absData.z > 0.5 || length(WorldPosition) > 500.0) {\n" //hacky way of eliminating against skybox
"        discard;\n"
"    }\n"
"    vec2 uvs = (ObjectPosition.xy + 0.5);\n"
//normal mapping...
/*
vec3 ddxWp = ddx(WorldPosition);
vec3 ddyWp = ddy(WorldPosition);
vec3 normal = normalize(cross(ddyWp, ddxWp));

//Normalizing things is cool
vec3 binormal = normalize(ddxWp);
vec3 tangent = normalize(ddyWp);

//Create a matrix transforming from tangent space to view space
mat3 tangentToView;
tangentToView[0] = CameraView * tangent;
tangentToView[1] = CameraView * binormal;
tangentToView[2] = CameraView * normal;

//Transform normal from tangent space into view space
normal = tangentToView * normal;
*/
"    InData inData;\n"
"    inData.uv = uvs;\n"
"    inData.diffuse = vec4(0.0, 0.0, 0.0, 0.0001);\n" //this is extremely wierd, but we need some form of alpha to get painters algorithm to work...
"    inData.objectColor = Unpack32BitUIntTo4ColorFloats(Object_Color);\n"
"    inData.normals = normalize(Normals);\n"
"    inData.glow = MaterialBasePropertiesOne.x;\n"
"    inData.specular = 1.0;\n"
"    inData.ao = MaterialBasePropertiesOne.y;\n"
"    inData.metalness = MaterialBasePropertiesOne.z;\n"
"    inData.smoothness = MaterialBasePropertiesOne.w;\n"
"    inData.worldPosition = WorldPosition;\n"
"\n"
"    ProcessComponentDiffuse(components[0], inData);\n"
"    ProcessComponentNormal(components[1], inData);\n"
"    ProcessComponentGlow(components[2], inData);\n"
"    ProcessComponentSpecular(components[3], inData);\n"
//"    ProcessComponentMetalness(components[4], inData);\n"
//"    ProcessComponentSmoothness(components[5], inData);\n"
//"    ProcessComponentAO(components[6], inData);\n"
//"    ProcessComponentReflection(components[7], inData);\n"
//"    ProcessComponentRefraction(components[8], inData);\n"
//"    ProcessComponentParallax(components[9], inData);\n"


"    if(Shadeless == 1){\n"
"        inData.normals = ConstantOneVec3;\n"
"        inData.diffuse *= (1.0 + inData.glow);\n" // we want shadeless items to be influenced by the glow somewhat...
"    }\n"
"    vec4 GodRays = Unpack32BitUIntTo4ColorFloats(Gods_Rays_Color);\n"
"    inData.diffuse.a *= MaterialBasePropertiesTwo.x;\n"

//"    inData.diffuse = (inData.diffuse * vec4(0.001)) + vec4(1.0, 0.0, 0.0, 1.0);\n"

"\n"
"    SUBMIT_DIFFUSE(inData.diffuse);\n"
//"    SUBMIT_NORMALS(1.0, 1.0);\n"
//"    SUBMIT_MATERIAL_ID_AND_AO(0.0, 0.0);\n"
//"    SUBMIT_METALNESS_AND_SMOOTHNESS(0.0);\n"
//"    SUBMIT_GLOW(0.0);\n"
//"    SUBMIT_SPECULAR(inData.specular);\n"
//"    SUBMIT_GOD_RAYS_COLOR(GodRays.r, GodRays.g, GodRays.b);\n"
"    gl_FragData[3] = inData.diffuse;\n"
"}";
#pragma endregion

#pragma region VertexBasic
Engine::priv::EShaders::vertex_basic = R"(
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
varying vec3 ViewPosition;
varying mat3 TBN;

flat varying vec3 CamPosition;
flat varying vec3 CamRealPosition;
varying vec3 TangentCameraPos;
varying vec3 TangentFragPos;

void main(){
    mat4 BoneTransform = mat4(1.0);
    if(AnimationPlaying == 1){
        BoneTransform  = gBones[int(BoneIDs[0])] * Weights[0];
        BoneTransform += gBones[int(BoneIDs[1])] * Weights[1];
        BoneTransform += gBones[int(BoneIDs[2])] * Weights[2];
        BoneTransform += gBones[int(BoneIDs[3])] * Weights[3];
    }
    vec4 PosTrans      =  (BoneTransform * vec4(position,     1.0));
    vec3 NormalTrans   =  (BoneTransform * vec4(normal.xyz,   0.0)).xyz;
    vec3 BinormalTrans =  (BoneTransform * vec4(binormal.xyz, 0.0)).xyz;
    vec3 TangentTrans  =  (BoneTransform * vec4(tangent.xyz,  0.0)).xyz;

         Normals       = (NormalMatrix * NormalTrans).xyz;
    vec3 Binormals     = (NormalMatrix * BinormalTrans).xyz;
    vec3 Tangents      = (NormalMatrix * TangentTrans).xyz;
    TBN = mat3(Tangents, Binormals, Normals);

    mat4 ModelMatrix   = Model;
    ModelMatrix[3][0] -= CameraRealPosition.x;
    ModelMatrix[3][1] -= CameraRealPosition.y;
    ModelMatrix[3][2] -= CameraRealPosition.z;

    vec4 worldPos      = ModelMatrix * PosTrans;
    WorldPosition      = worldPos.xyz;
    ViewPosition       = vec4(CameraView * worldPos).xyz;

    gl_Position        = CameraViewProj * worldPos;

    CamPosition        = CameraPosition;
    CamRealPosition    = CameraRealPosition;
    TangentCameraPos   = TBN * CameraPosition;
    TangentFragPos     = TBN * WorldPosition;

    UV = uv;
}
)";

#pragma endregion

#pragma region Vertex2DAPI
Engine::priv::EShaders::vertex_2DAPI = R"(
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec4 aColor;

uniform mat4 VP;
uniform mat4 Model;

varying vec2 vUV;
varying vec4 vColor;

void main() {
    vUV = aUV;
    vColor = aColor;
    gl_Position = VP * Model * vec4(aPosition, 1.0);
}
)";
#pragma endregion

#pragma region VertexSkybox
Engine::priv::EShaders::vertex_skybox = R"(
layout (location = 0) in vec3 position;
uniform mat4 VP;
varying vec3 UV;
void main(){
    UV = position;
    gl_Position = VP * vec4(position, 1.0);
    gl_Position.z = gl_Position.w;
}
)";
#pragma endregion

#pragma region CubemapConvoludeFrag
Engine::priv::EShaders::cubemap_convolude_frag = R"(
varying vec3 UV;
uniform SAMPLER_TYPE_Cube cubemap;
const float PI = 3.14159265;
const vec3 UP = vec3(0.0, 1.0, 0.0);
void main(){
    vec3 N = normalize(UV);
    vec3 irradiance = vec3(0.0);
    vec3 up = UP;
    vec3 right = cross(up, N);
    up = cross(N, right);
    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta){
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta){
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;
            irradiance += textureCube(cubemap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / nrSamples);
    gl_FragColor = vec4(irradiance, 1.0);
}
)";
#pragma endregion

#pragma region CubemapPrefilterEnvMap
//https://placeholderart.wordpress.com/2015/07/28/implementation-notes-runtime-environment-map-filtering-for-image-based-lighting/
//  ^
//  L_________ visit that link to optimize this process
//
// this shader is heavily modified based on optimizations in the link above. the optimizations are not complete yet, and 
// what seems to look correct may not be. this shader might have to be modified against the original later on.
//Data.x = roughness, Data.y = a2
Engine::priv::EShaders::cubemap_prefilter_envmap_frag = R"(
varying vec3 UV;

uniform SAMPLER_TYPE_Cube cubemap;
uniform vec2 Data;

uniform float PiFourDividedByResSquaredTimesSix;
uniform int NUM_SAMPLES;

const float PI = 3.14159265;
const float PI2 = 6.283185;
const float roughness = Data.x;
const float a2 = Data.y;

float DistributionGGX(vec3 Half){
    float NdotH2 = Half.z * Half.z;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return a2 / denom;
}
vec3 ImportanceSampleGGX(vec2 Xi, mat3 TBN){
    float phi = PI2 * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a2 - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
    vec3 Half;
    Half.x = sinTheta * cos(phi);
    Half.y = sinTheta * sin(phi);
    Half.z = cosTheta;
    return normalize(TBN * Half);
}
void main(){
    vec3 N = normalize(UV);
    vec3 R = N;
    vec3 V = R;
    vec3 PreEnv = vec3(0.0);
    float totalWeight = 0.0;
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
    mat3 TBN = mat3(tangent,bitangent,N);
    for(int i = 0; i < NUM_SAMPLES; ++i){
        vec2 Xi = HammersleySequence(i, NUM_SAMPLES);
        vec3 L = ImportanceSampleGGX(Xi,TBN);
        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0){
            float D   = DistributionGGX(L);
            float pdf = D * L.z / (4.0 * L.z) + 0.0001;
            float saSample = 1.0 / (float(NUM_SAMPLES) * pdf + 0.0001);
            float mipLevel = (roughness == 0.0) ? 0.0 : 0.5 * log2(saSample / PiFourDividedByResSquaredTimesSix);
            PreEnv += textureCubeLod(cubemap, L, mipLevel).rgb * NdotL;
            totalWeight += NdotL;
        }
    }
    PreEnv = PreEnv / totalWeight;
    gl_FragColor = vec4(PreEnv, 1.0);
}
)";
#pragma endregion

#pragma region BRDFPrecompute
Engine::priv::EShaders::brdf_precompute = R"(
const float PI2 = 6.283185;
uniform int NUM_SAMPLES;
varying vec2 texcoords;

vec3 ImportanceSampleGGX(vec2 Xi, float a2, mat3 TBN){
    float phi = PI2 * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a2 - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    vec3 Half;
    Half.x = cos(phi) * sinTheta;
    Half.y = sin(phi) * sinTheta;
    Half.z = cosTheta;
    return normalize(TBN * Half);
}
float GeometrySchlickGGX(float NdotV, float a){
    float k = a / 2.0;
    float denom = NdotV * (1.0 - k) + k;
    return NdotV / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float a){
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, a);
    float ggx1 = GeometrySchlickGGX(NdotL, a);
    return ggx1 * ggx2;
}
vec2 IntegrateBRDF(float NdotV, float roughness){
    vec3 V;
    V.x = sqrt(1.0 - NdotV * NdotV);
    V.y = 0.0;
    V.z = NdotV;
    float A = 0.0;
    float B = 0.0;
    vec3 N = vec3(0.0, 0.0, 1.0);
    float a = roughness * roughness;
    float a2 = a * a;
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
    mat3 TBN = mat3(tangent,bitangent,N);
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        vec2 Xi = HammersleySequence(i, NUM_SAMPLES);
        vec3 H = ImportanceSampleGGX(Xi, a2, TBN);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);
        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);
        if (NdotL > 0.0) {
            float G = GeometrySmith(N, V, L, a);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);
            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= float(NUM_SAMPLES);
    B /= float(NUM_SAMPLES);
    return vec2(A, B);
}
void main(){
    gl_FragColor.rg = IntegrateBRDF(texcoords.x, texcoords.y);
}
)";
#pragma endregion

#pragma region StencilPass
Engine::priv::EShaders::stencil_passover = R"(
const vec3 comparison = vec3(1.0, 1.0, 1.0);
uniform SAMPLER_TYPE_2D gNormalMap;
varying vec2 texcoords;
void main(){
    vec3 normal = DecodeOctahedron(texture2D(gNormalMap, texcoords).rg);
    if(distance(normal, comparison) < 0.01){
        discard;
    }
}
)";
#pragma endregion

#pragma region FullscreenQuadVertex
Engine::priv::EShaders::fullscreen_quad_vertex = R"(
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

uniform mat4 Model;
uniform mat4 VP;

varying vec2 texcoords;

void main(){
    vec3 vert = position;
    texcoords = uv;
    gl_Position = VP * Model * vec4(vert, 1.0);
}
)";
#pragma endregion
   
#pragma region ForwardFrag
Engine::priv::EShaders::forward_frag =
    "USE_LOG_DEPTH_FRAGMENT\n"
    "USE_MAX_MATERIAL_LAYERS_PER_COMPONENT\n"
    "USE_MAX_MATERIAL_COMPONENTS\n"
    "USE_MAX_LIGHTS_PER_PASS\n"
    "\n"
    "struct InData {\n"
    "    vec4  diffuse;\n"
    "    vec4  objectColor;\n"
    "    vec3  normals;\n"
    "    float glow;\n"
    "    vec2  uv;\n"
    "    float specular;\n"
    "    float ao;\n"
    "    vec3  materialF0;\n"
    "    float metalness;\n"
    "    vec3  worldPosition;\n"
    "    float smoothness;\n"
    "};\n"
    "struct Layer {\n"
    "    vec4 d0;\n"
    "    vec4 d1;\n"//x = blend mode | y = texture enabled? | z = mask enabled? | w = cubemap enabled?
    "    vec4 d2;\n"
    "    vec4 uvMods;\n"
    "    SAMPLER_TYPE_2D texture;\n"
    "    SAMPLER_TYPE_2D mask;\n"
    "    SAMPLER_TYPE_Cube cubemap;\n"
    "};\n"
    "struct Component {\n"
    "    ivec2 compDat;\n" //x = numLayers, y = componentType
    "    Layer layers[MAX_MATERIAL_LAYERS_PER_COMPONENT];\n"
    "};\n"
    "\n"
    "struct Light{\n"
    "    vec4 DataA;\n" //x = ambient, y = diffuse, z = specular, w = LightDirection.x
    "    vec4 DataB;\n" //x = LightDirection.y, y = LightDirection.z, z = const, w = linear
    "    vec4 DataC;\n" //x = exp, y = LightPosition.x, z = LightPosition.y, w = LightPosition.z
    "    vec4 DataD;\n" //x = LightColor.r, y = LightColor.g, z = LightColor.b, w = LightType
    "    vec4 DataE;\n" //x = cutoff, y = outerCutoff, z = AttenuationModel, w = UNUSED
    "};\n"
    "\n"
    "uniform Light       light[MAX_LIGHTS_PER_PASS];\n"
    "uniform int         numLights;\n"
    "\n"
    "uniform Component   components[MAX_MATERIAL_COMPONENTS];\n"
    "uniform int         numComponents;\n"
    "\n"
    "uniform vec4        MaterialBasePropertiesOne;\n"//x = BaseGlow, y = BaseAO, z = BaseMetalness, w = BaseSmoothness
    "uniform vec4        MaterialBasePropertiesTwo;\n"//x = BaseAlpha, y = diffuseModel, z = specularModel, w = UNUSED
    "\n"
    "uniform int Shadeless;\n"
    "\n"
    "uniform uint Object_Color;\n"
    "uniform vec4 Material_F0AndID;\n"
    "uniform uint Gods_Rays_Color;\n"
    "\n"
    "varying vec3 WorldPosition;\n"
    "varying vec3 ViewPosition;\n"
    "varying vec2 UV;\n"
    "varying vec3 Normals;\n"
    "varying mat3 TBN;\n"
    "flat varying vec3 CamPosition;\n"
    "flat varying vec3 CamRealPosition;\n"
    "varying vec3 TangentCameraPos;\n"
    "varying vec4 FragPosLightSpace;\n"
    "varying vec3 TangentFragPos;\n"
    "\n"
    "uniform SAMPLER_TYPE_2D gDepthMap;\n"
    "uniform SAMPLER_TYPE_Cube irradianceMap;\n"
    "uniform SAMPLER_TYPE_Cube prefilterMap;\n"
    "uniform SAMPLER_TYPE_2D brdfLUT;\n"
    "uniform SAMPLER_TYPE_2D gTextureMap;\n"

    "uniform mat4 uLightMatrix[NUM_CASCADES];\n"

    "\n"
    "const float MAX_REFLECTION_LOD = 5.0;\n"
    "\n"
    "vec3 SchlickFrenselRoughness(float _theta, vec3 _F0, float _roughness){\n"
    "    vec3 ret = _F0 + (max(vec3(1.0 - _roughness),_F0) - _F0) * pow(1.0 - _theta,5.0);\n"
    "    return ret;\n"
    "}\n"
    "void main(){\n"
    "    InData inData;\n"
    "    inData.uv = UV;\n"
    "    inData.diffuse = vec4(0.0,0.0,0.0,0.0001);\n" //this is extremely wierd, but we need some form of alpha to get painters algorithm to work...
    "    inData.objectColor = Unpack32BitUIntTo4ColorFloats(Object_Color);\n"
    "    inData.normals = normalize(Normals);\n"
    "    inData.glow = MaterialBasePropertiesOne.x;\n"
    "    inData.specular = 1.0;\n"
    "    inData.ao = MaterialBasePropertiesOne.y;\n"
    "    inData.metalness = MaterialBasePropertiesOne.z;\n"
    "    inData.smoothness = MaterialBasePropertiesOne.w;\n"
    "    inData.materialF0 = Material_F0AndID.rgb;\n"
    "    inData.worldPosition = WorldPosition;\n"

    "    ProcessComponentDiffuse(components[0], inData);\n"
    "    ProcessComponentNormal(components[1], inData);\n"
    "    ProcessComponentGlow(components[2], inData);\n"
    "    ProcessComponentSpecular(components[3], inData);\n"
    "    ProcessComponentMetalness(components[4], inData);\n"
    "    ProcessComponentSmoothness(components[5], inData);\n"
    "    ProcessComponentAO(components[6], inData);\n"
    "    ProcessComponentReflection(components[7], inData);\n"
    "    ProcessComponentRefraction(components[8], inData);\n"
    "    ProcessComponentParallax(components[9], inData);\n"

    "    vec3 lightTotal = ConstantZeroVec3;\n"
    "    if(Shadeless != 1){\n"
    "        vec3 lightCalculation = ConstantZeroVec3;\n"
    "        float SSAO            = 1.0;\n"
    "        vec2 MetalSmooth      = vec2(inData.metalness, inData.smoothness);\n"
    "        float AO              = inData.ao + 0.0001;\n"//the 0.0001 makes up for the clamp in material class
    "        for (int j = 0; j < numLights; ++j) {\n"
    "            Light currentLight    = light[j];\n"
    "            vec3 lightCalculation = ConstantZeroVec3;\n"
    "            vec3 LightPosition    = vec3(currentLight.DataC.yzw) - CamRealPosition;\n"
    "            vec3 LightDirection   = normalize(vec3(currentLight.DataA.w, currentLight.DataB.x, currentLight.DataB.y));\n"
    "            if(currentLight.DataD.w == 0.0){\n"       //sun
    "                lightCalculation = CalcLightInternal(currentLight, normalize(LightPosition - WorldPosition), WorldPosition, ViewPosition, inData.normals, inData.specular, inData.diffuse.rgb, SSAO, MetalSmooth, MaterialBasePropertiesTwo.x, inData.materialF0, MaterialBasePropertiesTwo.y, MaterialBasePropertiesTwo.z, AO);\n"
    "            }else if(currentLight.DataD.w == 1.0){\n" //point
    "                lightCalculation = CalcPointLight(currentLight, LightPosition, WorldPosition, ViewPosition, inData.normals, inData.specular, inData.diffuse.rgb, SSAO, MetalSmooth, MaterialBasePropertiesTwo.x, inData.materialF0, MaterialBasePropertiesTwo.y, MaterialBasePropertiesTwo.z, AO);\n"
    "            }else if(currentLight.DataD.w == 2.0){\n" //directional
    "                lightCalculation = CalcLightInternal(currentLight, LightDirection, WorldPosition, ViewPosition, inData.normals, inData.specular, inData.diffuse.rgb, SSAO, MetalSmooth, MaterialBasePropertiesTwo.x, inData.materialF0, MaterialBasePropertiesTwo.y, MaterialBasePropertiesTwo.z, AO);\n"
    "            }else if(currentLight.DataD.w == 3.0){\n" //spot
    "                lightCalculation = CalcSpotLight(currentLight, LightDirection, LightPosition, WorldPosition, ViewPosition, inData.normals, inData.specular, inData.diffuse.rgb, SSAO, MetalSmooth, MaterialBasePropertiesTwo.x, inData.materialF0, MaterialBasePropertiesTwo.y, MaterialBasePropertiesTwo.z, AO);\n"
    "            }else if(currentLight.DataD.w == 4.0){\n" //rod
    "                lightCalculation = CalcRodLight(currentLight, vec3(currentLight.DataA.w, currentLight.DataB.xy), currentLight.DataC.yzw, WorldPosition, ViewPosition, inData.normals, inData.specular, inData.diffuse.rgb, SSAO, MetalSmooth, MaterialBasePropertiesTwo.x, inData.materialF0, MaterialBasePropertiesTwo.y, MaterialBasePropertiesTwo.z, AO);\n"
    "            }else if(currentLight.DataD.w == 5.0){\n" //projection
    "                lightCalculation = CalcProjectionLight(currentLight, vec3(currentLight.DataA.w, currentLight.DataB.xy), currentLight.DataC.yzw, WorldPosition, ViewPosition, inData.normals, inData.specular, inData.diffuse.rgb, SSAO, MetalSmooth, MaterialBasePropertiesTwo.x, inData.materialF0, MaterialBasePropertiesTwo.y, MaterialBasePropertiesTwo.z, AO);\n"
    "            }\n"
    "            lightTotal += lightCalculation;\n"
    "        }\n"
             //GI here
    "        vec3 inGIContribution = Unpack3FloatsInto1FloatUnsigned(RendererInfo1.x);\n" //x = diffuse, y = specular, z = global
    "        lightTotal           += CalcGILight(SSAO, inData.normals, inData.diffuse.xyz, WorldPosition, AO, MetalSmooth.x, MetalSmooth.y, inData.glow, inData.materialF0, MaterialBasePropertiesTwo.x, inGIContribution).rgb;\n"
    "    }else{\n"
    "        lightTotal = inData.diffuse.rgb;\n"
    "    }\n"
    "    inData.diffuse.a *= MaterialBasePropertiesTwo.x;\n"
    "    vec4 GodRays = Unpack32BitUIntTo4ColorFloats(Gods_Rays_Color);\n"
    "    SUBMIT_DIFFUSE(vec4(lightTotal.rgb, inData.diffuse.a));\n"
    "    SUBMIT_NORMALS(1.0, 1.0);\n"
    "    SUBMIT_MATERIAL_ID_AND_AO(0.0, 0.0);\n"
    //"    SUBMIT_METALNESS_AND_SMOOTHNESS(0.0);\n"
    "    SUBMIT_GLOW(0.0);\n"
    //"    SUBMIT_SPECULAR(inData.specular);\n"
    //"    SUBMIT_GOD_RAYS_COLOR(GodRays.r, GodRays.g, GodRays.b);\n"
    "    gl_FragData[3] = vec4(lightTotal.rgb, inData.diffuse.a);\n"
    "}";
#pragma endregion

#pragma region ParticleFrag
Engine::priv::EShaders::particle_frag =
"\n"
"USE_LOG_DEPTH_FRAGMENT\n"

"uniform SAMPLER_TYPE_2D DiffuseTexture0;\n";

for (uint32_t i = 1; i < std::min(priv::OpenGLState::constants.MAX_TEXTURE_IMAGE_UNITS - 1U, MAX_UNIQUE_PARTICLE_TEXTURES_PER_FRAME); ++i) {
    priv::EShaders::particle_frag +=
        "uniform SAMPLER_TYPE_2D DiffuseTexture" + std::to_string(i) + ";\n";
}

priv::EShaders::particle_frag +=

"uniform SAMPLER_TYPE_2D gDepthMap;\n"

"varying vec2 UV;\n"
"varying vec3 WorldPosition;\n"

"flat varying uint MaterialIndex;\n"
"flat varying vec4 ParticleColor;\n"

"void main(){\n"
//this code is for soft particles////////////////////////////////////
"    vec2 screen_uv = gl_FragCoord.xy / vec2(ScreenInfo.x, ScreenInfo.y);\n"
"    vec3 worldPos = GetWorldPosition(USE_SAMPLER_2D(gDepthMap), screen_uv, CameraNear, CameraFar);\n"
"    float dist = distance(worldPos, WorldPosition) * 4.2;\n" //increasing that number will make the particles fade less from edges, but might increase the risk for sharper edges like without soft particles
"    float alpha = clamp(dist, 0.0, 1.0);\n"
//////////////////////////////////////////////////////////////////////////
"    vec4 finalColor = ParticleColor;\n"
"    if(MaterialIndex == 0U)\n"
"        finalColor *= texture2D(DiffuseTexture0, UV); \n";
for (uint32_t i = 1; i < std::min(priv::OpenGLState::constants.MAX_TEXTURE_IMAGE_UNITS - 1U, MAX_UNIQUE_PARTICLE_TEXTURES_PER_FRAME); ++i) {
    priv::EShaders::particle_frag +=
"    else if (MaterialIndex == " + std::to_string(i) + "U)\n"
"        finalColor *= texture2D(DiffuseTexture" + std::to_string(i) + ", UV); \n";
}
priv::EShaders::particle_frag +=
"    else\n"
//"        finalColor *= texture2D(DiffuseTexture0, UV); \n"
"        finalColor = vec4(1.0, 0.0, 0.0, 1.0); \n"

"    finalColor.a *= alpha;\n"

"    SUBMIT_DIFFUSE(finalColor);\n"
"    gl_FragData[3] = finalColor;\n"
"}";
#pragma endregion

#pragma region DeferredFrag

//ivec2 compDat; //x = numLayers, y = componentType
//vec4 d1; //x = blend mode | y = texture enabled? | z = mask enabled? | w = cubemap enabled?
//uniform vec4        MaterialBasePropertiesOne; //x = BaseGlow, y = BaseAO, z = BaseMetalness, w = BaseSmoothness
//uniform vec4        MaterialBasePropertiesTwo; //x = BaseAlpha, y = diffuseModel, z = specularModel, w = UNUSED
//inData.diffuse = vec4(0.0,0.0,0.0,0.0001); //this is extremely wierd, but we need some form of alpha to get painters algorithm to work...
//inData.diffuse *= (1.0 + inData.glow); // we want shadeless items to be influenced by the glow somewhat...

Engine::priv::EShaders::deferred_frag = R"(
USE_LOG_DEPTH_FRAGMENT
USE_MAX_MATERIAL_LAYERS_PER_COMPONENT
USE_MAX_MATERIAL_COMPONENTS

struct InData {
    vec4  diffuse;
    vec4  objectColor;
    vec3  normals;
    float glow;
    vec2  uv;
    float specular;
    float ao;
    vec3  worldPosition;
    float metalness;
    float smoothness;
};
struct Layer {
    vec4 d0;
    vec4 d1;
    vec4 d2;
    vec4 uvMods;
    SAMPLER_TYPE_2D texture;
    SAMPLER_TYPE_2D mask;
    SAMPLER_TYPE_Cube cubemap;
};
struct Component {
    ivec2 compDat;
    Layer layers[MAX_MATERIAL_LAYERS_PER_COMPONENT];
};

uniform Component   components[MAX_MATERIAL_COMPONENTS];
uniform int         numComponents;

uniform vec4        MaterialBasePropertiesOne;
uniform vec4        MaterialBasePropertiesTwo;

uniform int Shadeless;

uniform uint Object_Color;
uniform vec4 Material_F0AndID;
uniform uint Gods_Rays_Color;

varying vec3 WorldPosition;
varying vec2 UV;
varying vec3 Normals;
varying mat3 TBN;
flat varying vec3 CamPosition;
varying vec3 TangentCameraPos;
varying vec3 TangentFragPos;

void main(){
    InData inData;
    inData.uv = UV;
    inData.diffuse = vec4(0.0, 0.0, 0.0, 0.0001);
    inData.objectColor = Unpack32BitUIntTo4ColorFloats(Object_Color);
    inData.normals = normalize(Normals);
    inData.glow = MaterialBasePropertiesOne.x;
    inData.specular = 1.0;
    inData.ao = MaterialBasePropertiesOne.y;
    inData.metalness = MaterialBasePropertiesOne.z;
    inData.smoothness = MaterialBasePropertiesOne.w;
    inData.worldPosition = WorldPosition;

    ProcessComponentDiffuse(components[0], inData);
    ProcessComponentNormal(components[1], inData);
    ProcessComponentGlow(components[2], inData);
    ProcessComponentSpecular(components[3], inData);
    ProcessComponentMetalness(components[4], inData);
    ProcessComponentSmoothness(components[5], inData);
    ProcessComponentAO(components[6], inData);
    ProcessComponentReflection(components[7], inData);
    ProcessComponentRefraction(components[8], inData);
    ProcessComponentParallax(components[9], inData);

    vec2 OutNormals = EncodeOctahedron(inData.normals);
    if(Shadeless == 1){
        OutNormals = ConstantOneVec2;
        inData.diffuse *= (1.0 + inData.glow);
    }
    vec4 GodRays = Unpack32BitUIntTo4ColorFloats(Gods_Rays_Color);
    inData.diffuse.a *= MaterialBasePropertiesTwo.x;

    SUBMIT_DIFFUSE(inData.diffuse);
    gl_FragData[1] = vec4(OutNormals, Material_F0AndID.w + inData.ao, inData.glow);
    gl_FragData[2] = vec4(Pack2NibblesInto8BitChannel(inData.metalness, inData.smoothness), inData.specular, Pack2NibblesInto8BitChannel(GodRays.r, GodRays.g), GodRays.b);
}
)";
#pragma endregion

#pragma region NormalessDiffuseFrag
Engine::priv::EShaders::normaless_diffuse_frag = R"(
const vec3 comparison = vec3(1.0, 1.0, 1.0);
uniform SAMPLER_TYPE_2D gNormalMap;
uniform SAMPLER_TYPE_2D gDiffuseMap;
uniform int HasLighting;
varying vec2 texcoords;
void main(){
    vec3 normal   = DecodeOctahedron(texture2D(gNormalMap, texcoords).rg);
    vec3 diffuse  = texture2D(USE_SAMPLER_2D(gDiffuseMap), texcoords).rgb;
    if(HasLighting == 0 || distance(normal, comparison) < 0.01){
        gl_FragColor = vec4(diffuse, 1.0);
    }else{
        discard;
    }
}
)";
#pragma endregion

#pragma region ZPrepassFrag
Engine::priv::EShaders::zprepass_frag = R"(
USE_LOG_DEPTH_FRAGMENT
//USE_MAX_MATERIAL_LAYERS_PER_COMPONENT
//USE_MAX_MATERIAL_COMPONENTS
void main(){
}
)";
#pragma endregion

#pragma region DeferredFragHUD
Engine::priv::EShaders::deferred_frag_hud = R"(
uniform SAMPLER_TYPE_2D DiffuseTexture;
uniform int DiffuseTextureEnabled;
uniform vec4 Object_Color;

varying vec2 vUV;
varying vec4 vColor;

void main() {
    gl_FragColor = Object_Color;
    if(DiffuseTextureEnabled == 1){
        gl_FragColor *= texture2D(DiffuseTexture, vUV);
    }
    gl_FragColor *= vColor;
    gl_FragColor.rgb = pow(gl_FragColor.rgb, vec3(1.0 / RendererInfo1.y));
}
)";
#pragma endregion

#pragma region DeferredFragSkybox
Engine::priv::EShaders::deferred_frag_skybox = R"(
uniform vec4 Color;
uniform int IsFake;
uniform SAMPLER_TYPE_Cube Texture;
varying vec3 UV;
varying vec3 WorldPosition;
void main(){
    if(IsFake == 1){
        SUBMIT_DIFFUSE(Color);
    }else{
        SUBMIT_DIFFUSE(textureCube(Texture, UV));
    }
    gl_FragData[0].rgb = pow(gl_FragData[0].rgb, vec3(1.0 / RendererInfo1.y));
    gl_FragData[1].rg = vec2(1.0);
    gl_FragData[2]    = vec4(0.0);
}
)";
#pragma endregion

#pragma region CopyDepthFrag
Engine::priv::EShaders::copy_depth_frag = R"(
uniform SAMPLER_TYPE_2D gDepthMap;
varying vec2 texcoords;
void main(){
    gl_FragDepth = texture2D(gDepthMap, texcoords).r;
}
)";
#pragma endregion

#pragma region Blur
Engine::priv::EShaders::blur_frag = R"(
uniform SAMPLER_TYPE_2D image;

uniform vec4 DataA; //radius, UNUSED, H,V
uniform vec4 strengthModifier;

varying vec2 texcoords;

const int NUM_SAMPLES = 9;
const float weight[NUM_SAMPLES] = float[](0.227, 0.21, 0.1946, 0.162, 0.12, 0.08, 0.054, 0.03, 0.016);

void main(){
    vec4 Sum = vec4(0.0);
    vec2 inverseResolution = vec2(1.0) / ScreenInfo.zw;
    for(int i = 0; i < NUM_SAMPLES; ++i){
        vec2 offset = (inverseResolution * float(i)) * DataA.x;
        Sum += (texture2D(image,texcoords + vec2(offset.x * DataA.z, offset.y * DataA.w)) * weight[i]) * strengthModifier;
        Sum += (texture2D(image,texcoords - vec2(offset.x * DataA.z, offset.y * DataA.w)) * weight[i]) * strengthModifier;
    }
    gl_FragColor = Sum;
}
)";
#pragma endregion

#pragma region Greyscale
/*
Engine::priv::EShaders::greyscale_frag = R"(
uniform SAMPLER_TYPE_2D textureMap;
varying vec2 texcoords;
void main(){
    vec4 col = texture2D(textureMap, texcoords);
    float lum = dot(col.rgb, vec3(0.299, 0.587, 0.114));
    gl_FragColor = vec4(vec3(lum), 1.0);
}
)";
*/
#pragma endregion
    
#pragma region FinalFrag
Engine::priv::EShaders::final_frag = R"(
uniform SAMPLER_TYPE_2D SceneTexture;
uniform SAMPLER_TYPE_2D gBloomMap;
uniform SAMPLER_TYPE_2D gDepthMap;
uniform SAMPLER_TYPE_2D gDiffuseMap;

uniform int HasBloom;
uniform int HasFog;

uniform vec4 FogColor;
uniform float FogDistNull;
uniform float FogDistBlend;

varying vec2 texcoords;

void main(){
    vec4 scene = texture2D(SceneTexture, texcoords);
    vec4 diffuse = texture2D(gDiffuseMap, texcoords);
    if (HasBloom == 1) {
        vec4 bloom = texture2D(gBloomMap, texcoords);
        scene += bloom;
    }
    gl_FragColor = scene;
    if (HasFog == 1) {
        float distFrag = distance(GetWorldPosition(USE_SAMPLER_2D(gDepthMap), texcoords, CameraNear, CameraFar), CameraPosition);
        float distVoid = FogDistNull + FogDistBlend;
        float distBlendIn = FogDistBlend - (distVoid - distFrag);
        float omega = smoothstep(0.0, 1.0, (distBlendIn / FogDistBlend));
        vec4 fc = FogColor * clamp(omega, 0.0, 1.0);
        gl_FragColor = PaintersAlgorithm(fc, gl_FragColor);
    }
}
)";
#pragma endregion

#pragma region DepthAndTransparency
Engine::priv::EShaders::depth_and_transparency_frag =
    "\n"
    "uniform SAMPLER_TYPE_2D SceneTexture;\n"
    "uniform SAMPLER_TYPE_2D gDepthMap;\n"
    "\n"
    //"uniform vec4 TransparencyMaskColor;\n"
    //"uniform int TransparencyMaskActive;\n"
    "uniform float DepthMaskValue;\n"
    "uniform int DepthMaskActive;\n"
    "\n"
    "varying vec2 texcoords;\n"
    "\n";
Engine::priv::EShaders::depth_and_transparency_frag +=
    "\n"
    "void main(){\n"
    "    vec4 scene = texture2D(SceneTexture, texcoords);\n"
    "    scene.a = 1.0;\n"
    "    float depth = distance(GetWorldPosition(USE_SAMPLER_2D(gDepthMap), texcoords, CameraNear, CameraFar), CameraPosition);\n"
    //"    if (TransparencyMaskActive == 1 && scene.rgb == TransparencyMaskColor.rgb) {\n"
    //"        scene.a = 0.0;\n"
    //"    }\n"
    "    if (DepthMaskActive == 1 && depth > DepthMaskValue) {\n"
    //"        scene.a = 0.0;\n" //"erase" the pixel if the depth exceeds what the viewport wants
    "        discard;\n"
    "    }\n"
    "    gl_FragColor = scene;\n"
    //"    gl_FragDepth = 0.000001;\n"
    "}";
#pragma endregion

#pragma region LightingFrag

//struct Light {
//    vec4 DataA;   //x = ambient,          y = diffuse,          z = specular,         w = LightDirection.x
//    vec4 DataB;   //x = LightDirection.y, y = LightDirection.z, z = const,            w = linear
//    vec4 DataC;   //x = exp,              y = LightPosition.x,  z = LightPosition.y,  w = LightPosition.z
//    vec4 DataD;   //x = LightColor.r,     y = LightColor.g,     z = LightColor.b,     w = LightType
//    vec4 DataE;   //x = cutoff,           y = outerCutoff,      z = AttenuationModel, w = UNUSED
//};
//vec4 materials[MATERIAL_COUNT_LIMIT]; //r = MaterialF0Color (packed into float), g = baseSmoothness, b = specularModel, a = diffuseModel
//
//if     (light.DataD.w == 0.0){  //sun
//else if(light.DataD.w == 1.0){  //point
//else if(light.DataD.w == 2.0){  //directional
//else if(light.DataD.w == 3.0){  //spot
//else if(light.DataD.w == 4.0){  //rod
//else if(light.DataD.w == 5.0){  //projection

//vec3 PxlViewPosition  = CameraView * vec4(PxlPosition.xyz, 1.0); TODO: pass this into lighting functions for shadows

Engine::priv::EShaders::lighting_frag = R"(
#define MATERIAL_COUNT_LIMIT 255

struct Light {
    vec4 DataA;
    vec4 DataB;
    vec4 DataC;
    vec4 DataD;
    vec4 DataE;
};
uniform Light light;

uniform SAMPLER_TYPE_2D gDiffuseMap;
uniform SAMPLER_TYPE_2D gNormalMap;
uniform SAMPLER_TYPE_2D gMiscMap;
uniform SAMPLER_TYPE_2D gDepthMap;
uniform SAMPLER_TYPE_2D gSSAOMap;
uniform SAMPLER_TYPE_2D gTextureMap;

uniform mat4 uLightMatrix[NUM_CASCADES];

uniform vec4 materials[MATERIAL_COUNT_LIMIT];

varying vec2 texcoords;
flat varying vec3 CamRealPosition;
flat varying vec3 SceneAmbientColor;

void main(){
    vec2 uv = gl_FragCoord.xy / ScreenInfo.xy;
    vec3 PxlNormal        = DecodeOctahedron(texture2D(USE_SAMPLER_2D(gNormalMap), uv).rg);
    vec3 PxlPosition      = GetWorldPosition(USE_SAMPLER_2D(gDepthMap), uv, CameraNear, CameraFar);
    vec3 PxlViewPosition  = (CameraView * vec4(PxlPosition, 1.0)).xyz;
    vec3 lightCalculation = ConstantZeroVec3;
    vec3 LightPosition    = vec3(light.DataC.yzw) - CamRealPosition;
    vec3 LightDirection   = normalize(vec3(light.DataA.w, light.DataB.x, light.DataB.y));
    float Specular        = texture2D(USE_SAMPLER_2D(gMiscMap), uv).g;
    vec3 Albedo           = texture2D(USE_SAMPLER_2D(gDiffuseMap), uv).rgb;
    float MatIDAndAO      = texture2D(USE_SAMPLER_2D(gNormalMap), uv).b;
    float SSAO            = 1.0 - texture2D(gSSAOMap, uv).a;
    vec2 MetalSmooth      = Unpack2NibblesFrom8BitChannel(texture2D(USE_SAMPLER_2D(gMiscMap), uv).r);
    highp int matID       = int(floor(MatIDAndAO));
    float MatAlpha        = materials[matID].g;
    vec3 MatF0            = Unpack3FloatsInto1FloatUnsigned(materials[matID].r);
    float MatTypeDiffuse  = materials[matID].a;
    float MatTypeSpecular = materials[matID].b;
    float AO              = fract(MatIDAndAO) + 0.0001;
    if (light.DataD.w == 0.0) {
        lightCalculation = CalcLightInternal(light, normalize(LightPosition - PxlPosition), PxlPosition, PxlViewPosition, PxlNormal, Specular, Albedo, SSAO, MetalSmooth, MatAlpha, MatF0, MatTypeDiffuse, MatTypeSpecular, AO);
    } else if (light.DataD.w == 1.0) {
        lightCalculation = CalcPointLight(light, LightPosition, PxlPosition, PxlViewPosition, PxlNormal, Specular, Albedo, SSAO, MetalSmooth, MatAlpha, MatF0, MatTypeDiffuse, MatTypeSpecular, AO);
    } else if (light.DataD.w == 2.0) {
        lightCalculation = CalcLightInternal(light, LightDirection, PxlPosition, PxlViewPosition, PxlNormal, Specular, Albedo, SSAO, MetalSmooth, MatAlpha, MatF0, MatTypeDiffuse, MatTypeSpecular, AO);
    } else if (light.DataD.w == 3.0) {
        lightCalculation = CalcSpotLight(light, LightDirection, LightPosition, PxlPosition, PxlViewPosition, PxlNormal, Specular, Albedo, SSAO, MetalSmooth, MatAlpha, MatF0, MatTypeDiffuse, MatTypeSpecular, AO);
    } else if (light.DataD.w == 4.0) {
        lightCalculation = CalcRodLight(light, vec3(light.DataA.w, light.DataB.xy), light.DataC.yzw, PxlPosition, PxlViewPosition, PxlNormal, Specular, Albedo, SSAO, MetalSmooth, MatAlpha, MatF0, MatTypeDiffuse, MatTypeSpecular, AO);
    } else if (light.DataD.w == 5.0) {
        lightCalculation = CalcProjectionLight(light, vec3(light.DataA.w, light.DataB.xy), light.DataC.yzw, PxlPosition, PxlViewPosition, PxlNormal, Specular, Albedo, SSAO, MetalSmooth, MatAlpha, MatF0, MatTypeDiffuse, MatTypeSpecular, AO);
    }
    gl_FragData[0].rgb = lightCalculation;
}
)";
#pragma endregion

#pragma region LightingFragGI
//vec4 materials[MATERIAL_COUNT_LIMIT];  //r = MaterialF0Color (packed into float), g = baseSmoothness, b = specularModel, a = diffuseModel
Engine::priv::EShaders::lighting_frag_gi = R"(
#define MATERIAL_COUNT_LIMIT 255

const float MAX_REFLECTION_LOD = 5.0;

uniform SAMPLER_TYPE_2D gDiffuseMap;
uniform SAMPLER_TYPE_2D gNormalMap;
uniform SAMPLER_TYPE_2D gDepthMap;
uniform SAMPLER_TYPE_2D gSSAOMap;
uniform SAMPLER_TYPE_2D gMiscMap;
uniform SAMPLER_TYPE_2D brdfLUT;

uniform SAMPLER_TYPE_Cube irradianceMap;
uniform SAMPLER_TYPE_Cube prefilterMap;

uniform vec4 materials[MATERIAL_COUNT_LIMIT];

varying vec2 texcoords;
flat varying vec3 CamRealPosition;

vec3 SchlickFrenselRoughness(float inTheta, vec3 inF0, float inRoughness) {
    return inF0 + (max(vec3(1.0 - inRoughness), inF0) - inF0) * pow(1.0 - inTheta, 5.0);
}
void main() {
    vec2 uv               = gl_FragCoord.xy / ScreenInfo.xy;

    float inSSAO          = 1.0 - texture2D(USE_SAMPLER_2D(gSSAOMap), uv).a;
    vec3 inNormals        = DecodeOctahedron(texture2D(USE_SAMPLER_2D(gNormalMap), uv).rg);
    vec3 inAlbedo         = texture2D(USE_SAMPLER_2D(gDiffuseMap), uv).rgb;
    vec3 inWorldPosition  = GetWorldPosition(USE_SAMPLER_2D(gDepthMap), uv, CameraNear, CameraFar);
    float inGlow          = texture2D(USE_SAMPLER_2D(gNormalMap), uv).a;
    vec2 inMetalSmooth    = Unpack2NibblesFrom8BitChannel(texture2D(USE_SAMPLER_2D(gMiscMap), uv).r);
    float MatIDAndAO      = texture2D(USE_SAMPLER_2D(gNormalMap), uv).b;

    highp int index       = int(floor(MatIDAndAO));
    float inAO            = fract(MatIDAndAO) + 0.0001;
    vec3 inMatF0          = Unpack3FloatsInto1FloatUnsigned(materials[index].r);
    vec3 inGIContribution = Unpack3FloatsInto1FloatUnsigned(RendererInfo1.x);
    vec4 GILight          = CalcGILight(inSSAO, inNormals, inAlbedo, inWorldPosition, inAO, inMetalSmooth.x, inMetalSmooth.y, inGlow, inMatF0, materials[index].g, inGIContribution);
    gl_FragColor.rgb = max(GILight.rgb, inGlow * inAlbedo);
}
)";
#pragma endregion

#pragma region LightingFragBasic
Engine::priv::EShaders::lighting_frag_basic = R"(
#define MATERIAL_COUNT_LIMIT 255

struct Light {
    vec4 DataA;
    vec4 DataB;
    vec4 DataC;
    vec4 DataD;
    vec4 DataE;
};
uniform Light light;

uniform SAMPLER_TYPE_2D gDiffuseMap;
uniform SAMPLER_TYPE_2D gNormalMap;
uniform SAMPLER_TYPE_2D gMiscMap;
uniform SAMPLER_TYPE_2D gDepthMap;
uniform SAMPLER_TYPE_2D gSSAOMap;
uniform SAMPLER_TYPE_2D gTextureMap;

uniform mat4 uLightMatrix[NUM_CASCADES];

uniform vec4 materials[MATERIAL_COUNT_LIMIT];

varying vec2 texcoords;
flat varying vec3 CamRealPosition;

void main(){
    vec2 uv = gl_FragCoord.xy / ScreenInfo.xy;
    vec3 PxlNormal        = DecodeOctahedron(texture2D(USE_SAMPLER_2D(gNormalMap), uv).rg);
    vec3 PxlPosition      = GetWorldPosition(USE_SAMPLER_2D(gDepthMap), uv, CameraNear, CameraFar);
    vec3 PxlViewPosition  = (CameraView * vec4(PxlPosition, 1.0)).xyz;
    vec3 LightPosition    = vec3(light.DataC.yzw) - CamRealPosition;
    vec3 LightDirection   = normalize(vec3(light.DataA.w, light.DataB.x, light.DataB.y));
    float Specular        = texture2D(USE_SAMPLER_2D(gMiscMap), uv).g;
    vec3 Albedo           = texture2D(USE_SAMPLER_2D(gDiffuseMap), uv).rgb;
    float MatIDAndAO      = texture2D(USE_SAMPLER_2D(gNormalMap), uv).b;
    float SSAO            = 1.0 - texture2D(gSSAOMap, uv).a;
    highp int matID       = int(floor(MatIDAndAO));
    float MatAlpha        = materials[matID].g;
    float AO              = fract(MatIDAndAO) + 0.0001;
    vec3 LightCalculation = ConstantZeroVec3;
    if(light.DataD.w == 0.0){
        LightCalculation = CalcLightInternalBasic(light, normalize(LightPosition - PxlPosition), PxlPosition, PxlViewPosition, PxlNormal, Specular, Albedo, SSAO, MatAlpha, AO);
    }else if(light.DataD.w == 1.0){
        LightCalculation = CalcPointLightBasic(light, LightPosition, PxlPosition, PxlViewPosition, PxlNormal, Specular, Albedo, SSAO, MatAlpha, AO);
    }else if(light.DataD.w == 2.0){
        LightCalculation = CalcLightInternalBasic(light, LightDirection, PxlPosition, PxlViewPosition, PxlNormal, Specular, Albedo, SSAO, MatAlpha, AO);
    }else if(light.DataD.w == 3.0){
        LightCalculation = CalcSpotLightBasic(light, LightDirection, LightPosition, PxlPosition, PxlViewPosition, PxlNormal, Specular, Albedo, SSAO, MatAlpha, AO);
    }else if(light.DataD.w == 4.0){
        LightCalculation = CalcRodLightBasic(light, vec3(light.DataA.w, light.DataB.xy), light.DataC.yzw, PxlPosition, PxlViewPosition, PxlNormal, Specular, Albedo, SSAO, MatAlpha, AO);
    }else if(light.DataD.w == 5.0){
        LightCalculation = CalcProjectionLightBasic(light, vec3(light.DataA.w, light.DataB.xy), light.DataC.yzw, PxlPosition, PxlViewPosition, PxlNormal, Specular, Albedo, SSAO, MatAlpha, AO);
    }
    gl_FragData[0].rgb = LightCalculation;
}
)";
#pragma endregion

#pragma region LightingFragGIBasic
//gl_FragColor.rgb = inAlbedo * inGlow; frambuffer blending is enabled, so color is not overwritten just blended on
Engine::priv::EShaders::lighting_frag_gi_basic = R"(
#define MATERIAL_COUNT_LIMIT 255

uniform SAMPLER_TYPE_2D gDiffuseMap;
uniform SAMPLER_TYPE_2D gNormalMap;

varying vec2 texcoords;
flat varying vec3 CamRealPosition;

void main(){
    vec4 PrevCanvas  = gl_FragColor;
    vec2 uv          = gl_FragCoord.xy / ScreenInfo.xy;
    vec3 inAlbedo    = texture2D(USE_SAMPLER_2D(gDiffuseMap), uv).rgb;
    float inGlow     = texture2D(USE_SAMPLER_2D(gNormalMap), uv).a;
    gl_FragColor.rgb = inAlbedo * inGlow;
}
)";
#pragma endregion

//TODO: move this code to the Shadows.cpp file?
#pragma region ShadowMapDepth

Engine::priv::EShaders::shadow_depth_vert = R"(
layout(location = 0) in vec3 position;

uniform mat4 uLightMatrix;
uniform mat4 Model;

void main() {
    gl_Position = uLightMatrix * Model * vec4(position, 1.0);
}  
)";

/*
//gl_FragDepth += gl_FrontFacing ? BIAS : 0.0;
const float BIAS = 0.002;
void main() {
    gl_FragDepth  = gl_FragCoord.z;
    gl_FragDepth += float(gl_FrontFacing) * BIAS;
}
*/
Engine::priv::EShaders::shadow_depth_frag = R"(
void main(){}
)";
#pragma endregion
}