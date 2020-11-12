#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/resources/Engine_BuiltInShaders.h>
#include <core/engine/renderer/particles/ParticleSystem.h>
#include <core/engine/materials/MaterialEnums.h>
#include <core/engine/renderer/opengl/State.h>

using namespace Engine;

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
std::string priv::EShaders::conditional_functions;
std::string priv::EShaders::decal_vertex;
std::string priv::EShaders::decal_frag;
std::string priv::EShaders::bullet_physics_vert;
std::string priv::EShaders::bullet_physcis_frag;
std::string priv::EShaders::fullscreen_quad_vertex;
std::string priv::EShaders::vertex_basic;
std::string priv::EShaders::vertex_2DAPI;
std::string priv::EShaders::vertex_skybox;
std::string priv::EShaders::particle_vertex;
std::string priv::EShaders::lighting_vert;
std::string priv::EShaders::stencil_passover;
std::string priv::EShaders::forward_frag;
std::string priv::EShaders::particle_frag;
std::string priv::EShaders::deferred_frag;
std::string priv::EShaders::zprepass_frag;
std::string priv::EShaders::deferred_frag_hud;
std::string priv::EShaders::deferred_frag_skybox;
std::string priv::EShaders::copy_depth_frag;
std::string priv::EShaders::cubemap_convolude_frag;
std::string priv::EShaders::cubemap_prefilter_envmap_frag;
std::string priv::EShaders::brdf_precompute;
std::string priv::EShaders::blur_frag;
std::string priv::EShaders::normaless_diffuse_frag;
std::string priv::EShaders::final_frag;
std::string priv::EShaders::depth_and_transparency_frag;
std::string priv::EShaders::lighting_frag;
std::string priv::EShaders::lighting_frag_gi;
#pragma endregion

void priv::EShaders::init(const uint32_t openglVersion, const uint32_t glslVersion){

#pragma region Functions

priv::EShaders::conditional_functions =
    "\n"
    "vec4 when_eq(vec4 x, vec4 y) { return 1.0 - abs(sign(x - y)); }\n"
    "vec4 when_neq(vec4 x, vec4 y) { return abs(sign(x - y)); }\n"
    "vec4 when_gt(vec4 x, vec4 y) { return max(sign(x - y), 0.0); }\n"
    "vec4 when_lt(vec4 x, vec4 y) { return max(sign(y - x), 0.0); }\n"
    "vec4 when_ge(vec4 x, vec4 y) { return 1.0 - when_lt(x, y); }\n"
    "vec4 when_le(vec4 x, vec4 y) { return 1.0 - when_gt(x, y); }\n"
    "vec4 and(vec4 a, vec4 b) { return a * b; }\n"
    "vec4 or(vec4 a, vec4 b) { return min(a + b, 1.0); }\n"
    //"vec4 xor(vec4 a, vec4 b) { return (a + b) % 2.0; }\n"//this is commented out
    "vec4 not(vec4 a) { return 1.0 - a; }\n"
    "\n"
    "vec3 when_eq(vec3 x, vec3 y) { return 1.0 - abs(sign(x - y)); }\n"
    "vec3 when_neq(vec3 x, vec3 y) { return abs(sign(x - y)); }\n"
    "vec3 when_gt(vec3 x, vec3 y) { return max(sign(x - y), 0.0); }\n"
    "vec3 when_lt(vec3 x, vec3 y) { return max(sign(y - x), 0.0); }\n"
    "vec3 when_ge(vec3 x, vec3 y) { return 1.0 - when_lt(x, y); }\n"
    "vec3 when_le(vec3 x, vec3 y) { return 1.0 - when_gt(x, y); }\n"
    "vec3 and(vec3 a, vec3 b) { return a * b; }\n"
    "vec3 or(vec3 a, vec3 b) { return min(a + b, 1.0); }\n"
    //"vec3 xor(vec3 a, vec3 b) { return (a + b) % 2.0; }\n"//this is commented out
    "vec3 not(vec3 a) { return 1.0 - a; }\n"
    "\n"
    "vec2 when_eq(vec2 x, vec2 y) { return 1.0 - abs(sign(x - y)); }\n"
    "vec2 when_neq(vec2 x, vec2 y) { return abs(sign(x - y)); }\n"
    "vec2 when_gt(vec2 x, vec2 y) { return max(sign(x - y), 0.0); }\n"
    "vec2 when_lt(vec2 x, vec2 y) { return max(sign(y - x), 0.0); }\n"
    "vec2 when_ge(vec2 x, vec2 y) { return 1.0 - when_lt(x, y); }\n"
    "vec2 when_le(vec2 x, vec2 y) { return 1.0 - when_gt(x, y); }\n"
    "vec2 and(vec2 a, vec2 b) { return a * b; }\n"
    "vec2 or(vec2 a, vec2 b) { return min(a + b, 1.0); }\n"
    //"vec2 xor(vec2 a, vec2 b) { return (a + b) % 2.0; }\n"//this is commented out
    "vec2 not(vec2 a) { return 1.0 - a; }\n"
    "\n"

    "float when_eq(float x, float y) { return 1.0 - (abs(sign(x - y))); }\n"
    "float when_neq(float x, float y) { return abs(sign(x - y)); }\n"
    "float when_gt(float x, float y) { return max(sign(x - y), 0.0); }\n"
    "float when_lt(float x, float y) { return max(sign(y - x), 0.0); }\n"
    "float when_ge(float x, float y) { return 1.0 - when_lt(x, y); }\n"
    "float when_le(float x, float y) { return 1.0 - when_gt(x, y); }\n"
    "float and(float a, float b) { return a * b; }\n"
    "float or(float a, float b) { return min(a + b, 1.0); }\n"
    //"float xor(float a, float b) { return (a + b) % 2.0; }\n"//this is commented out
    "float not(float a) { return 1.0 - a; }\n"
    "\n"
    "int when_ieq(int x, int y) { return 1 - (abs(sign(x - y))); }\n"
    "int when_ineq(int x, int y) { return abs(sign(x - y)); }\n"
    "int when_igt(int x, int y) { return max(sign(x - y), 0); }\n"
    "int when_ilt(int x, int y) { return max(sign(y - x), 0); }\n"
    "int when_ige(int x, int y) { return 1 - when_ilt(x, y); }\n"
    "int when_ile(int x, int y) { return 1 - when_igt(x, y); }\n"
    "int iand(int a, int b) { return a * b; }\n"
    "int ior(int a, int b) { return min(a + b, 1); }\n"
    "int inot(int a) { return 1 - a; }\n"
    "\n";

#pragma endregion

#pragma region LightingVertex
priv::EShaders::lighting_vert = 
    "layout (location = 0) in vec3 position;\n"
    "layout (location = 1) in vec2 uv;\n"
    "\n"
    "uniform mat4 Model;\n"
    "uniform mat4 VP;\n"
    "uniform vec2 VertexShaderData;\n" //x = outercutoff, y = radius
    "uniform float Type;\n" // 2.0 = spot light. 1.0 = any other light. 0.0 = fullscreen quad / triangle
    "\n"
    "varying vec2 texcoords;\n"
    "flat varying vec3 CamRealPosition;\n"
    "\n"
    "vec3 doSpotLightStuff(vec3 inPositions){\n"
    "    float opposite = tan(VertexShaderData.x * 0.5) * VertexShaderData.y;\n" //outerCutoff might need to be in degrees?
    "    inPositions.xy *= vec2(opposite / VertexShaderData.y);\n" //might need to switch around x,y,z to fit GL's coordinate system
    "    return inPositions;\n"
    "}\n"
    "void main(){\n"
    "    mat4 ModelClone = Model;\n"
    "    vec3 ModelSpacePositions = position;\n"
    "    if(Type == 2.0){\n" //spot light
    "        ModelSpacePositions = doSpotLightStuff(ModelSpacePositions);\n"
    "        ModelClone[3][0] -= CameraRealPosition.x;\n"
    "        ModelClone[3][1] -= CameraRealPosition.y;\n"
    "        ModelClone[3][2] -= CameraRealPosition.z;\n"
    "    }else if(Type == 1.0){\n" //point / rod / etc
    "        ModelClone[3][0] -= CameraRealPosition.x;\n"
    "        ModelClone[3][1] -= CameraRealPosition.y;\n"
    "        ModelClone[3][2] -= CameraRealPosition.z;\n"
    "    }else if(Type == 0.0){\n" //fullscreen quad / triangle
    "    }\n"
    "    texcoords = uv;\n"
    "    CamRealPosition = CameraRealPosition;\n"
    "    gl_Position = (VP * ModelClone) * vec4(ModelSpacePositions, 1.0);\n"
    "}";
#pragma endregion

#pragma region BulletPhysicsVertex
priv::EShaders::bullet_physics_vert =
    "layout (location = 0) in vec3 position;\n"
    "layout (location = 1) in vec3 color;\n"
    "\n"
    "varying vec3 OutColor;\n"
    "\n"
    "uniform mat4 Model;\n"
    "uniform mat4 VP;\n"
    "\n"
    "void main(){\n"
    "	gl_Position = VP * Model * vec4(position, 1.0f);\n"
    "	OutColor = color;\n"
    "}";
#pragma endregion

#pragma region BulletPhysicsFragment
priv::EShaders::bullet_physcis_frag =
    "in vec3 OutColor;\n"
    "void main(){\n"
    "	gl_FragColor = vec4(OutColor,1.0);\n"
    "}";
#pragma endregion

#pragma region DecalVertex
//TODO: get rid of the useless info here
priv::EShaders::decal_vertex =  
    "USE_LOG_DEPTH_VERTEX\n"

    "layout (location = 0) in vec3 position;\n"
    "layout (location = 1) in vec2 uv;\n"
    "layout (location = 2) in vec4 normal;\n" //Order is ZYXW
    "layout (location = 3) in vec4 binormal;\n"//Order is ZYXW
    "layout (location = 4) in vec4 tangent;\n"//Order is ZYXW

    "uniform mat4 Model;\n"
    "uniform mat3 NormalMatrix;\n"

    "varying vec3 Normals;\n"
    "varying mat3 TBN;\n"

    "flat varying mat4 WorldMatrix;\n"
    "flat varying vec3 CamPosition;\n"
    "flat varying vec3 CamRealPosition;\n"

    "varying vec4 VertexPositionsClipSpace;\n"
    "varying vec4 VertexPositionsViewSpace;\n"
    "varying vec3 TangentCameraPos;\n"
    "varying vec3 TangentFragPos;\n"

    "void main(){\n"
    "    mat4 ModelMatrix = Model;\n"
    "    ModelMatrix[3][0] -= CameraRealPosition.x;\n"
    "    ModelMatrix[3][1] -= CameraRealPosition.y;\n"
    "    ModelMatrix[3][2] -= CameraRealPosition.z;\n"
    "    WorldMatrix = ModelMatrix;\n"

    "    vec4 worldPos = (ModelMatrix * vec4(position, 1.0));\n"
    "    gl_Position = CameraViewProj * worldPos;\n"
    "    VertexPositionsViewSpace = CameraView * worldPos;\n"
    "    VertexPositionsClipSpace = gl_Position;\n"

    "    vec3 NormalTrans   = vec4(normal.zyx,   0.0).xyz;\n"  //Order is ZYXW so to bring it to XYZ we need to use ZYX
    "    vec3 BinormalTrans = vec4(binormal.zyx, 0.0).xyz;\n"//Order is ZYXW so to bring it to XYZ we need to use ZYX
    "    vec3 TangentTrans  = vec4(tangent.zyx,  0.0).xyz;\n" //Order is ZYXW so to bring it to XYZ we need to use ZYX

    "           Normals = NormalMatrix * NormalTrans;\n"
    "    vec3 Binormals = NormalMatrix * BinormalTrans;\n"
    "    vec3  Tangents = NormalMatrix * TangentTrans;\n"
    "    TBN = mat3(Tangents,Binormals,Normals);\n"

    "    CamPosition = CameraPosition;\n"

    "    CamRealPosition  = CameraRealPosition;\n"
    "    TangentCameraPos = TBN * CameraPosition;\n"
    "    TangentFragPos   = TBN * worldPos.xyz;\n"
    "}";
#pragma endregion

#pragma region ParticleVertex
priv::EShaders::particle_vertex =
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

    "void main(){\n"
    "    float sine = sin(ParticleScaleYAndRotation.y);\n"
    "    float cose = cos(ParticleScaleYAndRotation.y);\n"

    "    float xPrime = position.x * cose - position.y * sine;\n"
    "    float yPrime = position.x * sine + position.y * cose;\n"

    "    vec3 CameraRight = vec3(CameraView[0][0], CameraView[1][0], CameraView[2][0]);\n"
    "    vec3 CameraUp = vec3(CameraView[0][1], CameraView[1][1], CameraView[2][1]);\n"
    "    vec3 VertexWorldSpace = (ParticlePositionAndScaleX.xyz) + CameraRight * (xPrime) * ParticlePositionAndScaleX.w + CameraUp * (yPrime) * ParticleScaleYAndRotation.x;\n"

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
priv::EShaders::decal_frag =
    "USE_LOG_DEPTH_FRAGMENT\n"
    "USE_MAX_MATERIAL_LAYERS_PER_COMPONENT\n"
    "USE_MAX_MATERIAL_COMPONENTS\n"
    "\n"
    "struct InData {\n"
    "    vec2  uv;\n"
    "    vec4  diffuse;\n"
    "    vec4  objectColor;\n"
    "    vec3  normals;\n"
    "    float glow;\n"
    "    float specular;\n"
    "    float ao;\n"
    "    float metalness;\n"
    "    float smoothness;\n"
    "    vec3  worldPosition;\n"
    "};\n"
    "struct Layer {\n"
    "    vec4 data1;\n"//x = blend mode | y = texture enabled? | z = mask enabled? | w = cubemap enabled?
    "    vec4 data2;\n"
    "    SAMPLER_TYPE_2D texture;\n"
    "    SAMPLER_TYPE_2D mask;\n"
    "    SAMPLER_TYPE_Cube cubemap;\n"
    "    vec4 uvModifications;\n"
    "};\n"
    "struct Component {\n"
    "    ivec2 componentData;\n" //x = numLayers, y = componentType
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
    "flat varying vec3 CamPosition;\n"
    "flat varying vec3 CamRealPosition;\n"
    "varying vec3 TangentCameraPos;\n"
    "varying vec3 TangentFragPos;\n"
    "varying vec4 VertexPositionsClipSpace;\n"
    "varying vec4 VertexPositionsViewSpace;\n"
    "\n"
    "void main(){\n"
    "    vec2 screenPos = gl_FragCoord.xy / vec2(ScreenInfo.x, ScreenInfo.y); \n"
    "    vec3 WorldPosition = GetWorldPosition(USE_SAMPLER_2D(gDepthMap), screenPos, CameraNear, CameraFar);\n"
    "    vec4 ObjectPosition = inverse(WorldMatrix) * vec4(WorldPosition, 1.0);\n"
    "    float x = 1.0 - abs(ObjectPosition.x);\n"
    "    float y = 1.0 - abs(ObjectPosition.y);\n"
    "    float z = 1.0 - abs(ObjectPosition.z);\n"
    "    if ( x < 0.0 || y < 0.0 || z < 0.0 || length(WorldPosition) > 500.0) {\n" //hacky way of eliminating against skybox
    "        discard;\n"
    "    }\n"
    "    vec2 uvs = (ObjectPosition.xy + 1.0) * 0.5;\n"
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
    "    for (int j = 0; j < numComponents; ++j) {\n"
    "        ProcessComponent(components[j], inData);\n"
    "    }\n"
    "    if(Shadeless == 1){\n"
    "        inData.normals = ConstantOneVec3;\n"
    "        inData.diffuse *= (1.0 + inData.glow);\n" // we want shadeless items to be influenced by the glow somewhat...
    "    }\n"
    "    vec4 GodRays = Unpack32BitUIntTo4ColorFloats(Gods_Rays_Color);\n"
    "    inData.diffuse.a *= MaterialBasePropertiesTwo.x;\n"
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
priv::EShaders::vertex_basic = 
    "USE_LOG_DEPTH_VERTEX\n"
    "\n"
    "layout (location = 0) in vec3 position;\n"
    "layout (location = 1) in vec2 uv;\n"
    "layout (location = 2) in vec4 normal;\n" //Order is ZYXW
    "layout (location = 3) in vec4 binormal;\n"//Order is ZYXW
    "layout (location = 4) in vec4 tangent;\n"//Order is ZYXW
    "layout (location = 5) in vec4 BoneIDs;\n"
    "layout (location = 6) in vec4 Weights;\n"
    "\n"
    "uniform mat4 Model;\n"
    "uniform mat3 NormalMatrix;\n"
    "uniform int AnimationPlaying;\n"
    "uniform mat4 gBones[100];\n"
    "\n"
    "varying vec2 UV;\n"
    "\n"
    "varying vec3 Normals;\n"
    "varying vec3 WorldPosition;\n"
    "varying mat3 TBN;\n"
    "\n"
    "flat varying vec3 CamPosition;\n"
    "flat varying vec3 CamRealPosition;\n"
    "varying vec3 TangentCameraPos;\n"
    "varying vec3 TangentFragPos;\n"
    "\n"
    "void main(){\n"
    "    mat4 ModelMatrix = Model;\n"
    "    ModelMatrix[3][0] -= CameraRealPosition.x;\n"
    "    ModelMatrix[3][1] -= CameraRealPosition.y;\n"
    "    ModelMatrix[3][2] -= CameraRealPosition.z;\n"
    "    mat4 BoneTransform = mat4(1.0);\n"
    "    if(AnimationPlaying == 1.0){\n"
    "        BoneTransform  = gBones[int(BoneIDs.x)] * Weights.x;\n"
    "        BoneTransform += gBones[int(BoneIDs.y)] * Weights.y;\n"
    "        BoneTransform += gBones[int(BoneIDs.z)] * Weights.z;\n"
    "        BoneTransform += gBones[int(BoneIDs.w)] * Weights.w;\n"
    "    }\n"
    "    vec4 PosTrans      =   BoneTransform * vec4(position,     1.0);\n"
    "    vec3 NormalTrans   =  (BoneTransform * vec4(normal.zyx,   0.0)).xyz;\n"  //Order is ZYXW so to bring it to XYZ we need to use ZYX
    "    vec3 BinormalTrans =  (BoneTransform * vec4(binormal.zyx, 0.0)).xyz;\n"//Order is ZYXW so to bring it to XYZ we need to use ZYX
    "    vec3 TangentTrans  =  (BoneTransform * vec4(tangent.zyx,  0.0)).xyz;\n" //Order is ZYXW so to bring it to XYZ we need to use ZYX
    "\n"
    "           Normals = NormalMatrix * NormalTrans;\n"
    "    vec3 Binormals = NormalMatrix * BinormalTrans;\n"
    "    vec3  Tangents = NormalMatrix * TangentTrans;\n"
    "    TBN = mat3(Tangents,Binormals,Normals);\n"
    "\n"
    "    vec4 worldPos = (ModelMatrix * PosTrans);\n"
    "\n"
    "    gl_Position = CameraViewProj * worldPos;\n"
    "    WorldPosition = worldPos.xyz;\n"
    "\n"
    "    CamPosition = CameraPosition;\n"
    "    CamRealPosition = CameraRealPosition;\n"
    "    TangentCameraPos = TBN * CameraPosition;\n"
    "    TangentFragPos = TBN * WorldPosition;\n"
    "\n"
    "    UV = uv;\n"
    "}";
#pragma endregion

#pragma region Vertex2DAPI
priv::EShaders::vertex_2DAPI =
    "layout (location = 0) in vec3 position;\n"
    "layout (location = 1) in vec2 uv;\n"
    "\n"
    "uniform mat4 VP;\n"
    "uniform mat4 Model;\n"
    "varying vec2 UV;\n"
    "void main(){\n"
    "    UV = uv;\n"
    "    gl_Position = VP * Model * vec4(position, 1.0);\n"
    "}";
#pragma endregion

#pragma region VertexSkybox
priv::EShaders::vertex_skybox =
    "\n"
    "layout (location = 0) in vec3 position;\n"
    "uniform mat4 VP;\n"
    "varying vec3 UV;\n"
    "void main(){\n"
    "    UV = position;\n"
    "    gl_Position = VP * vec4(position, 1.0);\n"
    "    gl_Position.z = gl_Position.w;\n"
    "}";
#pragma endregion

#pragma region CubemapConvoludeFrag
priv::EShaders::cubemap_convolude_frag =
    "\n"
    "varying vec3 UV;\n"
    "uniform SAMPLER_TYPE_Cube cubemap;\n"
    "const float PI = 3.14159265;\n"
    "const vec3 UP = vec3(0.0, 1.0, 0.0);\n"

    "void main(){\n"
    "    vec3 N = normalize(UV);\n"
    "    vec3 irradiance = vec3(0.0);\n"
    "    vec3 up = UP;\n"
    "    vec3 right = cross(up, N);\n"
    "    up = cross(N, right);\n"
    "    float sampleDelta = 0.025;\n"
    "    float nrSamples = 0.0;\n"
    "    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta){\n"
    "        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta){\n"
    "            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));\n"
    "            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;\n"
    "            irradiance += textureCube(cubemap, sampleVec).rgb * cos(theta) * sin(theta);\n"
    "            nrSamples++;\n"
    "        }\n"
    "    }\n"
    "    irradiance = PI * irradiance * (1.0 / nrSamples);\n"
    "    gl_FragColor = vec4(irradiance, 1.0);\n"
    "}\n";
#pragma endregion

#pragma region CubemapPrefilterEnvMap
//https://placeholderart.wordpress.com/2015/07/28/implementation-notes-runtime-environment-map-filtering-for-image-based-lighting/
//  ^
//  L_________ visit that link to optimize this process
//
// this shader is heavily modified based on optimizations in the link above. the optimizations are not complete yet, and 
// what seems to look correct may not be. this shader might have to be modified against the original later on.
priv::EShaders::cubemap_prefilter_envmap_frag =
    "varying vec3 UV;\n"

    "uniform SAMPLER_TYPE_Cube cubemap;\n"
    "uniform float roughness;\n"
    "uniform float a2;\n"
    "uniform float PiFourDividedByResSquaredTimesSix;\n"
    "uniform int NUM_SAMPLES;\n"

    "const float PI = 3.14159265;\n"
    "const float PI2 = 6.283185;\n"

    "float DistributionGGX(vec3 Half){\n"
    "    float NdotH2 = Half.z * Half.z;\n"
    "    float denom = (NdotH2 * (a2 - 1.0) + 1.0);\n"
    "    denom = PI * denom * denom;\n"
    "    return a2 / denom;\n"
    "}\n"
    
    "vec3 ImportanceSampleGGX(vec2 Xi, mat3 TBN){\n"
    "    float phi = PI2 * Xi.x;\n"
    "    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a2 - 1.0) * Xi.y));\n"
    "    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);\n"
    "    vec3 Half;\n"
    "    Half.x = sinTheta * cos(phi);\n"
    "    Half.y = sinTheta * sin(phi);\n"
    "    Half.z = cosTheta;\n"
    "    return normalize(TBN * Half);\n"
    "}\n"

    "void main(){\n"
    "    vec3 N = normalize(UV);\n"
    "    vec3 R = N;\n"
    "    vec3 V = R;\n"
    "    vec3 PreEnv = vec3(0.0);\n"
    "    float totalWeight = 0.0;\n"
    "    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);\n"
    "    vec3 tangent = normalize(cross(up, N));\n"
    "    vec3 bitangent = cross(N, tangent);\n"
    "    mat3 TBN = mat3(tangent,bitangent,N);\n"
    "    for(int i = 0; i < NUM_SAMPLES; ++i){\n"
    "        vec2 Xi = HammersleySequence(i, NUM_SAMPLES);\n"
    "        vec3 L = ImportanceSampleGGX(Xi,TBN);\n"
    "        float NdotL = max(dot(N, L), 0.0);\n"
    "        if(NdotL > 0.0){\n"
    "            float D   = DistributionGGX(L);\n"
    "            float pdf = D * L.z / (4.0 * L.z) + 0.0001;\n"
    "            float saSample = 1.0 / (float(NUM_SAMPLES) * pdf + 0.0001);\n"
    "            float mipLevel = (roughness == 0.0) ? 0.0 : 0.5 * log2(saSample / PiFourDividedByResSquaredTimesSix);\n"
    "            PreEnv += textureCubeLod(cubemap, L, mipLevel).rgb * NdotL;\n"
    "            totalWeight += NdotL;\n"
    "        }\n"
    "    }\n"
    "    PreEnv = PreEnv / totalWeight;\n"
    "    gl_FragColor = vec4(PreEnv, 1.0);\n"
    "}";
#pragma endregion

#pragma region BRDFPrecompute

priv::EShaders::brdf_precompute =
    "const float PI2 = 6.283185;\n"
    "uniform int NUM_SAMPLES;\n"
    "varying vec2 texcoords;\n"

    "vec3 ImportanceSampleGGX(vec2 Xi, float a2,mat3 TBN){\n"
    "    float phi = PI2 * Xi.x;\n"
    "    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a2 - 1.0) * Xi.y));\n"
    "    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);\n"
    "    vec3 Half;\n"
    "    Half.x = cos(phi) * sinTheta;\n"
    "    Half.y = sin(phi) * sinTheta;\n"
    "    Half.z = cosTheta;\n"
    "    return normalize(TBN * Half);\n"
    "}\n"

    "float GeometrySchlickGGX(float NdotV, float a){\n"
    "    float k = a / 2.0;\n"
    "    float denom = NdotV * (1.0 - k) + k;\n"
    "    return NdotV / denom;\n"
    "}\n"

    "float GeometrySmith(vec3 N, vec3 V, vec3 L, float a){\n"
    "    float NdotV = max(dot(N, V), 0.0);\n"
    "    float NdotL = max(dot(N, L), 0.0);\n"
    "    float ggx2 = GeometrySchlickGGX(NdotV, a);\n"
    "    float ggx1 = GeometrySchlickGGX(NdotL, a);\n"
    "    return ggx1 * ggx2;\n"
    "}\n"

    "vec2 IntegrateBRDF(float NdotV, float roughness){\n"
    "    vec3 V;\n"
    "    V.x = sqrt(1.0 - NdotV * NdotV);\n"
    "    V.y = 0.0;\n"
    "    V.z = NdotV;\n"
    "    float A = 0.0;\n"
    "    float B = 0.0;\n"
    "    vec3 N = vec3(0.0, 0.0, 1.0);\n"
    "    float a = roughness * roughness;\n"
    "    float a2 = a * a;\n"
    "    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);\n"
    "    vec3 tangent   = normalize(cross(up, N));\n"
    "    vec3 bitangent = cross(N, tangent);\n"
    "    mat3 TBN = mat3(tangent,bitangent,N);\n"
    "    for(int i = 0; i < NUM_SAMPLES; ++i){\n"
    "        vec2 Xi = HammersleySequence(i, NUM_SAMPLES);\n"
    "        vec3 H = ImportanceSampleGGX(Xi,a2,TBN);\n"
    "        vec3 L = normalize(2.0 * dot(V, H) * H - V);\n"
    "        float NdotL = max(L.z, 0.0);\n"
    "        float NdotH = max(H.z, 0.0);\n"
    "        float VdotH = max(dot(V, H), 0.0);\n"
    "        if(NdotL > 0.0){\n"
    "            float G = GeometrySmith(N, V, L, a);\n"
    "            float G_Vis = (G * VdotH) / (NdotH * NdotV);\n"
    "            float Fc = pow(1.0 - VdotH, 5.0);\n"
    "            A += (1.0 - Fc) * G_Vis;\n"
    "            B += Fc * G_Vis;\n"
    "        }\n"
    "    }\n"
    "    A /= float(NUM_SAMPLES);\n"
    "    B /= float(NUM_SAMPLES);\n"
    "    return vec2(A, B);\n"
    "}\n"

    "void main(){\n"
    "    gl_FragColor.rg = IntegrateBRDF(texcoords.x, texcoords.y);\n"
    "}";
#pragma endregion

#pragma region StencilPass

priv::EShaders::stencil_passover =
    "\n"
    "const vec3 comparison = vec3(1.0, 1.0, 1.0);\n"
    "uniform SAMPLER_TYPE_2D gNormalMap;\n"
    "varying vec2 texcoords;\n"
    "void main(){\n"
    "    vec3 normal = DecodeOctahedron(texture2D(gNormalMap, texcoords).rg);\n"
    "    if(distance(normal, comparison) < 0.01){\n"
    "        discard;\n"//this is where the magic happens with the stencil buffer.
    "    }\n"
    "}";

#pragma endregion

#pragma region FullscreenQuadVertex
priv::EShaders::fullscreen_quad_vertex = 
    "\n"
    "layout (location = 0) in vec3 position;\n"
    "layout (location = 1) in vec2 uv;\n"
    "\n"
    "uniform mat4 Model;\n"
    "uniform mat4 VP;\n"
    "\n"
    "varying vec2 texcoords;\n"
    "\n"
    "void main(){\n"
    "    vec3 vert = position;\n"
    "    texcoords = uv;\n"
    "    gl_Position = VP * Model * vec4(vert, 1.0);\n"
    "}";
#pragma endregion
   
#pragma region ForwardFrag
priv::EShaders::forward_frag =
    "\n"
    "USE_LOG_DEPTH_FRAGMENT\n"
    "USE_MAX_MATERIAL_LAYERS_PER_COMPONENT\n"
    "USE_MAX_MATERIAL_COMPONENTS\n"
    "USE_MAX_LIGHTS_PER_PASS\n"
    "\n"
    "struct InData {\n"
    "    vec2  uv;\n"
    "    vec4  diffuse;\n"
    "    vec4  objectColor;\n"
    "    vec3  normals;\n"
    "    float glow;\n"
    "    float specular;\n"
    "    float ao;\n"
    "    float metalness;\n"
    "    float smoothness;\n"
    "    vec3  materialF0;\n"
    "    vec3  worldPosition;\n"
    "};\n"
    "struct Layer {\n"
    "    vec4 data1;\n"//x = blend mode | y = texture enabled? | z = mask enabled? | w = cubemap enabled?
    "    vec4 data2;\n"
    "    SAMPLER_TYPE_2D texture;\n"
    "    SAMPLER_TYPE_2D mask;\n"
    "    SAMPLER_TYPE_Cube cubemap;\n"
    "    vec4 uvModifications;\n"
    "};\n"
    "struct Component {\n"
    "    ivec2 componentData;\n" //x = numLayers, y = componentType
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
    "varying vec2 UV;\n"
    "varying vec3 Normals;\n"
    "varying mat3 TBN;\n"
    "flat varying vec3 CamPosition;\n"
    "flat varying vec3 CamRealPosition;\n"
    "varying vec3 TangentCameraPos;\n"
    "varying vec3 TangentFragPos;\n"
    "\n"
    "uniform SAMPLER_TYPE_Cube irradianceMap;\n"
    "uniform SAMPLER_TYPE_Cube prefilterMap;\n"
    "uniform SAMPLER_TYPE_2D brdfLUT;\n"
    "uniform SAMPLER_TYPE_2D gTextureMap;\n"
    "uniform vec4 ScreenData;\n" //x = GIContribution, y = gamma, z = winSize.x, w = winSize.y
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
    "    for (int j = 0; j < numComponents; ++j) {\n"
    "        ProcessComponent(components[j], inData);\n"
    "    }\n"
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
    "                lightCalculation = CalcLightInternal(currentLight, normalize(LightPosition - WorldPosition), WorldPosition, inData.normals, inData.specular, inData.diffuse.rgb, SSAO, MetalSmooth, MaterialBasePropertiesTwo.x, inData.materialF0, MaterialBasePropertiesTwo.y, MaterialBasePropertiesTwo.z, AO);\n"
    "            }else if(currentLight.DataD.w == 1.0){\n" //point
    "                lightCalculation = CalcPointLight(currentLight, LightPosition, WorldPosition, inData.normals, inData.specular, inData.diffuse.rgb, SSAO, MetalSmooth, MaterialBasePropertiesTwo.x, inData.materialF0, MaterialBasePropertiesTwo.y, MaterialBasePropertiesTwo.z, AO);\n"
    "            }else if(currentLight.DataD.w == 2.0){\n" //directional
    "                lightCalculation = CalcLightInternal(currentLight, LightDirection, WorldPosition, inData.normals, inData.specular, inData.diffuse.rgb, SSAO, MetalSmooth, MaterialBasePropertiesTwo.x, inData.materialF0, MaterialBasePropertiesTwo.y, MaterialBasePropertiesTwo.z, AO);\n"
    "            }else if(currentLight.DataD.w == 3.0){\n" //spot
    "                lightCalculation = CalcSpotLight(currentLight, LightDirection, LightPosition, WorldPosition, inData.normals, inData.specular, inData.diffuse.rgb, SSAO, MetalSmooth, MaterialBasePropertiesTwo.x, inData.materialF0, MaterialBasePropertiesTwo.y, MaterialBasePropertiesTwo.z, AO);\n"
    "            }else if(currentLight.DataD.w == 4.0){\n" //rod
    "                lightCalculation = CalcRodLight(currentLight, vec3(currentLight.DataA.w, currentLight.DataB.xy), currentLight.DataC.yzw, WorldPosition, inData.normals, inData.specular, inData.diffuse.rgb, SSAO, MetalSmooth, MaterialBasePropertiesTwo.x, inData.materialF0, MaterialBasePropertiesTwo.y, MaterialBasePropertiesTwo.z, AO);\n"
    "            }else if(currentLight.DataD.w == 5.0){\n" //projection
    "                lightCalculation = CalcProjectionLight(currentLight, vec3(currentLight.DataA.w, currentLight.DataB.xy), currentLight.DataC.yzw, WorldPosition, inData.normals, inData.specular, inData.diffuse.rgb, SSAO, MetalSmooth, MaterialBasePropertiesTwo.x, inData.materialF0, MaterialBasePropertiesTwo.y, MaterialBasePropertiesTwo.z, AO);\n"
    "            }\n"
    "            lightTotal += lightCalculation;\n"
    "        }\n"
             //GI here
    "        vec3 inGIContribution = Unpack3FloatsInto1FloatUnsigned(ScreenData.x);\n" //x = diffuse, y = specular, z = global
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
    priv::EShaders::particle_frag =
    "\n"
    "USE_LOG_DEPTH_FRAGMENT\n"

    "uniform SAMPLER_TYPE_2D DiffuseTexture0;\n";

    for (uint32_t i = 1; i < std::min(priv::OpenGLState::MAX_TEXTURE_UNITS - 1U, MAX_UNIQUE_PARTICLE_TEXTURES_PER_FRAME); ++i) {
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

    priv::EShaders::particle_frag += "";

    for (uint32_t i = 1; i < std::min(priv::OpenGLState::MAX_TEXTURE_UNITS - 1U, MAX_UNIQUE_PARTICLE_TEXTURES_PER_FRAME); ++i) {
        priv::EShaders::particle_frag +=
            "    else if (MaterialIndex == " + std::to_string(i) + "U)\n"
            "        finalColor *= texture2D(DiffuseTexture" + std::to_string(i) + ", UV); \n";
    }

    priv::EShaders::particle_frag +=
    "    else\n"
    "        finalColor *= texture2D(DiffuseTexture0, UV); \n"

    "    finalColor.a *= alpha;\n"

    "    SUBMIT_DIFFUSE(finalColor);\n"
    "    gl_FragData[3] = finalColor;\n"
    "}";
#pragma endregion

#pragma region DeferredFrag
priv::EShaders::deferred_frag =
    "\n"
    "USE_LOG_DEPTH_FRAGMENT\n"
    "USE_MAX_MATERIAL_LAYERS_PER_COMPONENT\n"
    "USE_MAX_MATERIAL_COMPONENTS\n"
    "\n"
    "struct InData {\n"
    "    vec2  uv;\n"
    "    vec4  diffuse;\n"
    "    vec4  objectColor;\n"
    "    vec3  normals;\n"
    "    float glow;\n"
    "    float specular;\n"
    "    float ao;\n"
    "    float metalness;\n"
    "    float smoothness;\n"
    "    vec3  worldPosition;\n"
    "};\n"
    "struct Layer {\n"
    "    vec4 data1;\n"//x = blend mode | y = texture enabled? | z = mask enabled? | w = cubemap enabled?
    "    vec4 data2;\n"
    "    SAMPLER_TYPE_2D texture;\n"
    "    SAMPLER_TYPE_2D mask;\n"
    "    SAMPLER_TYPE_Cube cubemap;\n"
    "    vec4 uvModifications;\n"
    "};\n"
    "struct Component {\n"
    "    ivec2 componentData;\n" //x = numLayers, y = componentType
    "    Layer layers[MAX_MATERIAL_LAYERS_PER_COMPONENT];\n"
    "};\n"
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
    "varying vec2 UV;\n"
    "varying vec3 Normals;\n"
    "varying mat3 TBN;\n"
    "flat varying vec3 CamPosition;\n"
    "varying vec3 TangentCameraPos;\n"
    "varying vec3 TangentFragPos;\n"
    "\n"
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
    "    inData.worldPosition = WorldPosition;\n"
    "\n"
    "    for (int j = 0; j < numComponents; ++j) {\n"
    "        ProcessComponent(components[j], inData);\n"
    "    }\n"
    "\n"
    "    vec2 OutNormals = EncodeOctahedron(inData.normals);\n"
    "    if(Shadeless == 1){\n"
    "        OutNormals = ConstantOneVec2;\n"
    "        inData.diffuse *= (1.0 + inData.glow);\n" // we want shadeless items to be influenced by the glow somewhat...
    "    }\n"
    "    vec4 GodRays = Unpack32BitUIntTo4ColorFloats(Gods_Rays_Color);\n"
    "    inData.diffuse.a *= MaterialBasePropertiesTwo.x;\n"

    "    SUBMIT_DIFFUSE(inData.diffuse);\n"
    "    SUBMIT_NORMALS(OutNormals);\n"
    "    SUBMIT_MATERIAL_ID_AND_AO(Material_F0AndID.w, inData.ao);\n"
    "    SUBMIT_METALNESS_AND_SMOOTHNESS(inData.metalness,inData.smoothness);\n"
    "    SUBMIT_GLOW(inData.glow);\n"
    "    SUBMIT_SPECULAR(inData.specular);\n"
    "    SUBMIT_GOD_RAYS_COLOR(GodRays.r, GodRays.g, GodRays.b);\n"
    "}";
#pragma endregion

#pragma region NormalessDiffuseFrag

priv::EShaders::normaless_diffuse_frag = 
    "\n"
    "const vec3 comparison = vec3(1.0, 1.0, 1.0);\n"
    "uniform SAMPLER_TYPE_2D gNormalMap;\n"
    "uniform SAMPLER_TYPE_2D gDiffuseMap;\n"
    "uniform int HasLighting;\n"
    "varying vec2 texcoords;\n"
    "void main(){\n"
    "    vec3 normal   = DecodeOctahedron(texture2D(gNormalMap, texcoords).rg);\n"
    "    vec3 diffuse  = texture2D(USE_SAMPLER_2D(gDiffuseMap), texcoords).rgb;\n"
    "    if(HasLighting == 0 || distance(normal, comparison) < 0.01){\n"
    "        gl_FragColor = vec4(diffuse, 1.0);\n"
    "    }else{\n"
    "        discard;\n"
    "    }\n"
    "}";

#pragma endregion

#pragma region ZPrepassFrag
priv::EShaders::zprepass_frag =
    "USE_LOG_DEPTH_FRAGMENT\n"
    //"USE_MAX_MATERIAL_LAYERS_PER_COMPONENT\n"
    //"USE_MAX_MATERIAL_COMPONENTS\n"
    "void main(){\n"
    "}";

#pragma endregion

#pragma region DeferredFragHUD
priv::EShaders::deferred_frag_hud =
    "\n"
    "uniform SAMPLER_TYPE_2D DiffuseTexture;\n"

    "uniform int DiffuseTextureEnabled;\n"
    "uniform float ScreenGamma;\n"
    "uniform vec4 Object_Color;\n"
    "varying vec2 UV;\n"
    "void main(){\n"

    "    gl_FragColor = Object_Color;\n"
    "    if(DiffuseTextureEnabled == 1){\n"
    "        vec4 color = texture2D(DiffuseTexture, UV); \n"
    "        color.rgb = pow(color.rgb, vec3(1.0 / ScreenGamma));\n" //ScreenGamma is gamma
    "        gl_FragColor *= color;\n"
    "    }\n"
    "}";
#pragma endregion

#pragma region DeferredFragSkybox
priv::EShaders::deferred_frag_skybox =
    "\n"
    "uniform vec4 Color;\n"
    "uniform int IsFake;\n"
    "uniform float ScreenGamma;\n"
    "uniform SAMPLER_TYPE_Cube Texture;\n"
    "varying vec3 UV;\n"
    "varying vec3 WorldPosition;\n"
    "void main(){\n"
    "    if(IsFake == 1){\n"
    "        SUBMIT_DIFFUSE(Color);\n"
    "    }else{\n"
    "        SUBMIT_DIFFUSE(textureCube(Texture, UV));\n"
    "    }\n"
    "    gl_FragData[0].rgb = pow(gl_FragData[0].rgb, vec3(1.0 / ScreenGamma));\n" //ScreenGamma is gamma
    "    gl_FragData[1].rg = vec2(1.0);\n"
    "    gl_FragData[2]    = vec4(0.0);\n"
    "}";
#pragma endregion

#pragma region CopyDepthFrag
priv::EShaders::copy_depth_frag =
    "\n"
    "uniform SAMPLER_TYPE_2D gDepthMap;\n"
    "varying vec2 texcoords;\n"
    "void main(){\n"
    "    gl_FragDepth = texture2D(gDepthMap, texcoords).r;\n"
    "}";
#pragma endregion

#pragma region Blur
priv::EShaders::blur_frag =
    "uniform SAMPLER_TYPE_2D image;\n"
    "\n"
    "uniform vec4 DataA;\n"//radius, UNUSED, H,V
    "uniform vec4 strengthModifier;\n"
    "\n"
    "varying vec2 texcoords;\n"
    "\n"
    "const int NUM_SAMPLES = 9;\n"
    "const float weight[NUM_SAMPLES] = float[](0.227,0.21,0.1946,0.162,0.12,0.08,0.054,0.03,0.016);\n"
    "\n"
    "void main(){\n"
    "    vec4 Sum = vec4(0.0);\n"
    "    vec2 inverseResolution = vec2(1.0) / vec2(ScreenInfo.z, ScreenInfo.w);\n"
    "    for(int i = 0; i < NUM_SAMPLES; ++i){\n"
    "        vec2 offset = (inverseResolution * float(i)) * DataA.x;\n"
    "        Sum += (texture2D(image,texcoords + vec2(offset.x * DataA.z,offset.y * DataA.w)) * weight[i]) * strengthModifier;\n"
    "        Sum += (texture2D(image,texcoords - vec2(offset.x * DataA.z,offset.y * DataA.w)) * weight[i]) * strengthModifier;\n"
    "    }\n"
    "    gl_FragColor = Sum;\n"
    "}";
#pragma endregion

#pragma region Greyscale
/*
priv::EShaders::greyscale_frag =
    "\n"
    "uniform SAMPLER_TYPE_2D textureMap;\n"
    "varying vec2 texcoords;\n"
    "void main(){\n"
    "    vec4 col = texture2D(textureMap, texcoords);\n"
    "    float lum = dot(col.rgb, vec3(0.299, 0.587, 0.114));\n"
    "    gl_FragColor = vec4(vec3(lum), 1.0);\n"
    "}";
*/
#pragma endregion
    
#pragma region FinalFrag
priv::EShaders::final_frag =
    "\n"
    "uniform SAMPLER_TYPE_2D SceneTexture;\n"
    "uniform SAMPLER_TYPE_2D gBloomMap;\n"
    "uniform SAMPLER_TYPE_2D gDepthMap;\n"
    "uniform SAMPLER_TYPE_2D gDiffuseMap;\n"
    "\n"
    "uniform int HasBloom;\n"
    "uniform int HasFog;\n"
    "\n"
    "uniform vec4 FogColor;\n"
    "uniform float FogDistNull;\n"
    "uniform float FogDistBlend;\n"
    "\n"
    "varying vec2 texcoords;\n"
    "\n"
    "void main(){\n"
    "    vec4 scene = texture2D(SceneTexture,texcoords);\n"
    "    vec4 diffuse = texture2D(gDiffuseMap,texcoords);\n"
    "    if(HasBloom == 1){\n"
    "        vec4 bloom = texture2D(gBloomMap,texcoords);\n"
    "        scene += bloom;\n"
    "    }\n"
    "    gl_FragColor = scene;\n"
    "    if(HasFog == 1){\n"
    "        float distFrag = distance(GetWorldPosition(USE_SAMPLER_2D(gDepthMap), texcoords,CameraNear,CameraFar),CameraPosition);\n"
    "        float distVoid = FogDistNull + FogDistBlend;\n"
    "        float distBlendIn = FogDistBlend - (distVoid - distFrag);\n"
    "        float omega = smoothstep(0.0,1.0,(distBlendIn / FogDistBlend));\n"
    "        vec4 fc = FogColor * clamp(omega,0.0,1.0);\n"
    "        gl_FragColor = PaintersAlgorithm(fc,gl_FragColor);\n"
    "    }\n"
    //"    gl_FragColor = (gl_FragColor * 0.0001) + vec4(1.0 - texture2D(gBloomMap,texcoords).a);\n"
    "}";

#pragma endregion

#pragma region DepthAndTransparency

priv::EShaders::depth_and_transparency_frag = 
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
priv::EShaders::depth_and_transparency_frag +=
    "\n"
    "void main(){\n"
    "    vec4 scene = texture2D(SceneTexture,texcoords);\n"
    "    float depth = distance(GetWorldPosition(USE_SAMPLER_2D(gDepthMap), texcoords, CameraNear, CameraFar), CameraPosition);\n"
    //"    if(TransparencyMaskActive == 1 && scene.rgb == TransparencyMaskColor.rgb){\n"
    //"        scene.a = 0.0;\n"
    //"    }\n"
    "    if(DepthMaskActive == 1 && depth > DepthMaskValue){\n"
    //"        scene.a = 0.0;\n" //"erase" the texture if the depth exceeds what the viewport wants
    "           discard;\n"
    "    }\n"
    "    gl_FragColor = scene;\n"
    "}";
#pragma endregion

#pragma region LightingFrag
priv::EShaders::lighting_frag =
    "#define MATERIAL_COUNT_LIMIT 255\n"
    "\n"
    "struct Light {\n"
    "    vec4 DataA;\n" //x = ambient,          y = diffuse,          z = specular,         w = LightDirection.x
    "    vec4 DataB;\n" //x = LightDirection.y, y = LightDirection.z, z = const,            w = linear
    "    vec4 DataC;\n" //x = exp,              y = LightPosition.x,  z = LightPosition.y,  w = LightPosition.z
    "    vec4 DataD;\n" //x = LightColor.r,     y = LightColor.g,     z = LightColor.b,     w = LightType
    "    vec4 DataE;\n" //x = cutoff,           y = outerCutoff,      z = AttenuationModel, w = UNUSED
    "};\n"
    "uniform Light light;\n"
    "\n"
    "uniform SAMPLER_TYPE_2D gDiffuseMap;\n"
    "uniform SAMPLER_TYPE_2D gNormalMap;\n"
    "uniform SAMPLER_TYPE_2D gMiscMap;\n"
    "uniform SAMPLER_TYPE_2D gDepthMap;\n"
    "uniform SAMPLER_TYPE_2D gSSAOMap;\n"
    "uniform SAMPLER_TYPE_2D gTextureMap;\n"
    "\n"
    "uniform vec4 ScreenData;\n" //x = UNUSED, y = screenGamma, z = winSize.x, w = winSize.y
    "uniform vec4 materials[MATERIAL_COUNT_LIMIT];\n"//r = MaterialF0Color (packed into float), g = baseSmoothness, b = specularModel, a = diffuseModel
    "\n"
    "varying vec2 texcoords;\n"
    "flat varying vec3 CamRealPosition;\n"
    "\n"
    "void main(){\n"                      //windowX      //windowY
    "    vec2 uv = gl_FragCoord.xy / vec2(ScreenData.z, ScreenData.w);\n"
    "    vec3 PxlNormal        = DecodeOctahedron(texture2D(USE_SAMPLER_2D(gNormalMap), uv).rg);\n"
    "    vec3 PxlPosition      = GetWorldPosition(USE_SAMPLER_2D(gDepthMap), uv, CameraNear, CameraFar);\n"
    "    vec3 lightCalculation = ConstantZeroVec3;\n"
    "    vec3 LightPosition    = vec3(light.DataC.yzw) - CamRealPosition;\n"
    "    vec3 LightDirection   = normalize(vec3(light.DataA.w, light.DataB.x, light.DataB.y));\n"
    "    float Specular        = texture2D(USE_SAMPLER_2D(gMiscMap), uv).g;\n"
    "    vec3 Albedo           = texture2D(USE_SAMPLER_2D(gDiffuseMap), uv).rgb;\n"
    "    float MatIDAndAO      = texture2D(USE_SAMPLER_2D(gNormalMap), uv).b;\n"
    "    float SSAO            = 1.0 - texture2D(gSSAOMap, uv).a;\n"
    "    vec2 MetalSmooth      = Unpack2NibblesFrom8BitChannel(texture2D(USE_SAMPLER_2D(gMiscMap), uv).r);\n"
    "    highp int matID       = int(floor(MatIDAndAO));\n"
    "    float MatAlpha        = materials[matID].g;\n"
    "    vec3 MatF0            = Unpack3FloatsInto1FloatUnsigned(materials[matID].r);\n"
    "    float MatTypeDiffuse  = materials[matID].a;\n"
    "    float MatTypeSpecular = materials[matID].b;\n"
    "    float AO              = (fract(MatIDAndAO) + 0.0001);\n"
    "    if(light.DataD.w == 0.0){\n"       //sun
    "        lightCalculation = CalcLightInternal(light, normalize(LightPosition - PxlPosition), PxlPosition, PxlNormal, Specular, Albedo, SSAO, MetalSmooth, MatAlpha, MatF0, MatTypeDiffuse, MatTypeSpecular, AO);\n"
    "    }else if(light.DataD.w == 1.0){\n" //point
    "        lightCalculation = CalcPointLight(light, LightPosition, PxlPosition, PxlNormal, Specular, Albedo, SSAO, MetalSmooth, MatAlpha, MatF0, MatTypeDiffuse, MatTypeSpecular, AO);\n"
    "    }else if(light.DataD.w == 2.0){\n" //directional
    "        lightCalculation = CalcLightInternal(light, LightDirection, PxlPosition, PxlNormal, Specular, Albedo, SSAO, MetalSmooth, MatAlpha, MatF0, MatTypeDiffuse, MatTypeSpecular, AO);\n"
    "    }else if(light.DataD.w == 3.0){\n" //spot
    "        lightCalculation = CalcSpotLight(light, LightDirection, LightPosition, PxlPosition, PxlNormal, Specular, Albedo, SSAO, MetalSmooth, MatAlpha, MatF0, MatTypeDiffuse, MatTypeSpecular, AO);\n"
    "    }else if(light.DataD.w == 4.0){\n" //rod
    "        lightCalculation = CalcRodLight(light, vec3(light.DataA.w, light.DataB.xy), light.DataC.yzw, PxlPosition, PxlNormal, Specular, Albedo, SSAO, MetalSmooth, MatAlpha, MatF0, MatTypeDiffuse, MatTypeSpecular, AO);\n"
    "    }else if(light.DataD.w == 5.0){\n" //projection
    "        lightCalculation = CalcProjectionLight(light, vec3(light.DataA.w, light.DataB.xy), light.DataC.yzw, PxlPosition, PxlNormal, Specular, Albedo, SSAO, MetalSmooth, MatAlpha, MatF0, MatTypeDiffuse, MatTypeSpecular, AO);\n"
    "    }\n"
    "    gl_FragData[0].rgb = lightCalculation;\n"
    "}";
#pragma endregion

#pragma region LightingFragGI
priv::EShaders::lighting_frag_gi =
    "#define MATERIAL_COUNT_LIMIT 255\n"
    "\n"
    "const float MAX_REFLECTION_LOD = 5.0;\n"
    "\n"
    "uniform SAMPLER_TYPE_2D gDiffuseMap;\n"
    "uniform SAMPLER_TYPE_2D gNormalMap;\n"
    "uniform SAMPLER_TYPE_2D gDepthMap;\n"
    "uniform SAMPLER_TYPE_2D gSSAOMap;\n"
    "uniform SAMPLER_TYPE_2D gMiscMap;\n"
    "uniform SAMPLER_TYPE_2D brdfLUT;\n"
    "\n"
    "uniform SAMPLER_TYPE_Cube irradianceMap;\n"
    "uniform SAMPLER_TYPE_Cube prefilterMap;\n"
    "\n"
    "uniform vec4 ScreenData;\n" //x = GIContribution, y = gamma, z = winSize.x, w = winSize.y
    "uniform vec4 materials[MATERIAL_COUNT_LIMIT];\n"//r = MaterialF0Color (packed into float), g = baseSmoothness, b = specularModel, a = diffuseModel
    "\n"
    "varying vec2 texcoords;\n"
    "flat varying vec3 CamRealPosition;\n" //add this to calculations?
    "\n"
    "vec3 SchlickFrenselRoughness(float inTheta, vec3 inF0, float inRoughness){\n"
    "    return inF0 + (max(vec3(1.0 - inRoughness), inF0) - inF0) * pow(1.0 - inTheta, 5.0);\n"
    "}\n"
    "void main(){\n"
    "    vec2 uv               = gl_FragCoord.xy / vec2(ScreenData.z, ScreenData.w);\n"

    "    float inSSAO          = 1.0 - texture2D(USE_SAMPLER_2D(gSSAOMap), uv).a;\n"
    "    vec3 inNormals        = DecodeOctahedron(texture2D(USE_SAMPLER_2D(gNormalMap), uv).rg);\n"
    "    vec3 inAlbedo         = texture2D(USE_SAMPLER_2D(gDiffuseMap), uv).rgb;\n"
    "    vec3 inWorldPosition  = GetWorldPosition(USE_SAMPLER_2D(gDepthMap), uv, CameraNear, CameraFar);\n"
    "    float inGlow          = texture2D(USE_SAMPLER_2D(gNormalMap), uv).a;\n"
    "    vec2 inMetalSmooth    = Unpack2NibblesFrom8BitChannel(texture2D(USE_SAMPLER_2D(gMiscMap), uv).r);\n"
    "    float MatIDAndAO      = texture2D(USE_SAMPLER_2D(gNormalMap), uv).b;\n"

    "    highp int index       = int(floor(MatIDAndAO));\n"
    "    float inAO            = (fract(MatIDAndAO) + 0.0001);\n"
    "    vec3 inMatF0          = Unpack3FloatsInto1FloatUnsigned(materials[index].r);\n"
    "    vec3 inGIContribution = Unpack3FloatsInto1FloatUnsigned(ScreenData.x);\n" //x = diffuse, y = specular, z = global
    "    gl_FragColor         += CalcGILight(inSSAO, inNormals, inAlbedo, inWorldPosition, inAO, inMetalSmooth.x, inMetalSmooth.y, inGlow, inMatF0, materials[index].g, inGIContribution);\n"
    "}";

#pragma endregion

}
