#include "Engine_BuiltInShaders.h"

using namespace Engine;
using namespace std;

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
string epriv::EShaders::constants;
string epriv::EShaders::conditional_functions;
string epriv::EShaders::float_into_2_floats;
string epriv::EShaders::normals_octahedron_compression_functions;
string epriv::EShaders::bullet_physics_vert;
string epriv::EShaders::bullet_physcis_frag;
string epriv::EShaders::fullscreen_quad_vertex;
string epriv::EShaders::vertex_basic;
string epriv::EShaders::vertex_hud;
string epriv::EShaders::vertex_skybox;
string epriv::EShaders::lighting_vert;
string epriv::EShaders::stencil_passover;
string epriv::EShaders::depth_of_field;
string epriv::EShaders::smaa_common;
string epriv::EShaders::smaa_frag_1_stencil;
string epriv::EShaders::smaa_vertex_1;
string epriv::EShaders::smaa_frag_1;
string epriv::EShaders::smaa_vertex_2;
string epriv::EShaders::smaa_frag_2;
string epriv::EShaders::smaa_vertex_3;
string epriv::EShaders::smaa_frag_3;
string epriv::EShaders::smaa_vertex_4;
string epriv::EShaders::smaa_frag_4;
string epriv::EShaders::fxaa_frag;
string epriv::EShaders::forward_frag;
string epriv::EShaders::deferred_frag;
string epriv::EShaders::deferred_frag_hud;
string epriv::EShaders::deferred_frag_skybox;
string epriv::EShaders::deferred_frag_skybox_fake;
string epriv::EShaders::copy_depth_frag;
string epriv::EShaders::cubemap_convolude_frag;
string epriv::EShaders::cubemap_prefilter_envmap_frag;
string epriv::EShaders::brdf_precompute;
string epriv::EShaders::ssao_frag;
string epriv::EShaders::bloom_frag;
string epriv::EShaders::hdr_frag;
string epriv::EShaders::godRays_frag;
string epriv::EShaders::blur_frag;
string epriv::EShaders::ssao_blur_frag;
string epriv::EShaders::greyscale_frag;
string epriv::EShaders::final_frag;
string epriv::EShaders::lighting_frag;
string epriv::EShaders::lighting_frag_gi;
#pragma endregion

void epriv::EShaders::init(){

#pragma region Constants
epriv::EShaders::constants = 
    "const vec3 ConstantOneVec3 = vec3(1.0,1.0,1.0);\n"
    "const vec2 ConstantOneVec2 = vec2(1.0,1.0);\n"
    "\n"
    "const vec3 ConstantAlmostOneVec3 = vec3(0.9999,0.9999,0.9999);\n"
    "const vec2 ConstantAlmostOneVec2 = vec2(0.9999,0.9999);\n"
    "\n"
    "const vec3 ConstantZeroVec3 = vec3(0.0,0.0,0.0);\n"
    "const vec2 ConstantZeroVec2 = vec2(0.0,0.0);\n"
    "\n"
    "const float KPI = 3.1415926535898;\n"
    "\n";
#pragma endregion

#pragma region Functions

epriv::EShaders::conditional_functions =
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

epriv::EShaders::float_into_2_floats =
    //designed to work with floats from 0 to 1, packing them into an 8 bit component of a render target
    "float Pack2NibblesInto8BitChannel(float x,float y){\n"
    "    float _x = clamp(x,0.01,0.99);\n"
    "    float _y = clamp(y,0.01,0.99);\n"
    "    float xF = round(_x / 0.0666);\n"
    "    float yF = round(_y / 0.0666) * 16.0;\n"
    "    return (xF + yF) / 255.0;\n"
    "}\n"
    "vec2 Unpack2NibblesFrom8BitChannel(float data){\n"
    "    float d = data * 255.0;\n"
    "    float y = fract(d / 16.0);\n"
    "    float x = (d - (y * 16.0));\n"
    "    return vec2(y, x / 255.0);\n"
    "}\n"
    //designed to work with uniform floats coming from the cpu
    "vec3 Unpack3FloatsInto1FloatUnsigned(float v){\n"
    "    vec3 ret;\n"
    "    ret.r = mod(v,          1.0);\n"
    "    ret.g = mod(v * 256.0,  1.0);\n"
    "    ret.b = mod(v * 65536.0,1.0);\n"
    "    return ret;\n"
    "}\n"
    "vec3 Unpack3FloatsInto1Float(float v){\n"
    "    vec3 ret;\n"
    "    ret.r = mod(v,          1.0);\n"
    "    ret.g = mod(v * 256.0,  1.0);\n"
    "    ret.b = mod(v * 65536.0,1.0);\n"
    "    ret = ret * 2.0 - 1.0;\n"
    "    return ret;\n"
    "}\n"
    //designed to work with gbuffer 16-bit floating point buffers
    "float Pack2FloatIntoFloat16(float x,float y){\n"
    "    x = clamp(x,0.0001,0.9999);\n"
    "    y = clamp(y,0.0001,0.9999);\n"
    "    float _x = (x + 1.0) * 0.5;\n"
    "    float _y = (y + 1.0) * 0.5;\n"
    "    return floor(_x * 100.0) + _y;\n"
    "}\n"
    "vec2 UnpackFloat16Into2Floats(float i){\n"
    "    vec2 res;\n"
    "    res.y = i - floor(i);\n"
    "    res.x = (i - res.y) * 0.01;\n"
    "    res.x = (res.x - 0.5) * 2.0;\n"
    "    res.y = (res.y - 0.5) * 2.0;\n"
    "    return res;\n"
    "}\n"
    //currently unused
    /*
    "float Pack2FloatIntoFloat32(float x,float y){\n"
    "    x = clamp(x,0.0001,0.9999);\n"
    "    y = clamp(y,0.0001,0.9999);\n"
    "    float _x = (x + 1.0) * 0.5;\n"
    "    float _y = (y + 1.0) * 0.5;\n"
    "    return floor(_x * 1000.0) + _y;\n"
    "}\n"
    "vec2 UnpackFloat32Into2Floats(float i){\n"
    "    vec2 res;\n"
    "    res.y = i - floor(i);\n"
    "    res.x = (i - res.y) * 0.001;\n"
    "    res.x = (res.x - 0.5) * 2.0;\n"
    "    res.y = (res.y - 0.5) * 2.0;\n"
    "    return res;\n"
    "}\n"
    */
    "\n";
epriv::EShaders::normals_octahedron_compression_functions = epriv::EShaders::constants +
    "vec2 SignNotZero(vec2 v) {\n"
    "    return vec2(v.x >= 0 ? 1.0 : -1.0, v.y >= 0 ? 1.0 : -1.0);\n"
    "}\n"
    "vec2 EncodeOctahedron(vec3 n) {\n"
    "    if(   all(greaterThan(n,ConstantAlmostOneVec3))   )\n"
    "        return ConstantOneVec2;\n" 
    "	 n.xy /= dot(abs(n), ConstantOneVec3);\n"
    "	 return mix(n.xy, (1.0 - abs(n.yx)) * SignNotZero(n.xy), step(n.z, 0.0));\n"
    "}\n"
    "vec3 DecodeOctahedron(vec2 n) {\n"
    "    if(    all(greaterThan(n,ConstantAlmostOneVec2))    )\n"
    "        return ConstantOneVec3;\n"
    "	 vec3 v = vec3(n.xy, 1.0 - abs(n.x) - abs(n.y));\n"
    "	 if (v.z < 0) v.xy = (1.0 - abs(v.yx)) * SignNotZero(v.xy);\n"
    "	 return normalize(v);\n"
    "}\n";

#pragma endregion

#pragma region LightingVertex
epriv::EShaders::lighting_vert = 
    "layout (location = 0) in vec3 position;\n"
    "layout (location = 1) in vec2 uv;\n"
    "\n"
    "uniform mat4 Model;\n"
    "uniform mat4 VP;\n"
    "uniform vec2 VertexShaderData;\n" //x = outercutoff, y = radius
    "uniform float Type;\n" // 2.0 = spot light. 1.0 = any other light. 0.0 = fullscreen quad / triangle
    "uniform vec2 screenSizeDivideBy2;\n"  //x = width/2, y = height/2
    "\n"
    "varying vec2 texcoords;\n"
    "\n"
    "vec3 doSpotLightStuff(vec3 v){\n"
    "    float opposite = tan(VertexShaderData.x * 0.5) * VertexShaderData.y;\n" //outerCutoff might need to be in degrees?
    "    v.xy *= vec2(opposite / VertexShaderData.y);\n" //might need to switch around x,y,z to fit GL's coordinate system
    "    return v;\n"
    "}\n"
    "void main(){\n"
    "    mat4 ModelClone = Model;\n"
    "    vec3 vert = position;\n"
    "    if(Type == 2.0){\n" //spot light
    "        vert = doSpotLightStuff(vert);\n"
    "        ModelClone[3][0] -= CameraRealPosition.x;\n"
    "        ModelClone[3][1] -= CameraRealPosition.y;\n"
    "        ModelClone[3][2] -= CameraRealPosition.z;\n"
    "    }else if(Type == 1.0){\n" //point / rod / etc
    "        ModelClone[3][0] -= CameraRealPosition.x;\n"
    "        ModelClone[3][1] -= CameraRealPosition.y;\n"
    "        ModelClone[3][2] -= CameraRealPosition.z;\n"
    "    }else if(Type == 0.0){\n" //fullscreen quad / triangle
    "        vert.x *= screenSizeDivideBy2.x;\n"
    "        vert.y *= screenSizeDivideBy2.y;\n"
    "    }\n"
    "    texcoords = uv;\n"
    "    gl_Position = VP * ModelClone * vec4(vert,1.0);\n"
    "}";
#pragma endregion

#pragma region BulletPhysicsVertex
epriv::EShaders::bullet_physics_vert = 
    "\n"
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
    "}\n"
    "\n";
#pragma endregion

#pragma region BulletPhysicsFragment
epriv::EShaders::bullet_physcis_frag =
    "\n"
    "in vec3 OutColor;\n"
    "void main(){\n"
    "	gl_FragColor = vec4(OutColor,1.0);\n" 
    "}\n"
    "\n";
#pragma endregion

#pragma region VertexBasic
epriv::EShaders::vertex_basic = 
    "\n"
    "USE_LOG_DEPTH_VERTEX\n"
    "\n"
    "layout (location = 0) in vec3 position;\n"
    //"//layout (location = 1) in float uv;\n"
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
    "varying vec3 TangentCameraPos;\n"
    "varying vec3 TangentFragPos;\n"
    "\n";
epriv::EShaders::vertex_basic += epriv::EShaders::float_into_2_floats;
epriv::EShaders::vertex_basic +=
"void main(){\n"
    "    mat4 ModelClone = Model;\n"
    "    ModelClone[3][0] -= CameraRealPosition.x;\n"
    "    ModelClone[3][1] -= CameraRealPosition.y;\n"
    "    ModelClone[3][2] -= CameraRealPosition.z;\n"
    "    mat4 BoneTransform = mat4(1.0);\n"
    "    if(AnimationPlaying == 1.0){\n"
    "        BoneTransform  = gBones[int(BoneIDs.x)] * Weights.x;\n"
    "        BoneTransform += gBones[int(BoneIDs.y)] * Weights.y;\n"
    "        BoneTransform += gBones[int(BoneIDs.z)] * Weights.z;\n"
    "        BoneTransform += gBones[int(BoneIDs.w)] * Weights.w;\n"
    "    }\n"
    "    vec4 PosTrans =       BoneTransform * vec4(position, 1.0);\n"
    "    vec3 NormalTrans =   (BoneTransform * vec4(normal.zyx, 0.0)).xyz;\n"  //Order is ZYXW so to bring it to XYZ we need to use ZYX
    "    vec3 BinormalTrans = (BoneTransform * vec4(binormal.zyx, 0.0)).xyz;\n"//Order is ZYXW so to bring it to XYZ we need to use ZYX
    "    vec3 TangentTrans =  (BoneTransform * vec4(tangent.zyx, 0.0)).xyz;\n" //Order is ZYXW so to bring it to XYZ we need to use ZYX
    "\n"
    "           Normals = (NormalMatrix * NormalTrans);\n"
    "    vec3 Binormals = (NormalMatrix * BinormalTrans);\n"
    "    vec3  Tangents = (NormalMatrix * TangentTrans);\n"
    "    TBN = (mat3(Tangents,Binormals,Normals));\n"
    "\n"
    "    gl_Position = CameraViewProj * ModelClone * PosTrans;\n"
    "    WorldPosition = (ModelClone * PosTrans).xyz;\n"
    "\n"
    "    CamPosition = CameraPosition;\n"
    "    TangentCameraPos = TBN * CameraPosition;\n"
    "    TangentFragPos = TBN * WorldPosition;\n"
    "\n"
    //"    //UV = UnpackFloat32Into2Floats(uv);\n"
    "    UV = uv;\n"
    "\n"
    "}";
#pragma endregion

#pragma region VertexHUD
epriv::EShaders::vertex_hud =
    "\n"
    "layout (location = 0) in vec3 position;\n"
    //"layout (location = 1) in float uv;\n"
    "layout (location = 1) in vec2 uv;\n"
    "\n"
    "uniform mat4 VP;\n"
    "uniform mat4 Model;\n"
    "varying vec2 UV;\n";
epriv::EShaders::vertex_hud += epriv::EShaders::float_into_2_floats;
epriv::EShaders::vertex_hud +=
    "void main(){\n"
    //"    UV = UnpackFloat32Into2Floats(uv);\n"
    "    UV = uv;\n"
    "    gl_Position = VP * Model * vec4(position, 1.0);\n"
    "}";
#pragma endregion

#pragma region VertexSkybox
epriv::EShaders::vertex_skybox =
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
epriv::EShaders::cubemap_convolude_frag =
    "\n"
    "varying vec3 UV;\n"
    "uniform samplerCube cubemap;\n"
    "const float PI = 3.14159265;\n"
    "void main(){\n"
    "    vec3 N = normalize(UV);\n"
    "    vec3 irradiance = vec3(0.0);\n"
    "    vec3 up = vec3(0.0, 1.0, 0.0);\n"
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
epriv::EShaders::cubemap_prefilter_envmap_frag =
    "varying vec3 UV;\n"
    "uniform samplerCube cubemap;\n"
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
    "float VanDerCorpus(int n, int base){\n"
    "    float invBase = 1.0 / float(base);\n"
    "    float denom   = 1.0;\n"
    "    float result  = 0.0;\n"
    "    for(int i = 0; i < 32; ++i){\n"
    "        if(n > 0){\n"
    "            denom = mod(float(n), 2.0);\n"
    "            result += denom * invBase;\n"
    "            invBase = invBase * 0.5;\n"
    "            n = int(float(n) * 0.5);\n"
    "        }\n"
    "    }\n"
    "    return result;\n"
    "}\n"
    "vec2 Hammersley(int i, int numSamples){\n"
    "    return vec2(float(i)/float(numSamples), VanDerCorpus(i,2));\n"
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
    "        vec2 Xi = Hammersley(i, NUM_SAMPLES);\n"
    "        vec3 L = ImportanceSampleGGX(Xi,TBN);\n"
    "        float NdotL = max(dot(N, L), 0.0);\n"
    "        if(NdotL > 0.0){\n"
    "            float D   = DistributionGGX(L);\n"
    "            float pdf = D * L.z / (4.0 * L.z) + 0.0001;\n"
    "            float saSample = 1.0 / (float(NUM_SAMPLES) * pdf + 0.0001);\n"
    "            float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / PiFourDividedByResSquaredTimesSix);\n"
    "            PreEnv += textureCubeLod(cubemap, L, mipLevel).rgb * NdotL;\n"
    "            totalWeight += NdotL;\n"
    "        }\n"
    "    }\n"
    "    PreEnv = PreEnv / totalWeight;\n"
    "    gl_FragColor = vec4(PreEnv, 1.0);\n"
    "}";
#pragma endregion

#pragma region BRDFPrecompute

epriv::EShaders::brdf_precompute =
    "const float PI2 = 6.283185;\n"
    "uniform int NUM_SAMPLES;\n"
    "varying vec2 texcoords;\n"
    "float VanDerCorpus(int n, int base){\n"
    "    float invBase = 1.0 / float(base);\n"
    "    float denom   = 1.0;\n"
    "    float result  = 0.0;\n"
    "    for(int i = 0; i < 32; ++i){\n"
    "        if(n > 0){\n"
    "            denom = mod(float(n), 2.0);\n"
    "            result += denom * invBase;\n"
    "            invBase = invBase * 0.5;\n"
    "            n = int(float(n) * 0.5);\n"
    "        }\n"
    "    }\n"
    "    return result;\n"
    "}\n"
    "vec2 Hammersley(int i, int N){\n"
    "    return vec2(float(i)/float(N), VanDerCorpus(i,2));\n"
    "}\n"
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
    "        vec2 Xi = Hammersley(i, NUM_SAMPLES);\n"
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

#pragma region FXAA
epriv::EShaders::fxaa_frag =
    "uniform float FXAA_REDUCE_MIN;\n"
    "uniform float FXAA_REDUCE_MUL;\n"
    "uniform float FXAA_SPAN_MAX;\n"
    "uniform sampler2D inTexture;\n"
    //"uniform sampler2D edgeTexture;\n"
    "uniform sampler2D depthTexture;\n"
    "uniform vec2 invRes;\n"
    "varying vec2 texcoords;\n"
    "void main(){\n"
    "   float depth = texture2D(depthTexture,texcoords).r;\n"
    //"   float edge = texture2D(edgeTexture,texcoords).r;\n"
    "   if(depth >= 0.999){\n"
    "       gl_FragColor = texture2D(inTexture, texcoords);\n"
    "       return;\n"
    "   }\n"
    "   vec3 rgbNW = texture2D(inTexture, texcoords + (vec2(-1.0,-1.0)) * invRes).xyz;\n"
    "   vec3 rgbNE = texture2D(inTexture, texcoords + (vec2(1.0,-1.0)) * invRes).xyz;\n"
    "   vec3 rgbSW = texture2D(inTexture, texcoords + (vec2(-1.0,1.0)) * invRes).xyz;\n"
    "   vec3 rgbSE = texture2D(inTexture, texcoords + (vec2(1.0,1.0)) * invRes).xyz;\n"
    "   vec3 rgbM  = texture2D(inTexture, texcoords).xyz;\n"
    "   vec3 luma = vec3(0.299, 0.587, 0.114);\n"
    "   float lumaNW = dot(rgbNW, luma);\n"
    "   float lumaNE = dot(rgbNE, luma);\n"
    "   float lumaSW = dot(rgbSW, luma);\n"
    "   float lumaSE = dot(rgbSE, luma);\n"
    "   float lumaM  = dot(rgbM,  luma);\n"
    "   float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));\n"
    "   float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE))); \n"
    "   vec2 dir;\n"
    "   dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));\n"
    "   dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));\n"
    "   float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),FXAA_REDUCE_MIN);\n"
    "   float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);\n"
    "   dir = min(vec2(FXAA_SPAN_MAX,FXAA_SPAN_MAX),max(vec2(-FXAA_SPAN_MAX,-FXAA_SPAN_MAX),dir * rcpDirMin)) * invRes;\n"
    "   vec3 rgbA = 0.5 * (texture2D(inTexture, texcoords   + dir * (1.0/3.0 - 0.5)).xyz + texture2D(inTexture,texcoords + dir * (2.0/3.0 - 0.5)).xyz);\n"
    "   vec3 rgbB = rgbA * 0.5 + 0.25 * (texture2D(inTexture,texcoords + dir * - 0.5).xyz + texture2D(inTexture,texcoords + dir * 0.5).xyz);\n"
    "   float lumaB = dot(rgbB,luma);\n"
    "   if((lumaB < lumaMin) || (lumaB > lumaMax)){\n"
    "       gl_FragColor = vec4(rgbA,1.0);\n"
    "   }else{\n"
    "       gl_FragColor = vec4(rgbB,1.0);\n"
    "   }\n"
    "}";
#pragma endregion

#pragma region LightingStencilPass

epriv::EShaders::stencil_passover = epriv::EShaders::normals_octahedron_compression_functions +
    "\n"
    "const vec3 comparison = vec3(1.0,1.0,1.0);\n"
    "uniform sampler2D gNormalMap;\n"
    "varying vec2 texcoords;\n"
    "void main(){\n"
    "    vec3 normal = DecodeOctahedron(texture2D(gNormalMap,texcoords).rg);\n"
    "    if(distance(normal,comparison) < 0.01){\n"
    "        discard;\n"//this is where the magic happens with the stencil buffer.
    "    }\n"
    "}";

#pragma endregion

#pragma region FullscreenQuadVertex
epriv::EShaders::fullscreen_quad_vertex = 
    "\n"
    "layout (location = 0) in vec3 position;\n"
    "layout (location = 1) in vec2 uv;\n"
    "\n"
    "uniform mat4 Model;\n"
    "uniform mat4 VP;\n"
    "uniform vec2 screenSizeDivideBy2;\n"  //x = width/2, y = height/2
    "\n"
    "varying vec2 texcoords;\n"
    "\n"
    "void main(){\n"
    "    vec3 vert = position;\n"
    "    vert.x *= screenSizeDivideBy2.x;\n"
    "    vert.y *= screenSizeDivideBy2.y;\n"
    "    texcoords = uv;\n"
    "    gl_Position = VP * Model * vec4(vert,1.0);\n"
    "}";
#pragma endregion

#pragma region DepthOfField
epriv::EShaders::depth_of_field =
    "\n"
    "const float weight[4] = float[](1.0,0.9,0.7,0.4);\n"
    "\n"
    "uniform sampler2D inTexture;\n"
    "uniform sampler2D depthTexture;\n"
    "\n"
    "uniform vec4 Data;\n" //x = blurClamp, y = bias, z = focus, w = aspectRatio
    "\n"
    "varying vec2 texcoords;\n"
    "void main(){\n"
    "    vec2 aspectcorrect = vec2(1.0, Data.w);\n"
    "    float depth = texture2D(depthTexture, texcoords).r;\n"
    "    float factor = (depth - Data.z);\n"
    "    vec2 dofblur = vec2(clamp(factor * Data.y, -Data.x, Data.x));\n"
    "    vec4 col = vec4(0.0);\n"
    "    col += texture2D(inTexture, texcoords);\n"
    "    col += texture2D(inTexture, texcoords + (vec2(0.0, 0.4)*aspectcorrect)     * dofblur);\n"
    "    col += texture2D(inTexture, texcoords + (vec2(0.0, -0.4)*aspectcorrect)    * dofblur);\n"
    "    col += texture2D(inTexture, texcoords + (vec2(0.4, 0.0)*aspectcorrect)     * dofblur);\n"
    "    col += texture2D(inTexture, texcoords + (vec2(-0.4, 0.0)*aspectcorrect)    * dofblur);\n"
    "    col += texture2D(inTexture, texcoords + (vec2(0.29, 0.29)*aspectcorrect)   * dofblur);\n"
    "    col += texture2D(inTexture, texcoords + (vec2(-0.29, 0.29)*aspectcorrect)  * dofblur);\n"
    "    col += texture2D(inTexture, texcoords + (vec2(0.29, -0.29)*aspectcorrect)  * dofblur);\n"
    "    col += texture2D(inTexture, texcoords + (vec2(-0.29, -0.29)*aspectcorrect) * dofblur);\n"
    "    for (int i = 0; i < 2; ++i) {\n"
    "        int k = i+2;\n"
    "        col += texture2D(inTexture, texcoords + (vec2(0.15, 0.37)*aspectcorrect)   * dofblur * weight[i]);\n"
    "        col += texture2D(inTexture, texcoords + (vec2(-0.15, -0.37)*aspectcorrect) * dofblur * weight[i]);\n"
    "        col += texture2D(inTexture, texcoords + (vec2(-0.15, 0.37)*aspectcorrect)  * dofblur * weight[i]);\n"
    "        col += texture2D(inTexture, texcoords + (vec2(0.15, -0.37)*aspectcorrect)  * dofblur * weight[i]);\n"
    "        col += texture2D(inTexture, texcoords + (vec2(-0.37, 0.15)*aspectcorrect)  * dofblur * weight[i]);\n"
    "        col += texture2D(inTexture, texcoords + (vec2(0.37, -0.15)*aspectcorrect)  * dofblur * weight[i]);\n"
    "        col += texture2D(inTexture, texcoords + (vec2(0.37, 0.15)*aspectcorrect)   * dofblur * weight[i]);\n"
    "        col += texture2D(inTexture, texcoords + (vec2(-0.37, -0.15)*aspectcorrect) * dofblur * weight[i]);\n"
    "\n"
    "        col += texture2D(inTexture, texcoords + (vec2(0.29, 0.29)*aspectcorrect)   * dofblur * weight[k]);\n"
    "        col += texture2D(inTexture, texcoords + (vec2(0.4, 0.0)*aspectcorrect)     * dofblur * weight[k]);\n"
    "        col += texture2D(inTexture, texcoords + (vec2(0.29, -0.29)*aspectcorrect)  * dofblur * weight[k]);\n"
    "        col += texture2D(inTexture, texcoords + (vec2(0.0, -0.4)*aspectcorrect)    * dofblur * weight[k]);\n"
    "        col += texture2D(inTexture, texcoords + (vec2(-0.29, 0.29)*aspectcorrect)  * dofblur * weight[k]);\n"
    "        col += texture2D(inTexture, texcoords + (vec2(-0.4, 0.0)*aspectcorrect)    * dofblur * weight[k]);\n"
    "        col += texture2D(inTexture, texcoords + (vec2(-0.29, -0.29)*aspectcorrect) * dofblur * weight[k]);\n"
    "        col += texture2D(inTexture, texcoords + (vec2(0.0, 0.4)*aspectcorrect)     * dofblur * weight[k]);\n"
    "    }\n"
    "    gl_FragColor.rgb = col.rgb * 0.02439; \n" //0.02439 = 1.0 / 41.0
    "    gl_FragColor.a = 1.0; \n"
    "}\n"
    "\n";
#pragma endregion

#pragma region SMAA
   
epriv::EShaders::smaa_common = 
    "vec4 mad(vec4 a, vec4 b, vec4 c){ return (a * b) + c; }\n"
    "vec3 mad(vec3 a, vec3 b, vec3 c){ return (a * b) + c; }\n"
    "vec2 mad(vec2 a, vec2 b, vec2 c){ return (a * b) + c; }\n"
    "float mad(float a, float b, float c){ return (a * b) + c; }\n"
    "vec4 saturate(vec4 a){ return clamp(a,0.0,1.0); }\n"
    "vec3 saturate(vec3 a){ return clamp(a,0.0,1.0); }\n"
    "vec2 saturate(vec2 a){ return clamp(a,0.0,1.0); }\n"
    "float saturate(float a){ return clamp(a,0.0,1.0); }\n"
    "vec4 round(vec4 a){ return floor(a + vec4(0.5)); }\n"
    "vec3 round(vec3 a){ return floor(a + vec3(0.5)); }\n"
    "vec2 round(vec2 a){ return floor(a + vec2(0.5)); }\n"
    "float round(float a){ return floor(a + 0.5); }\n"
    "void SMAAMovc(bvec2 cond, inout vec2 variable, vec2 value) {\n"
    "    if (cond.x) variable.x = value.x;\n"
    "    if (cond.y) variable.y = value.y;\n"
    "}\n"
    "void SMAAMovc(bvec4 cond, inout vec4 variable, vec4 value) {\n"
    "    SMAAMovc(cond.xy, variable.xy, value.xy);\n"
    "    SMAAMovc(cond.zw, variable.zw, value.zw);\n"
    "}\n"
    "\n"
    "float API_V_DIR(float v){ return -v; }\n"
    //"float API_V_COORD(float v){ return 1.0 - v; }\n"
    "bool API_V_BELOW(float v1, float v2){ if(v1 < v2) return true; return false; }\n"
    "bool API_V_ABOVE(float v1, float v2){ if(v1 > v2) return true; return false; }\n"
    "\n"
    //"float API_V_DIR(float v){ return v; }\n"
    "float API_V_COORD(float v){ return v; }\n"
    //"bool API_V_BELOW(float v1, float v2){ if(v1 > v2) return true; return false; }\n"
    //"bool API_V_ABOVE(float v1, float v2){ if(v1 < v2) return true; return false; }\n"
    "\n";
 
epriv::EShaders::smaa_frag_1_stencil = epriv::EShaders::smaa_common +
    "\n"//edge frag
    "const vec2 comparison = vec2(1.0,1.0);\n"
    "\n"
    "uniform sampler2D textureMap;\n"
    "\n"
    "uniform vec2 SMAAInfo0Floats;\n" //SMAA_THRESHOLD,SMAA_DEPTH_THRESHOLD
    "\n"
    "varying vec2 uv;\n"
    "varying vec4 _offset[3];\n"
    "\n"
    "vec3 SMAAGatherNeighbours(vec2 texcoord,vec4 offset[3],sampler2D tex) {\n"
    "    float P = texture2D(tex, texcoord).r;\n"
    "    float Pleft = texture2D(tex, offset[0].xy).r;\n"
    "    float Ptop  = texture2D(tex, offset[0].zw).r;\n"
    "    return vec3(P, Pleft, Ptop);\n"
    "}\n"
    "void SMAADepthEdgeDetectionPS(vec2 texcoord,vec4 offset[3],sampler2D depthTex) {\n"
    "    vec3 neighbours = SMAAGatherNeighbours(texcoord, offset, depthTex);\n"
    "    vec2 delta = abs(neighbours.xx - vec2(neighbours.y, neighbours.z));\n"
    "    vec2 edges = step(SMAAInfo0Floats.y, delta);\n"
    "    if (dot(edges, comparison) == 0.0){\n"
    "        discard;\n"
    "    }\n"
    "}\n"
    "void SMAAColorEdgeDetectionPS(vec2 texcoord,vec4 offset[3],sampler2D colorTex){\n"
    "    vec2 threshold = vec2(SMAAInfo0Floats.x, SMAAInfo0Floats.x);\n"
    "    vec4 delta;\n"
    "    vec3 C = texture2D(colorTex, texcoord).rgb;\n"
    "    vec3 Cleft = texture2D(colorTex, offset[0].xy).rgb;\n"
    "    vec3 t = abs(C - Cleft);\n"
    "    delta.x = max(max(t.r, t.g), t.b);\n"
    "    vec3 Ctop  = texture2D(colorTex, offset[0].zw).rgb;\n"
    "    t = abs(C - Ctop);\n"
    "    delta.y = max(max(t.r, t.g), t.b);\n"
    "    vec2 edges = step(threshold, delta.xy);\n"
    "    if (dot(edges, comparison) == 0.0){\n"
    "        discard;\n"
    "    }\n"
    "}\n"
    "void SMAALumaEdgeDetectionPS(vec2 texcoord,vec4 offset[3],sampler2D colorTex) {\n"
    "    vec2 threshold = vec2(SMAAInfo0Floats.x, SMAAInfo0Floats.x);\n"
    "    vec3 weights = vec3(0.2126, 0.7152, 0.0722);\n"
    "    float L =     dot(texture2D(colorTex, texcoord).rgb,     weights);\n"
    "    float Lleft = dot(texture2D(colorTex, offset[0].xy).rgb, weights);\n"
    "    float Ltop  = dot(texture2D(colorTex, offset[0].zw).rgb, weights);\n"
    "    vec4 delta;\n"
    "    delta.xy = abs(L - vec2(Lleft, Ltop));\n"
    "    vec2 edges = step(threshold, delta.xy);\n"
    "    if (dot(edges, comparison) == 0.0){\n"
    "        discard;\n"
    "    }\n"
    "}\n"
    "void main(){\n"
    "    SMAAColorEdgeDetectionPS(uv, _offset, textureMap);\n"
    //"    SMAADepthEdgeDetectionPS(uv, _offset, textureMap);\n"
    //"    SMAALumaEdgeDetectionPS(uv, _offset, textureMap);\n"
    "}\n"
    "\n";


epriv::EShaders::smaa_vertex_1 = epriv::EShaders::smaa_common +
    "\n"//edge vert
    "layout (location = 0) in vec3 position;\n"
    "layout (location = 1) in vec2 UV;\n"
    "\n"
    "uniform vec4 SMAA_PIXEL_SIZE;\n" //make this globally inherit for all smaa shaders
    "uniform mat4 MVP;\n"
    "uniform vec2 screenSizeDivideBy2;\n"
    "\n"
    "varying vec2 uv;\n"
    "varying vec4 _offset[3];\n"
    "\n"
    "void main(){\n"
    "    uv = UV;\n"
    "    vec3 vert = position;\n"
    "    vert.x *= screenSizeDivideBy2.x;\n"
    "    vert.y *= screenSizeDivideBy2.y;\n"
    "    _offset[0] = mad(SMAA_PIXEL_SIZE.xyxy,vec4(-1.0, 0.0, 0.0, API_V_DIR(-1.0)),uv.xyxy);\n"
    "    _offset[1] = mad(SMAA_PIXEL_SIZE.xyxy,vec4( 1.0, 0.0, 0.0,  API_V_DIR(1.0)),uv.xyxy);\n"
    "    _offset[2] = mad(SMAA_PIXEL_SIZE.xyxy,vec4(-2.0, 0.0, 0.0, API_V_DIR(-2.0)),uv.xyxy);\n"
    "    gl_Position = MVP * vec4(vert,1.0);\n"
    "}\n";  
    
epriv::EShaders::smaa_frag_1 = epriv::EShaders::smaa_common +
    "\n"//edge frag
    "\n"
    "uniform int SMAA_PREDICATION;\n"
    "const vec2 comparison = vec2(1.0,1.0);\n"
    "\n"
    "uniform sampler2D textureMap;\n"
    "uniform sampler2D texturePredication;\n"
    "\n"
    "uniform vec4 SMAAInfo1Floats;\n" //SMAA_THRESHOLD,SMAA_DEPTH_THRESHOLD,SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR,SMAA_PREDICATION_THRESHOLD
    "uniform vec4 SMAAInfo1FloatsA;\n" //SMAA_PREDICATION_SCALE,SMAA_PREDICATION_STRENGTH
    "\n"
    "varying vec2 uv;\n"
    "varying vec4 _offset[3];\n"
    "\n"
    "vec3 SMAAGatherNeighbours(vec2 texcoord,vec4 offset[3],sampler2D tex) {\n"
    "    float P = texture2D(tex, texcoord).r;\n"
    "    float Pleft = texture2D(tex, offset[0].xy).r;\n"
    "    float Ptop  = texture2D(tex, offset[0].zw).r;\n"
    "    return vec3(P, Pleft, Ptop);\n"
    "}\n"
    "vec2 SMAACalculatePredicatedThreshold(vec2 texcoord,vec4 offset[3]){\n"
    "    vec3 neighbours = SMAAGatherNeighbours(texcoord, offset,texturePredication);\n"
    "    vec2 delta = abs(neighbours.xx - neighbours.yz);\n"
    "    vec2 edges = step(SMAAInfo1Floats.w, delta);\n"
    "    return SMAAInfo1FloatsA.x * SMAAInfo1Floats.x * (1.0 - SMAAInfo1FloatsA.y * edges);\n"
    "}\n"
    "vec2 SMAADepthEdgeDetectionPS(vec2 texcoord,vec4 offset[3],sampler2D depthTex) {\n"
    "    vec3 neighbours = SMAAGatherNeighbours(texcoord, offset, depthTex);\n"
    "    vec2 delta = abs(neighbours.xx - vec2(neighbours.y, neighbours.z));\n"
    "    vec2 edges = step(SMAAInfo1Floats.y, delta);\n"
    "    if (dot(edges, comparison) == 0.0)\n"
    "        discard;\n"
    "    return edges;\n"
    "}\n"
    "vec2 SMAAColorEdgeDetectionPS(vec2 texcoord,vec4 offset[3],sampler2D colorTex){\n"
    "    vec2 threshold;\n"
    "    if(SMAA_PREDICATION == 1){\n"
    "        threshold = SMAACalculatePredicatedThreshold(texcoord, offset);\n"
    "    }else{\n"
    "        threshold = vec2(SMAAInfo1Floats.x, SMAAInfo1Floats.x);\n"
    "    }\n"
    "    vec4 delta;\n"
    "    vec3 C = texture2D(colorTex, texcoord).rgb;\n"
    "    vec3 Cleft = texture2D(colorTex, offset[0].xy).rgb;\n"
    "    vec3 t = abs(C - Cleft);\n"
    "    delta.x = max(max(t.r, t.g), t.b);\n"
    "    vec3 Ctop  = texture2D(colorTex, offset[0].zw).rgb;\n"
    "    t = abs(C - Ctop);\n"
    "    delta.y = max(max(t.r, t.g), t.b);\n"
    "    vec2 edges = step(threshold, delta.xy);\n"
    "    if (dot(edges, comparison) == 0.0)\n"
    "        discard;\n"
    "    vec3 Cright = texture2D(colorTex, offset[1].xy).rgb;\n"
    "    t = abs(C - Cright);\n"
    "    delta.z = max(max(t.r, t.g), t.b);\n"
    "    vec3 Cbottom  = texture2D(colorTex, offset[1].zw).rgb;\n"
    "    t = abs(C - Cbottom);\n"
    "    delta.w = max(max(t.r, t.g), t.b);\n"
    "    vec2 maxDelta = max(delta.xy, delta.zw);\n"
    "    vec3 Cleftleft  = texture2D(colorTex, offset[2].xy).rgb;\n"
    "    t = abs(C - Cleftleft);\n"
    "    delta.z = max(max(t.r, t.g), t.b);\n"
    "    vec3 Ctoptop = texture2D(colorTex, offset[2].zw).rgb;\n"
    "    t = abs(C - Ctoptop);\n"
    "    delta.w = max(max(t.r, t.g), t.b);\n"
    "    maxDelta = max(maxDelta.xy, delta.zw);\n"
    "    float finalDelta = max(maxDelta.x, maxDelta.y);\n"
    "    edges.xy *= step((finalDelta), (SMAAInfo1Floats.z) * delta.xy);\n" //do we need this line in opengl?
    "    return edges;\n"
    "}\n"
    "vec2 SMAALumaEdgeDetectionPS(vec2 texcoord,vec4 offset[3],sampler2D colorTex) {\n"
    "    vec2 threshold;\n"
    "    if(SMAA_PREDICATION == 1){\n"
    "        threshold = SMAACalculatePredicatedThreshold(texcoord, offset);\n"
    "    }else{\n"
    "        threshold = vec2(SMAAInfo1Floats.x, SMAAInfo1Floats.x);\n"
    "    }\n"
    "    vec3 weights = vec3(0.2126, 0.7152, 0.0722);\n"
    "    float L =     dot(texture2D(colorTex, texcoord).rgb,     weights);\n"
    "    float Lleft = dot(texture2D(colorTex, offset[0].xy).rgb, weights);\n"
    "    float Ltop  = dot(texture2D(colorTex, offset[0].zw).rgb, weights);\n"
    "    vec4 delta;\n"
    "    delta.xy = abs(L - vec2(Lleft, Ltop));\n"
    "    vec2 edges = step(threshold, delta.xy);\n"
    "    if (dot(edges, comparison) == 0.0)\n"
    "        discard;\n"
    "    float Lright = dot(texture2D(colorTex, offset[1].xy).rgb, weights);\n"
    "    float Lbottom  = dot(texture2D(colorTex, offset[1].zw).rgb, weights);\n"
    "    delta.zw = abs(L - vec2(Lright, Lbottom));\n"
    "    vec2 maxDelta = max(delta.xy, delta.zw);\n"
    "    float Lleftleft = dot(texture2D(colorTex, offset[2].xy).rgb, weights);\n"
    "    float Ltoptop = dot(texture2D(colorTex, offset[2].zw).rgb, weights);\n"
    "    delta.zw = abs(vec2(Lleft, Ltop) - vec2(Lleftleft, Ltoptop));\n"
    "    maxDelta = max(maxDelta.xy, delta.zw);\n"
    "    float finalDelta = max(maxDelta.x, maxDelta.y);\n"
    "    edges.xy *= step((finalDelta), (SMAAInfo1Floats.z) * delta.xy);\n" //do we need this line in opengl?
    "    return edges;\n"
    "}\n"
    "void main(){\n"
    "    gl_FragColor = vec4(SMAAColorEdgeDetectionPS(uv, _offset, textureMap),0.0,1.0);\n"
    //"    gl_FragColor = vec4(SMAADepthEdgeDetectionPS(uv, _offset, textureMap),0.0,1.0);\n"
    //"    gl_FragColor = vec4(SMAALumaEdgeDetectionPS(uv, _offset, textureMap),0.0,1.0);\n"
    "}\n";
epriv::EShaders::smaa_vertex_2 = epriv::EShaders::smaa_common +
    "\n"//blend vert
    "layout (location = 0) in vec3 position;\n"
    "layout (location = 1) in vec2 UV;\n"
    "\n"
    "uniform mat4 MVP;\n"
    "uniform vec4 SMAA_PIXEL_SIZE;\n" //make this globally inherit for all smaa shaders
    "uniform int SMAA_MAX_SEARCH_STEPS;\n" //make this globally inherit for all smaa shaders
    "uniform vec2 screenSizeDivideBy2;\n"
    "\n"
    "varying vec2 uv;\n"
    "varying vec2 pixCoord;\n"
    "varying vec4 _offset[3];\n"
    "\n"
    "flat varying vec4 _SMAA_PIXEL_SIZE;\n"
    "\n"
    "void main(){\n"
    "    uv = UV;\n"
    "    vec3 vert = position;\n"
    "    vert.x *= screenSizeDivideBy2.x;\n"
    "    vert.y *= screenSizeDivideBy2.y;\n"
    "    pixCoord = uv * SMAA_PIXEL_SIZE.zw;\n"
    "    _offset[0] = mad(SMAA_PIXEL_SIZE.xyxy,vec4(-0.25,API_V_DIR(-0.125), 1.25,API_V_DIR(-0.125)),uv.xyxy);\n"
    "    _offset[1] = mad(SMAA_PIXEL_SIZE.xyxy,vec4(-0.125,API_V_DIR(-0.25),-0.125,API_V_DIR(1.25)),uv.xyxy);\n"
    "    _offset[2] = mad(SMAA_PIXEL_SIZE.xxyy,vec4(-2.0, 2.0, API_V_DIR(-2.0), API_V_DIR(2.0)) * float(SMAA_MAX_SEARCH_STEPS),vec4(_offset[0].xz, _offset[1].yw));\n"
    "    gl_Position = MVP * vec4(vert,1.0);\n"
    "    _SMAA_PIXEL_SIZE = SMAA_PIXEL_SIZE;\n"
    "}\n";
epriv::EShaders::smaa_frag_2 = epriv::EShaders::smaa_common +
    "\n"
    "\n"//blend frag
    "uniform sampler2D edge_tex;\n"
    "uniform sampler2D area_tex;\n"
    "uniform sampler2D search_tex;\n"
    "\n"
    "varying vec2 uv;\n"
    "varying vec2 pixCoord;\n"
    "varying vec4 _offset[3];\n"
    "flat varying vec4 _SMAA_PIXEL_SIZE;\n"
    "\n"
    "uniform ivec4 SMAAInfo2Ints;\n" //x = MAX_SEARCH_STEPS_DIAG, y = AREATEX_MAX_DISTANCE, z = AREATEX_MAX_DISTANCE_DIAG, w = CORNER_ROUNDING
    "uniform vec4 SMAAInfo2Floats;\n" //x, y = SMAA_AREATEX_PIXEL_SIZE(x & y), z = SMAA_AREATEX_SUBTEX_SIZE, w = SMAA_CORNER_ROUNDING_NORM
    "\n"
    "float SMAASearchLength(sampler2D searchTex, vec2 e, float offset) {\n"
    "    vec2 scale = vec2(66.0, 33.0) * vec2(0.5, -1.0);\n"
    "    vec2 bias = vec2(66.0, 33.0) * vec2(offset, 1.0);\n"
    "    scale += vec2(-1.0,  1.0);\n"
    "    bias  += vec2( 0.5, -0.5);\n"
    "    scale *= 1.0 / vec2(64.0, 16.0);\n"
    "    bias *= 1.0 / vec2(64.0, 16.0);\n"
    "    vec2 coord = mad(scale, e, bias);\n"
    "    coord.y = API_V_COORD(coord.y);\n"
    "    return (texture2D(searchTex, coord)).r;\n"
    "}\n"
    "float SMAASearchXLeft(sampler2D edgesTex, sampler2D searchTex, vec2 texcoord, float end) {\n"
    "    vec2 e = vec2(0.0, 1.0);\n"
    "    while (texcoord.x > end && e.g > 0.8281 && e.r == 0.0) {\n"
    "        e = texture2D(edgesTex, texcoord).rg;\n"
    "        texcoord = mad(-vec2(2.0, 0.0), _SMAA_PIXEL_SIZE.xy, texcoord);\n"
    "    }\n"
    "    float offset = mad(-(255.0 / 127.0), SMAASearchLength(searchTex, e, 0.0), 3.25);\n"
    "    return mad(_SMAA_PIXEL_SIZE.x, offset, texcoord.x);\n"
    "}\n"
    "float SMAASearchXRight(sampler2D edgesTex, sampler2D searchTex, vec2 texcoord, float end) {\n"
    "    vec2 e = vec2(0.0, 1.0);\n"
    "    while (texcoord.x < end && e.g > 0.8281 && e.r == 0.0) {\n"
    "        e = texture2D(edgesTex, texcoord).rg;\n"
    "        texcoord = mad(vec2(2.0, 0.0), _SMAA_PIXEL_SIZE.xy, texcoord);\n"
    "    }\n"
    "    float offset = mad(-(255.0 / 127.0), SMAASearchLength(searchTex, e, 0.5), 3.25);\n"
    "    return mad(-_SMAA_PIXEL_SIZE.x, offset, texcoord.x);\n"
    "}\n"
    "float SMAASearchYUp(sampler2D edgesTex, sampler2D searchTex, vec2 texcoord, float end) {\n"
    "    vec2 e = vec2(1.0, 0.0);\n"
    "    while (API_V_BELOW(texcoord.y,end) && e.r > 0.8281 && e.g == 0.0) {\n"
    "        e = texture2D(edgesTex, texcoord).rg;\n"
    "        texcoord = mad(-vec2(0.0, API_V_DIR(2.0)), _SMAA_PIXEL_SIZE.xy, texcoord);\n"
    "    }\n"
    "    float offset = mad(-(255.0 / 127.0), SMAASearchLength(searchTex, e.gr, 0.0), 3.25);\n"
    "    return mad(_SMAA_PIXEL_SIZE.y, API_V_DIR(offset), texcoord.y);\n"
    "}\n"
    "float SMAASearchYDown(sampler2D edgesTex,sampler2D searchTex, vec2 texcoord, float end) {\n"
    "    vec2 e = vec2(1.0, 0.0);\n"
    "    while (API_V_ABOVE(texcoord.y,end) && e.r > 0.8281 && e.g == 0.0) {\n"
    "        e = texture2D(edgesTex, texcoord).rg;\n"
    "        texcoord = mad(vec2(0.0, API_V_DIR(2.0)), _SMAA_PIXEL_SIZE.xy, texcoord);\n"
    "    }\n"
    "    float offset = mad(-(255.0 / 127.0), SMAASearchLength(searchTex, e.gr, 0.5), 3.25);\n"
    "    return mad(-_SMAA_PIXEL_SIZE.y, API_V_DIR(offset), texcoord.y);\n"
    "}\n"
    "vec2 SMAAAreaDiag(sampler2D areaTex, vec2 dist, vec2 e, float offset) {\n"
    "    vec2 texcoord = mad(vec2(SMAAInfo2Ints.z, SMAAInfo2Ints.z), e, dist);\n"
    "    texcoord = mad(SMAAInfo2Floats.xy, texcoord, 0.5 * SMAAInfo2Floats.xy);\n"
    "    texcoord.x += 0.5;\n"
    "    texcoord.y += SMAAInfo2Floats.z * offset;\n"
    "    texcoord.y = API_V_COORD(texcoord.y);\n"
    "    return (texture2D(areaTex, texcoord)).rg;\n"
    "}\n"
    "vec2 SMAADecodeDiagBilinearAccess(vec2 e) {\n"
    "    e.r = e.r * abs(5.0 * e.r - 5.0 * 0.75);\n"
    "    return round(e);\n"
    "}\n"
    "vec4 SMAADecodeDiagBilinearAccess(vec4 e) {\n"
    "    e.rb = e.rb * abs(5.0 * e.rb - 5.0 * 0.75);\n"
    "    return round(e);\n"
    "}\n"
    "vec2 SMAASearchDiag1(sampler2D edgesTex,vec2 texcoord,vec2 dir,out vec2 e) {\n"
    "    dir.y = API_V_DIR(dir.y);\n"
    "    vec4 coord = vec4(texcoord, -1.0, 1.0);\n"
    "    vec3 t = vec3(_SMAA_PIXEL_SIZE.xy, 1.0);\n"
    "    while (coord.z < float(SMAAInfo2Ints.x - 1) && coord.w > 0.9) {\n"
    "        coord.xyz = mad(t, vec3(dir, 1.0), coord.xyz);\n"
    "        e = texture2D(edgesTex, coord.xy).rg;\n"
    "        coord.w = dot(e, vec2(0.5));\n"
    "    }\n"
    "    return coord.zw;\n"
    "}\n"
    "vec2 SMAASearchDiag2(sampler2D edgesTex,vec2 texcoord,vec2 dir,out vec2 e) {\n"
    "    dir.y = API_V_DIR(dir.y);\n"
    "    vec4 coord = vec4(texcoord, -1.0, 1.0);\n"
    "    coord.x += 0.25 * _SMAA_PIXEL_SIZE.x;\n" // See @SearchDiag2Optimization
    "    vec3 t = vec3(_SMAA_PIXEL_SIZE.xy, 1.0);\n"
    "    while (coord.z < float(SMAAInfo2Ints.x - 1) && coord.w > 0.9) {\n"
    "        coord.xyz = mad(t, vec3(dir, 1.0), coord.xyz);\n"
    "        e = texture2D(edgesTex, coord.xy).rg;\n"
    "        e = SMAADecodeDiagBilinearAccess(e);\n"
    "        coord.w = dot(e, vec2(0.5));\n"
    "    }\n"
    "    return coord.zw;\n"
    "}\n"
    "vec2 SMAACalculateDiagWeights(sampler2D edgesTex,sampler2D areaTex,vec2 texcoord,vec2 e,vec4 subsampleIndices){\n"
    "    vec2 weights = vec2(0.0);\n"
    "    vec4 d;\n"
    "    vec2 end;\n"
    "    if (e.r > 0.0) {\n"
    "        d.xz = SMAASearchDiag1(edgesTex,texcoord,vec2(-1.0,1.0),end);\n"
    "        d.x += float(end.y > 0.9);\n"
    "    }else\n"
    "        d.xz = vec2(0.0);\n"
    "    d.yw = SMAASearchDiag1(edgesTex,texcoord,vec2(1.0,-1.0),end);\n"
    "    if (d.x + d.y > 2.0) {\n" // d.x + d.y + 1 > 3
    "        vec4 coords = mad(vec4(-d.x + 0.25, API_V_DIR(d.x), d.y, API_V_DIR(-d.y - 0.25)), _SMAA_PIXEL_SIZE.xyxy, texcoord.xyxy);\n"
    "        vec4 c;\n"
    "        c.xy = texture2D(edgesTex, coords.xy + vec2(-1.0, 0.0) * _SMAA_PIXEL_SIZE.xy).rg;\n"
    "        c.zw = texture2D(edgesTex, coords.zw + vec2( 1.0, 0.0) * _SMAA_PIXEL_SIZE.xy).rg;\n"
    "        c.yxwz = SMAADecodeDiagBilinearAccess(c.xyzw);\n"
    "        vec2 cc = mad(vec2(2.0), c.xz, c.yw);\n"
    "        SMAAMovc(bvec2(step(0.9, d.zw)), cc, vec2(0.0));\n"
    "        weights += SMAAAreaDiag(areaTex, d.xy, cc, subsampleIndices.z);\n"
    "    }\n"
    "    d.xz = SMAASearchDiag2(edgesTex,texcoord,vec2(-1.0,-1.0), end);\n"
    "    if (texture2D(edgesTex, texcoord + vec2(1.0, 0.0)*_SMAA_PIXEL_SIZE.xy).r > 0.0) {\n"
    "        d.yw = SMAASearchDiag2(edgesTex, texcoord, vec2(1.0), end);\n"
    "        d.y += float(end.y > 0.9);\n"
    "    }else\n"
    "        d.yw = vec2(0.0);\n"
    "    if (d.x + d.y > 2.0) {\n" // d.x + d.y + 1 > 3
    "        vec4 coords = mad(vec4(-d.x, API_V_DIR(-d.x), d.y, API_V_DIR(d.y)), _SMAA_PIXEL_SIZE.xyxy, texcoord.xyxy);\n"
    "        vec4 c;\n"
    "        c.x  = texture2D(edgesTex, coords.xy + vec2(-1.0, 0.0)*_SMAA_PIXEL_SIZE.xy).g;\n"
    "        c.y  = texture2D(edgesTex, coords.xy + vec2( 0.0, API_V_DIR(-1.0))*_SMAA_PIXEL_SIZE.xy).r;\n"
    "        c.zw = texture2D(edgesTex, coords.zw + vec2( 1.0, 0.0)*_SMAA_PIXEL_SIZE.xy).gr;\n"
    "        vec2 cc = mad(vec2(2.0), c.xz, c.yw);\n"
    "        SMAAMovc(bvec2(step(0.9, d.zw)), cc, vec2(0.0));\n"
    "        weights += SMAAAreaDiag(areaTex, d.xy, cc, subsampleIndices.w).gr;\n"
    "    }\n"
    "    return weights;\n"
    "}\n"
    "vec2 SMAAArea(sampler2D areaTex, vec2 dist, float e1, float e2, float offset) {\n"
    "    vec2 texcoord = mad(vec2(SMAAInfo2Ints.y, SMAAInfo2Ints.y), round(4.0 * vec2(e1, e2)), dist);\n"
    "    texcoord = mad(SMAAInfo2Floats.xy, texcoord, 0.5 * SMAAInfo2Floats.xy);\n"
    "    texcoord.y = mad(SMAAInfo2Floats.z, offset, texcoord.y);\n"
    "    texcoord.y = API_V_COORD(texcoord.y);\n"
    "    return (texture2D(areaTex, texcoord)).rg;\n"
    "}\n"
    "void SMAADetectHorizontalCornerPattern(sampler2D edgesTex, inout vec2 weights, vec4 texcoord, vec2 d) {\n"
    "    if(SMAAInfo2Ints.w == 0) return;\n"
    "    vec2 leftRight = step(d.xy, d.yx);\n"
    "    vec2 rounding = (1.0 - SMAAInfo2Floats.w) * leftRight;\n"
    "    rounding /= leftRight.x + leftRight.y;\n"
    "    vec2 factor = vec2(1.0, 1.0);\n"
    "    factor.x -= rounding.x * texture2D(edgesTex, texcoord.xy + vec2(0.0,  API_V_DIR(1.0))*_SMAA_PIXEL_SIZE.xy).r;\n"
    "    factor.x -= rounding.y * texture2D(edgesTex, texcoord.zw + vec2(1.0,  API_V_DIR(1.0))*_SMAA_PIXEL_SIZE.xy).r;\n"
    "    factor.y -= rounding.x * texture2D(edgesTex, texcoord.xy + vec2(0.0, API_V_DIR(-2.0))*_SMAA_PIXEL_SIZE.xy).r;\n"
    "    factor.y -= rounding.y * texture2D(edgesTex, texcoord.zw + vec2(1.0, API_V_DIR(-2.0))*_SMAA_PIXEL_SIZE.xy).r;\n"
    "    weights *= saturate(factor);\n"
    "}\n"
    "void SMAADetectVerticalCornerPattern(sampler2D edgesTex, inout vec2 weights, vec4 texcoord, vec2 d) {\n"
    "    if(SMAAInfo2Ints.w == 0) return;\n"
    "    vec2 leftRight = step(d.xy, d.yx);\n"
    "    vec2 rounding = (1.0 - SMAAInfo2Floats.w) * leftRight;\n"
    "    rounding /= leftRight.x + leftRight.y;\n"
    "    vec2 factor = vec2(1.0, 1.0);\n"
    "    factor.x -= rounding.x * texture2D(edgesTex, texcoord.xy + vec2( 1.0, 0.0)*_SMAA_PIXEL_SIZE.xy).g;\n"
    "    factor.x -= rounding.y * texture2D(edgesTex, texcoord.zw + vec2( 1.0, API_V_DIR(1.0))*_SMAA_PIXEL_SIZE.xy).g;\n"
    "    factor.y -= rounding.x * texture2D(edgesTex, texcoord.xy + vec2(-2.0, 0.0)*_SMAA_PIXEL_SIZE.xy).g;\n"
    "    factor.y -= rounding.y * texture2D(edgesTex, texcoord.zw + vec2(-2.0, API_V_DIR(1.0))*_SMAA_PIXEL_SIZE.xy).g;\n"
    "    weights *= saturate(factor);\n"
    "}\n"
    "vec4 SMAABlendingWeightCalculationPS(vec2 texcoord,vec2 pixcoord,vec4 offset[3],sampler2D edgesTex,sampler2D areaTex,sampler2D searchTex,vec4 subsampleIndices) {\n"
    "    vec4 weights = vec4(0.0, 0.0, 0.0, 0.0);\n"
    "    vec2 e = texture2D(edgesTex, texcoord).rg;\n"
    "    if (e.g > 0.0) {\n"
    "        if(SMAAInfo2Ints.x > 0){\n"
    "            weights.rg = SMAACalculateDiagWeights(edgesTex,areaTex, texcoord, e, subsampleIndices);\n"
    "            if (weights.r == -weights.g) {\n"
    "                vec2 d;\n"
    "                vec3 coords;\n"
    "                coords.x = SMAASearchXLeft(edgesTex,searchTex, offset[0].xy, offset[2].x);\n"
    "                coords.y = offset[1].y;\n"
    "                d.x = coords.x;\n"
    "                float e1 = texture2D(edgesTex, coords.xy).r;\n"
    "                coords.z = SMAASearchXRight(edgesTex,searchTex,offset[0].zw, offset[2].y);\n"
    "                d.y = coords.z;\n"
    "                d = abs(round(mad(_SMAA_PIXEL_SIZE.zz, d, -pixcoord.xx)));\n"
    "                vec2 sqrt_d = sqrt(d);\n"
    "                float e2 = texture2D(edgesTex, coords.zy + vec2(1.0,0.0)*_SMAA_PIXEL_SIZE.xy).r;\n"
    "                weights.rg = SMAAArea(areaTex, sqrt_d, e1, e2, subsampleIndices.y);\n"
    "                coords.y = texcoord.y;\n"
    "                SMAADetectHorizontalCornerPattern(edgesTex, weights.rg, coords.xyzy, d);\n"
    "            }else{ \n"
    "                e.r = 0.0; \n"
    "            }\n"
    "        }else{\n"
    "           vec2 d;\n"
    "           vec3 coords;\n"
    "           coords.x = SMAASearchXLeft(edgesTex,searchTex, offset[0].xy, offset[2].x);\n"
    "           coords.y = offset[1].y;\n"
    "           d.x = coords.x;\n"
    "           float e1 = texture2D(edgesTex, coords.xy).r;\n"
    "           coords.z = SMAASearchXRight(edgesTex,searchTex,offset[0].zw, offset[2].y);\n"
    "           d.y = coords.z;\n"
    "           d = abs(round(mad(_SMAA_PIXEL_SIZE.zz, d, -pixcoord.xx)));\n"
    "           vec2 sqrt_d = sqrt(d);\n"
    "           float e2 = texture2D(edgesTex, coords.zy + vec2(1.0,0.0)*_SMAA_PIXEL_SIZE.xy).r;\n"
    "           weights.rg = SMAAArea(areaTex, sqrt_d, e1, e2, subsampleIndices.y);\n"
    "           coords.y = texcoord.y;\n"
    "           SMAADetectHorizontalCornerPattern(edgesTex, weights.rg, coords.xyzy, d);\n"  
    "        }\n"
    "    }\n"
    "    if (e.r > 0.0) {\n"
    "        vec2 d;\n"
    "        vec3 coords;\n"
    "        coords.y = SMAASearchYUp(edgesTex,searchTex, offset[1].xy, offset[2].z);\n"
    "        coords.x = offset[0].x;\n"
    "        d.x = coords.y;\n"
    "        float e1 = texture2D(edgesTex, coords.xy).g;\n"
    "        coords.z = SMAASearchYDown(edgesTex,searchTex, offset[1].zw, offset[2].w);\n"
    "        d.y = coords.z;\n"
    "        d = abs(round(mad(_SMAA_PIXEL_SIZE.ww, d, -pixcoord.yy)));\n"
    "        vec2 sqrt_d = sqrt(d);\n"
    "        float e2 = texture2D(edgesTex, coords.xz + vec2(0.0, API_V_DIR(1.0))*_SMAA_PIXEL_SIZE.xy).g;\n"
    "        weights.ba = SMAAArea(areaTex, sqrt_d, e1, e2, subsampleIndices.x);\n"
    "        coords.x = texcoord.x;\n"
    "        SMAADetectVerticalCornerPattern(edgesTex, weights.ba, coords.xyxz, d);\n"
    "    }\n"
    "    return weights;\n"
    "}\n"
    "void main(){\n"
    "    vec4 subSamples = vec4( 0.0 , 0.0 , 0.0 , 0.0 );\n"
    "    gl_FragColor = SMAABlendingWeightCalculationPS(uv,pixCoord,_offset,edge_tex,area_tex,search_tex,subSamples);\n"
    "}\n";
epriv::EShaders::smaa_vertex_3 = epriv::EShaders::smaa_common +
    "\n"//neightbor vert
    "layout (location = 0) in vec3 position;\n"
    "layout (location = 1) in vec2 UV;\n"
    "\n"
    "uniform mat4 MVP;\n"
    "uniform vec4 SMAA_PIXEL_SIZE;\n" //make this globally inherit for all smaa shaders
    "uniform vec2 screenSizeDivideBy2;\n"
    "\n"
    "varying vec2 uv;\n"
    "varying vec4 _offset;\n"
    "\n"
    "flat varying vec4 _SMAA_PIXEL_SIZE;\n"
    "\n"
    "void main(){\n"
    "    uv = UV;\n"
    "    vec3 vert = position;\n"
    "    vert.x *= screenSizeDivideBy2.x;\n"
    "    vert.y *= screenSizeDivideBy2.y;\n"
    "    _offset = mad(SMAA_PIXEL_SIZE.xyxy,vec4(1.0,0.0,0.0,API_V_DIR(1.0)),uv.xyxy);\n"
    "    gl_Position = MVP * vec4(vert,1.0);\n"
    "    _SMAA_PIXEL_SIZE = SMAA_PIXEL_SIZE;\n"
    "}\n";
epriv::EShaders::smaa_frag_3 = epriv::EShaders::smaa_common +
    "\n"//neighbor frag
    "uniform sampler2D textureMap;\n"
    "uniform sampler2D blend_tex;\n"
    "\n"
    "varying vec2 uv;\n"
    "varying vec4 _offset;\n"
    "\n"
    "flat varying vec4 _SMAA_PIXEL_SIZE;\n"
    "\n"
    "vec4 SMAANeighborhoodBlendingPS(vec2 texcoord,vec4 offset,sampler2D colorTex,sampler2D blendTex) {\n"
    "    vec4 a;\n"
    "    a.x = texture2D(blendTex, offset.xy).a;\n"
    "    a.y = texture2D(blendTex, offset.zw).g;\n"
    "    a.wz = texture2D(blendTex, texcoord).xz;\n"
    "    if (dot(a, vec4(1.0, 1.0, 1.0, 1.0)) <= 1e-5) {\n"
    "        vec4 color = texture2DLod(colorTex, texcoord,0.0);\n"
    "        return color;\n"
    "    }else{\n"
    "        bool h = max(a.x, a.z) > max(a.y, a.w);\n"
    "        vec4 blendingOffset = vec4(0.0, API_V_DIR(a.y), 0.0, API_V_DIR(a.w));\n"
    "        vec2 blendingWeight = a.yw;\n"
    "        SMAAMovc(bvec4(h, h, h, h), blendingOffset, vec4(a.x, 0.0, a.z, 0.0));\n"
    "        SMAAMovc(bvec2(h, h), blendingWeight, a.xz);\n"
    "        blendingWeight /= dot(blendingWeight, vec2(1.0, 1.0));\n"
    "        vec4 blendingCoord = mad(blendingOffset, vec4(_SMAA_PIXEL_SIZE.xy, -_SMAA_PIXEL_SIZE.xy), texcoord.xyxy);\n"
    "        vec4 color = blendingWeight.x * texture2D(colorTex, blendingCoord.xy);\n"
    "        color += blendingWeight.y * texture2D(colorTex, blendingCoord.zw);\n"
    "        return color;\n"
    "    }\n"
    "}\n"
    "void main(){\n"
    "    gl_FragColor = SMAANeighborhoodBlendingPS(uv, _offset, textureMap, blend_tex);\n"
    "}";

//vertex & frag 4 are optional passes
epriv::EShaders::smaa_vertex_4 = epriv::EShaders::smaa_common +
    "\n"
    "layout (location = 0) in vec3 position;\n"
    "layout (location = 1) in vec2 UV;\n"
    "\n"
    "uniform mat4 MVP;\n"
    "uniform vec2 screenSizeDivideBy2;\n"
    "\n"
    "varying vec2 uv;\n"
    "\n"
    "void SMAAResolveVS(inout vec2 uv){\n"
    "}\n"
    "void SMAASeparateVS(inout vec2 uv){\n"
    "}\n"
    "void main(){\n"
    "    uv = UV;\n"
    "    vec3 vert = position;\n"
    "    vert.x *= screenSizeDivideBy2.x;\n"
    "    vert.y *= screenSizeDivideBy2.y;\n"
    "    gl_Position = MVP * vec4(vert,1.0);\n"
    "}";
epriv::EShaders::smaa_frag_4 = epriv::EShaders::smaa_common +
    "\n"
    "varying vec2 uv;\n"
    "vec4 SMAAResolvePS(vec2 texcoord,sampler2D currentColorTex,sampler2D previousColorTex){\n"
    "    vec4 current = texture2D(currentColorTex, texcoord);\n"
    "    vec4 previous = texture2D(previousColorTex, texcoord);\n"
    "    return mix(current, previous, 0.5);\n"
    "}\n"
    "void main(){\n"
    "    gl_FragColor = vec4(0.0,0.0,0.0,1.0);\n"
    "}";
#pragma endregion
   
#pragma region ForwardFrag
epriv::EShaders::forward_frag =
    "\n"
    "void main(){\n"
    "}";
#pragma endregion

#pragma region DeferredFrag
epriv::EShaders::deferred_frag =
    "\n"
    "USE_LOG_DEPTH_FRAGMENT\n"
    "\n"
    "uniform sampler2D   DiffuseTexture;\n"
    "uniform sampler2D   NormalTexture;\n"
    "uniform sampler2D   GlowTexture;\n"
    "uniform sampler2D   SpecularTexture;\n"
    "uniform sampler2D   AOTexture;\n"
    "uniform sampler2D   MetalnessTexture;\n"
    "uniform sampler2D   SmoothnessTexture;\n"
    "uniform samplerCube ReflectionTexture;\n"
    "uniform sampler2D   ReflectionTextureMap;\n"
    "uniform samplerCube RefractionTexture;\n"
    "uniform sampler2D   RefractionTextureMap;\n"
    "uniform sampler2D   HeightmapTexture;\n"
    "\n"
    "uniform vec4        MaterialBasePropertiesOne;\n"//x = BaseGlow, y = BaseAO, z = BaseMetalness, w = BaseSmoothness
    "\n"
    //make these the fourth conditionals?
    "uniform float       CubemapMixFactor;\n"
    "uniform float       ParallaxHeightScale;\n"
    "uniform float       RefractiveIndex;\n"
    "\n"
    "uniform int Shadeless;\n"
    "uniform int HasGodsRays;\n"
    "\n"
    "uniform vec4 FirstConditionals;\n" //x = diffuse  y = normals    z = glow w = specular
    "uniform vec4 SecondConditionals;\n" //x = ao y = metal z = smoothness w = reflection
    "uniform vec4 ThirdConditionals;\n" //x = refraction y = heightmap z = UNUSED w = UNUSED
    "\n"
    "uniform vec4 Object_Color;\n"
    "uniform vec4 Material_F0AndID;\n"
    "uniform vec3 Gods_Rays_Color;\n"
    "\n"
    "varying vec3 WorldPosition;\n"
    "varying vec2 UV;\n"
    "varying vec3 Normals;\n"
    "varying mat3 TBN;\n"
    "flat varying vec3 CamPosition;\n"
    "varying vec3 TangentCameraPos;\n"
    "varying vec3 TangentFragPos;\n"
    "\n";
epriv::EShaders::deferred_frag += epriv::EShaders::float_into_2_floats;
epriv::EShaders::deferred_frag += epriv::EShaders::normals_octahedron_compression_functions;
epriv::EShaders::deferred_frag +=
    "vec4 Reflection(vec2 _uv,vec4 d, vec3 cpos, vec3 n, vec3 wpos){\n"
    "    vec4 r = vec4(0.0);\n"
    "    r = textureCube(ReflectionTexture,reflect(n,normalize(cpos - wpos))) * texture2D(ReflectionTextureMap,_uv).r;\n"
    "    r.a *= CubemapMixFactor;\n"
    "    r = PaintersAlgorithm(r,d);\n"
    "    return r;\n"
    "}\n"
    "vec4 Refraction(vec2 _uv,vec4 d, vec3 cpos, vec3 n, vec3 wpos){\n"
    "    vec4 r = vec4(0.0);\n"
    "    r = textureCube(RefractionTexture,refract(n,normalize(cpos - wpos),1.0 / RefractiveIndex)) * texture2D(RefractionTextureMap,_uv).r;\n"
    "    r.a *= CubemapMixFactor;\n"
    "    r = PaintersAlgorithm(r,d);\n"
    "    return r;\n"
    "}\n"
    "vec2 ParallaxMap(vec3 _ViewDir){\n"
    "    float minLayers = min(5.0,(5.0 * ParallaxHeightScale)+1.0);\n"
    "    float maxLayers = min(30.0,(30.0 * ParallaxHeightScale)+1.0);\n"
    "    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), _ViewDir)));\n"
    "    float layerDepth = 1.0 / numLayers;\n"// calculate the size of each layer 
    "    float currentLayerDepth = 0.0;\n"// depth of current layer  
    "    vec2 P = _ViewDir.xy * ParallaxHeightScale;\n"// the amount to shift the texture coordinates per layer (from vector P)
    "    vec2 deltaUV = P / numLayers;\n"
    "    vec2  currentUV = UV;\n"
    "    float currentDepth = texture2D(HeightmapTexture, currentUV).r;\n"
    "    while(currentLayerDepth < currentDepth){\n"	
    "    	currentUV -= deltaUV;\n"// shift texture coordinates along direction of P
    "    	currentDepth = texture2D(HeightmapTexture, currentUV).r;\n"
    "    	currentLayerDepth += layerDepth;\n"
    "    }\n"
    "    vec2 prevUV = currentUV + deltaUV;\n" // get texture coordinates before collision (reverse operations)
    "    float afterDepth  = currentDepth - currentLayerDepth;\n" // get depth after and before collision for linear interpolation
    "    float beforeDepth = texture2D(HeightmapTexture, prevUV).r - currentLayerDepth + layerDepth;\n"
    "    float weight = afterDepth / (afterDepth - beforeDepth);\n" // interpolation of texture coordinates
    "    return prevUV * weight + currentUV * (1.0 - weight);\n"
    "}\n"
    "void main(){\n"
    "    vec2 uv = UV;\n"
    "    float ao = MaterialBasePropertiesOne.y;\n"
    "    float metalness = MaterialBasePropertiesOne.z;\n"
    "    float smoothness = MaterialBasePropertiesOne.w;\n"
    "    vec3 InNormals = normalize(Normals);\n"
    "    vec4 InDiffuse = texture2D(DiffuseTexture, uv);\n"
    "    vec4 OutDiffuse = Object_Color;\n"
    "	 float OutGlow = MaterialBasePropertiesOne.x;\n"
    "	 float OutSpecular = 1.0;\n"
    "	 float OutMatIDAndAO = Material_F0AndID.w + ao;\n"
    "    if(ThirdConditionals.y > 0.5){\n"
    "        vec3 ViewDir = normalize(TangentCameraPos - TangentFragPos);\n"
    "        uv = ParallaxMap(ViewDir);\n"
    "    }\n"
    "    if(SecondConditionals.x > 0.5){\n"
    "        ao *= texture2D(AOTexture, uv).r;\n"
    "    }\n"
    "    if(SecondConditionals.y > 0.5){\n"
    "        metalness *= texture2D(MetalnessTexture, uv).r;\n"
    "    }\n"
    "    if(SecondConditionals.z > 0.5){\n"
    "        smoothness *= texture2D(SmoothnessTexture, uv).r;\n"
    "    }\n"
    "    if(FirstConditionals.x > 0.5){\n"
    "        OutDiffuse *= InDiffuse;\n"
    "    }else{\n"
    "        OutDiffuse *= vec4(Material_F0AndID.xyz,1.0);\n"
    "    }\n"
    "    if(FirstConditionals.y > 0.9){\n"
    "        InNormals = CalcBumpedNormal(uv,NormalTexture);\n"
    "    }else if(FirstConditionals.y > 0.4){\n"
    "        InNormals = CalcBumpedNormalCompressed(uv,NormalTexture);\n"
    "    }\n"
    "    if(SecondConditionals.w > 0.5){\n"
    "        OutDiffuse = Reflection(uv,OutDiffuse,CamPosition,InNormals,WorldPosition);\n"
    "    }\n"
    "    if(ThirdConditionals.x > 0.5){\n"
    "        OutDiffuse = Refraction(uv,OutDiffuse,CamPosition,InNormals,WorldPosition);\n"
    "    }\n"
    "    vec2 OutNormals = EncodeOctahedron(InNormals);\n"
    "    if(Shadeless == 0){\n"
    "        if(FirstConditionals.z > 0.5){ \n"
    "            OutGlow += texture2D(GlowTexture, uv).r; \n"
    "        }\n"
    "        if(FirstConditionals.w > 0.5){ \n"
    "            OutSpecular = texture2D(SpecularTexture, uv).r; \n"
    "        }\n"
    "    }else{\n"
    "        OutNormals = ConstantOneVec2; \n"
    "    }\n"
    "	 float OutPackedMetalnessAndSmoothness = Pack2FloatIntoFloat16(metalness,smoothness);\n"
    "    gl_FragData[0] = OutDiffuse;\n"
    "    gl_FragData[1] = vec4(OutNormals,OutMatIDAndAO,OutPackedMetalnessAndSmoothness);\n"
    "    gl_FragData[2].rg = vec2(OutGlow,OutSpecular);\n"
    "    if(HasGodsRays == 1){\n"
    "        gl_FragData[3] = (InDiffuse * vec4(Gods_Rays_Color,1.0)) * 0.5;\n"
    "    }\n"
    "}";
#pragma endregion

#pragma region DeferredFragHUD
epriv::EShaders::deferred_frag_hud =
    "\n"
    "uniform sampler2D DiffuseTexture;\n"
    "uniform int DiffuseTextureEnabled;\n"
    "uniform vec4 Object_Color;\n"
    "varying vec2 UV;\n"
    "void main(){\n"
    "    gl_FragColor = Object_Color;\n"
    "    if(DiffuseTextureEnabled == 1){\n"
    "        gl_FragColor *= texture2D(DiffuseTexture, UV);\n"
    "    }\n"
    "}";
#pragma endregion

#pragma region DeferredFragSkybox
epriv::EShaders::deferred_frag_skybox =
    "\n"
    "uniform samplerCube Texture;\n"
    "varying vec3 UV;\n"
    "varying vec3 WorldPosition;\n"
    "void main(){\n"
    "    gl_FragData[0] = textureCube(Texture, UV);\n"
    "    gl_FragData[1].rg = vec2(1.0);\n"
    "    gl_FragData[2].rb = vec2(0.0);\n"
    "}";
#pragma endregion

#pragma region DeferredFragSkyboxFake
epriv::EShaders::deferred_frag_skybox_fake =
    "\n"
    "uniform vec4 Color;\n"
    "varying vec3 UV;\n"
    "varying vec3 WorldPosition;\n"
    "void main(){\n"
    "    gl_FragData[0].rgba = Color;\n"
    "    gl_FragData[1].rg = vec2(1.0);\n"
    "    gl_FragData[2].rb = vec2(0.0);\n"
    "}";
#pragma endregion

#pragma region CopyDepthFrag
epriv::EShaders::copy_depth_frag =
    "\n"
    "uniform sampler2D gDepthMap;\n"
    "varying vec2 texcoords;\n"
    "void main(){\n"
    "    gl_FragDepth = texture2D(gDepthMap,texcoords).r;\n"
    "}";
#pragma endregion

#pragma region SSAO

epriv::EShaders::ssao_frag =
    "USE_LOG_DEPTH_FRAG_WORLD_POSITION\n"
    "\n"
    "uniform sampler2D gNormalMap;\n"
    "uniform sampler2D gRandomMap;\n"
    "uniform sampler2D gDepthMap;\n"
    "\n"
    "uniform vec2  ScreenSize;\n"
    "uniform vec4  SSAOInfo;\n"  //   x = radius     y = intensity    z = bias        w = scale
    "uniform ivec4 SSAOInfoA;\n"//    x = UNUSED     y = UNUSED       z = Samples     w = NoiseTextureSize
    "\n"
    "uniform vec3 poisson[32];\n"
    "\n"
    "varying vec2 texcoords;\n";
epriv::EShaders::ssao_frag += epriv::EShaders::normals_octahedron_compression_functions;
epriv::EShaders::ssao_frag +=
    "float occlude(vec2 offsetUV, vec3 origin, vec3 normal){\n"
    "    vec3 PositionOffset = GetWorldPosition(offsetUV,CameraNear,CameraFar) - origin;\n"
    "    float Len = length(PositionOffset);\n"
    "    vec3 vec = PositionOffset / Len;\n"
    "    return max(0.0, dot(normal,vec) - SSAOInfo.z) * (1.0 / (1.0 + (Len * SSAOInfo.w))) * SSAOInfo.y;\n"
    "}\n"
    "void main(){\n"
    "    vec3 WorldPos = GetWorldPosition(texcoords,CameraNear,CameraFar);\n"
    "    vec3 Normal = DecodeOctahedron(texture2D(gNormalMap, texcoords).rg);\n"
    "    vec2 RandVector = normalize(texture2D(gRandomMap, ScreenSize * texcoords / SSAOInfoA.w).xy) * 2.0 - 1.0;\n"
    "    float Distance = distance(WorldPos, CameraPosition) + 0.0001;\n"//cuz we dont like divide by zeros ;)
    "    float Radius = max(0.05,SSAOInfo.x / Distance);\n"
    "    float o = 0.0;\n"
    "    for (int i = 0; i < SSAOInfoA.z; ++i) {\n"
    "       vec2 coord1 = reflect(poisson[i].xy, RandVector) * Radius;\n"
    "       vec2 coord2 = vec2(coord1.x * 0.707 - coord1.y * 0.707, coord1.x * 0.707 + coord1.y * 0.707);\n"
    "       o += occlude(texcoords + (coord1 * 0.25), WorldPos, Normal);\n"
    "       o += occlude(texcoords + (coord2 * 0.50), WorldPos, Normal);\n"
    "       o += occlude(texcoords + (coord1 * 0.75), WorldPos, Normal);\n"
    "       o += occlude(texcoords + coord2,          WorldPos, Normal);\n"
    "    }\n"
    "    o /= SSAOInfoA.z * 4.0;\n"
    "    o = clamp(o,0.01,0.99);\n"
    "    o = mix(0.0,o,(o >= 0.99 ? 0.0 : 1.0 ));\n"//this gets rid of the dark annoying edges around models. in a very hacky way...
    "    gl_FragColor.a = o;\n"
    "}";
#pragma endregion

#pragma region Bloom
epriv::EShaders::bloom_frag =
    "\n"
    "const vec3 ConstantZeroVec3 = vec3(0.0,0.0,0.0);\n"
    "uniform sampler2D gLightMap;\n"
    "\n"
    "uniform vec4 Data;\n" //x = scale y = threshold z = exposure w = UNUSED
    "varying vec2 texcoords;\n"
    "\n";
epriv::EShaders::bloom_frag += epriv::EShaders::float_into_2_floats;
epriv::EShaders::bloom_frag +=
    "void main(){\n"
    "    vec3 lighting = texture2D(gLightMap,texcoords).rgb;\n"
    "    lighting = vec3(1.0) - exp(-lighting * Data.z);\n"
    "    gl_FragColor.rgb = max(ConstantZeroVec3,lighting - vec3(Data.y)) * Data.x;\n"
    "}";
#pragma endregion

#pragma region HDR
epriv::EShaders::hdr_frag =
    "\n"
    "uniform sampler2D lightingBuffer;\n"
    "uniform sampler2D gDiffuseMap;\n"
    "uniform sampler2D gNormalMap;\n"
    "uniform sampler2D gGodsRaysMap;\n"
    "varying vec2 texcoords;\n"
    "uniform vec4 HDRInfo;\n"// exposure | HasHDR | godRaysExposure | HDRAlgorithm
    "uniform ivec2 Has;\n" //HasGodRays | HasLighting
    "\n"
    "vec3 uncharted(vec3 x,float a,float b,float c,float d,float e,float f){ return vec3(((x*(a*x+c*b)+d*e)/(x*(a*x+b)+d*f))-e/f); }\n";
epriv::EShaders::hdr_frag += epriv::EShaders::normals_octahedron_compression_functions;
epriv::EShaders::hdr_frag +=
    "void main(){\n"
    "    vec3 diffuse = texture2D(gDiffuseMap,texcoords).rgb;\n"
    "    vec3 lighting = texture2D(lightingBuffer, texcoords).rgb;\n"
    "    vec3 normals = DecodeOctahedron(texture2D(gNormalMap,texcoords).rg);\n"
    "\n"
    "    if(distance(normals,ConstantOneVec3) < 0.01 || Has.y == 0){\n" //if normals are damn near 1.0,1.0,1.0 or no lighting
    "        lighting = diffuse;\n"
    "    }\n"
    "    if(HDRInfo.y == 1.0){\n"//HasHDR
    "        if(HDRInfo.w == 0.0){\n"// Reinhard tone mapping
    "            lighting = lighting / (lighting + ConstantOneVec3);\n"
    "        }else if(HDRInfo.w == 1.0){\n"// Filmic tone mapping
    "            vec3 x = max(vec3(0), lighting - vec3(0.004));\n"
    "            lighting = (x * (vec3(6.20) * x + vec3(0.5))) / (x * (vec3(6.2) * x + vec3(1.7)) + vec3(0.06));\n"
    "        }else if(HDRInfo.w == 2.0){\n"// Exposure tone mapping
    "            lighting = ConstantOneVec3 - exp(-lighting * HDRInfo.x);\n"
    "        }else if(HDRInfo.w == 3.0){\n"// Uncharted tone mapping
    "            float A = 0.15; float B = 0.5; float C = 0.1; float D = 0.2; float E = 0.02; float F = 0.3; float W = 11.2;\n"
    "            lighting = HDRInfo.x * uncharted(lighting,A,B,C,D,E,F);\n"
    "            vec3 white = 1.0 / uncharted( vec3(W),A,B,C,D,E,F );\n"
    "            lighting *= white;\n"
    "        }\n"
    "    }\n"
    "    if(Has.x == 1){\n"
    "        vec3 rays = texture2D(gGodsRaysMap,texcoords).rgb;\n"
    "        lighting = (lighting * 1.1) + (rays * HDRInfo.z);\n"
    "    }\n"
    "    gl_FragColor = vec4(lighting, 1.0);\n"
    "}";
#pragma endregion

#pragma region Blur
epriv::EShaders::blur_frag =
    "uniform sampler2D image;\n"
    "uniform vec4 RGBA;\n"
    "uniform vec4 DataA;\n"//radius, UNUSED, H,V
    "uniform vec4 strengthModifier;\n"
    "\n"
    "varying vec2 texcoords;\n"
    "\n"
    "const int NUM_SAMPLES = 9;\n"
    "const float weight[NUM_SAMPLES] = float[](0.227,0.21,0.1946,0.162,0.12,0.08,0.054,0.03,0.016);\n"
    "\n"
    "void main(){\n"
    "    vec2 texOffset = vec2(1.0) / textureSize(image,0);\n"
    "    vec4 v4Sum = vec4(0.0);\n"
    "    for(int i = 0; i < NUM_SAMPLES; ++i){\n"
    "        vec2 offset = (texOffset * float(i)) * DataA.x;\n"
    "        v4Sum += (texture2D(image,texcoords + vec2(offset.x * DataA.z,offset.y * DataA.w)) * weight[i]) * strengthModifier;\n"
    "        v4Sum += (texture2D(image,texcoords - vec2(offset.x * DataA.z,offset.y * DataA.w)) * weight[i]) * strengthModifier;\n"
    "    }\n"
    "    gl_FragColor = (v4Sum * RGBA) + (gl_FragColor * (vec4(1.0) - RGBA));\n"
    "}";
#pragma endregion

#pragma region SSAO Blur
epriv::EShaders::ssao_blur_frag =
"uniform sampler2D image;\n"
"uniform vec4 Data;\n"//radius, UNUSED, H,V
"uniform float strengthModifier;\n"
"uniform vec2 invRes;\n"
"\n"
"varying vec2 texcoords;\n"
"\n"
"const int NUM_SAMPLES = 9;\n"
"const float weight[NUM_SAMPLES] = float[](0.227,0.21,0.1946,0.162,0.12,0.08,0.054,0.03,0.016);\n"
"\n"
"void main(){\n"
"    float Sum = 0.0;\n"
"    for(int i = 0; i < NUM_SAMPLES; ++i){\n"
"        vec2 offset = invRes * float(i) * Data.x;\n"
"        Sum += texture2D(image,texcoords + vec2(offset.x * Data.z,offset.y * Data.w)).a * weight[i] * strengthModifier;\n"
"        Sum += texture2D(image,texcoords - vec2(offset.x * Data.z,offset.y * Data.w)).a * weight[i] * strengthModifier;\n"
"    }\n"
"    gl_FragColor.a = Sum;\n"
"}";
#pragma endregion

#pragma region GodRays
epriv::EShaders::godRays_frag =
    "uniform vec4 RaysInfo;\n"//exposure | decay | density | weight
    "\n"
    "uniform vec2 lightPositionOnScreen;\n"
    "uniform sampler2D firstPass;\n"
    "uniform int samples;\n"
    "\n"
    "uniform float alpha;\n"
    "varying vec2 texcoords;\n"
    "void main(){\n"
    "    vec2 uv = texcoords;\n"
    "    vec2 deltaUV = vec2(uv - lightPositionOnScreen);\n"
    "    deltaUV *= 1.0 /  float(samples) * RaysInfo.z;\n"
    "    float illuminationDecay = 1.0;\n"
    "    vec3 totalColor = vec3(0.0);\n"
    "    for(int i=0; i < samples; ++i){\n"
    "        uv -= deltaUV / 2.0;\n"
    "        vec3 sample = texture2D(firstPass,uv).rgb;\n"
    "        sample *= illuminationDecay * RaysInfo.w;\n"
    "        totalColor += sample;\n"
    "        illuminationDecay *= RaysInfo.y;\n"
    "    }\n"
    "    gl_FragColor.rgb = (totalColor * alpha) * RaysInfo.x;\n"
    "}";
#pragma endregion

#pragma region Greyscale
epriv::EShaders::greyscale_frag =
    "\n"
    "uniform sampler2D textureMap;\n"
    "varying vec2 texcoords;\n"
    "void main(){\n"
    "    vec4 col = texture2D(textureMap, texcoords);\n"
    "    float lum = dot(col.rgb, vec3(0.299, 0.587, 0.114));\n"
    "    gl_FragColor = vec4(vec3(lum), 1.0);\n"
    "}";
#pragma endregion
    
#pragma region FinalFrag
epriv::EShaders::final_frag =
    "\n"
    "uniform sampler2D SceneTexture;\n"
    "uniform sampler2D gBloomMap;\n"
    "uniform sampler2D gDepthMap;\n"
    "\n"
    "uniform int HasBloom;\n"
    "uniform int HasFog;\n"
    "\n"
    "uniform vec4 FogColor;\n"
    "uniform float FogDistNull;\n"
    "uniform float FogDistBlend;\n"
    "\n"
    "varying vec2 texcoords;\n"
    "\n";
epriv::EShaders::final_frag += epriv::EShaders::float_into_2_floats;
epriv::EShaders::final_frag += epriv::EShaders::normals_octahedron_compression_functions;
epriv::EShaders::final_frag +=
    "\n"
    "void main(){\n"
    "    vec3 scene = texture2D(SceneTexture,texcoords).rgb;\n"
    "    if(HasBloom == 1){\n"
    "        vec3 bloom = texture2D(gBloomMap,texcoords).rgb;\n"
    "        scene += bloom;\n"
    "    }\n"
    "    gl_FragColor = vec4(scene,1.0);\n"
    "    if(HasFog == 1){\n"
    "        float distFrag = abs(distance(GetWorldPosition(texcoords,CameraNear,CameraFar),CameraPosition));\n"
    "        float distVoid = FogDistNull + FogDistBlend;\n"
    "        float distBlendIn = FogDistBlend - (distVoid - distFrag);\n"
    "        float omega = smoothstep(0.0,1.0,(distBlendIn / FogDistBlend));\n"
    "        vec4 fc = FogColor * clamp(omega,0.0,1.0);\n"
    "        gl_FragColor = PaintersAlgorithm(fc,gl_FragColor);\n"
    "    }\n"
    "\n"
    "}";

#pragma endregion

#pragma region LightingFrag
epriv::EShaders::lighting_frag =
    "#define MATERIAL_COUNT_LIMIT 255\n"
    "\n"
    "uniform vec4 LightDataA;\n" //x = ambient, y = diffuse, z = specular, w = LightDirection.x
    "uniform vec4 LightDataB;\n" //x = LightDirection.y, y = LightDirection.z, z = const, w = linear
    "uniform vec4 LightDataC;\n" //x = exp, y = LightPosition.x, z = LightPosition.y, w = LightPosition.z
    "uniform vec4 LightDataD;\n" //x = LightColor.r, y = LightColor.g, z = LightColor.b, w = LightType
    "uniform vec4 LightDataE;\n" //x = cutoff, y = outerCutoff, z = AttenuationModel, w = UNUSED
    "\n"
    "uniform sampler2D gDiffuseMap;\n"
    "uniform sampler2D gNormalMap;\n"
    "uniform sampler2D gMiscMap;\n"
    "uniform sampler2D gDepthMap;\n"
    "uniform sampler2D gSSAOMap;\n"
    "\n"
    "uniform vec4 ScreenData;\n" //x = UNUSED, y = screenGamma, z = winSize.x, w = winSize.y
    "uniform vec4 materials[MATERIAL_COUNT_LIMIT];\n"//r = MaterialF0Color (packed into float), g = baseSmoothness, b = specularModel, a = diffuseModel
    "\n"
    "varying vec2 texcoords;\n"
    "\n";
epriv::EShaders::lighting_frag += epriv::EShaders::normals_octahedron_compression_functions;
epriv::EShaders::lighting_frag += epriv::EShaders::float_into_2_floats;
epriv::EShaders::lighting_frag +=
    "float azimuth(vec3 vector){\n"
    "    return atan(vector.y / vector.x);\n" //might also be x / y and not y / x
    "}\n"
    "float polar(vec3 vector){\n"
    "    return acos(vector.z / length(vector));\n"
    "}\n"
    "float BeckmannDist(float cos2a, float _alpha){\n"
    "    float b = (1.0 - cos2a) / (cos2a * _alpha);\n"
    "    return (exp(-b)) / (KPI * _alpha * cos2a * cos2a);\n"
    "}\n"
    "float GGXDist(float NdotHSquared, float alphaSquared){\n"
    "    float denom = (NdotHSquared * (alphaSquared - 1.0) + 1.0);\n"
    "    denom = KPI * denom * denom;\n"
    "    return alphaSquared / denom;\n"
    "}\n"
    "float GeometrySchlickGGX(float NdotV, float a){\n"
    "    float k = a * 0.125;\n"
    "    float denom = NdotV * (1.0 - k) + k;\n"
    "    return NdotV / denom;\n"
    "}\n"
    "vec3 SchlickFrensel(float theta, vec3 _F0){\n"
    "    vec3 ret = _F0 + (ConstantOneVec3 - _F0) * pow(1.0 - theta,5.0);\n"
    "    return ret;\n"
    "}\n"
    "float CalculateAttenuation(float Dist,float LightRadius){\n"
    "    float attenuation = 0.0;\n"
    //"   if(LightDataE.z == 0.0){\n" //constant
    //"       attenuation = 1.0 / max(1.0 , LightDataB.z);\n"
    //"   }else if(LightDataE.z == 1.0){\n" //distance
    //"       attenuation = 1.0 / max(1.0 , Dist);\n"
    //"   }else if(LightDataE.z == 2.0){\n" //distance squared
    //"       attenuation = 1.0 / max(1.0 , Dist * Dist);\n"
    //"   }else if(LightDataE.z == 3.0){\n" //constant linear exponent
    "       attenuation = 1.0 / max(1.0 , LightDataB.z + (LightDataB.w * Dist) + (LightDataC.x * Dist * Dist));\n"
    //"   }else if(LightDataE.z == 4.0){\n" //distance radius squared
    //"       attenuation = 1.0 / max(1.0 ,pow((Dist / LightRadius) + 1.0,2.0));\n"
    //"   }\n"
    "   return attenuation;\n"
    "}\n"
    "float DiffuseOrenNayar(vec3 _ViewDir, vec3 _LightDir,float _NdotL,float _VdotN,float _alpha,vec3 _PxlNormal){\n"
    "    float A = 1.0 - 0.5 * _alpha / (_alpha + 0.33);\n"
    "    float B = 0.45 * _alpha / (_alpha + 0.09);\n"
    "    float cosAzimuthSinPolarTanPolar = (dot(_LightDir, _ViewDir) - _VdotN * _NdotL) / max(_VdotN, _NdotL);\n"
    "    return (A + B * max(0.0, cosAzimuthSinPolarTanPolar));\n"
    "}\n"
    "vec3 DiffuseAshikhminShirley(float _smoothness,vec3 _MaterialAlbedoTexture,float _NdotL,float _VdotN){\n"
    "    vec3 ret;\n"
    "    float s = clamp(_smoothness,0.01,0.76);\n" //this lighting model has to have some form of roughness in it to look good. cant be 1.0
    "    vec3 A = (28.0 * _MaterialAlbedoTexture) / vec3(23.0 * KPI);\n"
    "    float B = 1.0 - s;\n"
    "    float C = (1.0 - pow((1.0 - (_NdotL * 0.5)),5.0));\n"
    "    float D = (1.0 - pow((1.0 - (_VdotN * 0.5)),5.0));\n"
    "    ret = A * B * C * D;\n"
    "    ret *= KPI;\n" //i know this isnt proper, but the diffuse component is *way* too dark otherwise...
    "    return ret;\n"
    "}\n"
    "vec3 SpecularBlinnPhong(float _smoothness,float _NdotH){\n"
    "    float gloss = exp2(10.0 * _smoothness + 1.0);\n"
    "    float kS = (8.0 + gloss ) / (8.0 * KPI);\n"
    "    return vec3(kS * pow(_NdotH, gloss));\n"
    "}\n"
    "vec3 SpecularPhong(float _smoothness,vec3 _LightDir,vec3 _PxlNormal,vec3 _ViewDir){\n"
    "    float gloss = exp2(10.0 * _smoothness + 1.0);\n"
    "    float kS = (2.0 + gloss ) / (2.0 * KPI);\n"
    "    vec3 Reflect = reflect(-_LightDir, _PxlNormal);\n"
    "    float VdotR = max(0.0, dot(_ViewDir,Reflect));\n"
    "    return vec3(kS * pow(VdotR, gloss));\n"
    "}\n"
    "vec3 SpecularGGX(inout vec3 _Frensel,vec3 _LightDir,vec3 _Half,float _alpha,float _NdotH,vec3 _F0,float _NdotL){\n"
    "    float LdotH = max(0.0, dot(_LightDir,_Half));\n"
    "    float alphaSqr = _alpha * _alpha;\n"
    "    float denom = _NdotH * _NdotH * (alphaSqr - 1.0) + 1.0;\n"
    "    float NDF = alphaSqr / (KPI * denom * denom);\n"
    "    _Frensel = SchlickFrensel(LdotH,_F0);\n"
    "    float k = _alpha * 0.5;\n"
    "    float k2 = k * k;\n"
    "    float invk2 = 1.0 - k2;\n"
    "    float G = 1.0 / (LdotH * LdotH * invk2 + k2);\n"
    "    return _NdotL * NDF * _Frensel * G;\n"
    "}\n"
    "vec3 SpecularCookTorrance(inout vec3 _Frensel,vec3 _F0,float _VdotH,float _NdotH,float _alpha,float _VdotN,float _roughness,float _NdotL){\n"
    "    _Frensel = SchlickFrensel(_VdotH,_F0);\n"
    "    float NDF = GGXDist(_NdotH * _NdotH, _alpha * _alpha);\n"
    "    float roughnessPlusOne = _roughness + 1.0;\n"
    "    float a = roughnessPlusOne * roughnessPlusOne;\n"
    "    float G = GeometrySchlickGGX(_VdotN,a) * GeometrySchlickGGX(_NdotL,a);\n"
    "    vec3 Top = NDF * _Frensel * G;\n"
    "    float Bottom = max(4.0 * _VdotN * _NdotL,0.0);\n"
    "    return Top / (Bottom + 0.001);\n"
    "}\n"
    "vec3 SpecularGaussian(float _NdotH,float _smoothness){\n"
    "    float b = acos(_NdotH);\n" //this might also be cos. find out
    "    float fin = b / _smoothness;\n"
    "    return vec3(exp(-fin*fin));\n"
    "}\n"
    "vec3 SpecularAshikhminShirley(vec3 _PxlNormal,vec3 _Half,float _NdotH,vec3 _LightDir,float _NdotL,float _VdotN){\n"
    "    const float Nu = 1000.0;\n"//make these controllable uniforms
    "    const float Nv = 1000.0;\n"//make these controllable uniforms
    "    vec3 epsilon = vec3(1.0,0.0,0.0);\n"
    "    vec3 tangent = normalize(cross(_PxlNormal,epsilon));\n"
    "    vec3 bitangent = normalize(cross(_PxlNormal,tangent));\n"
    "    float hdotT = dot(_Half,tangent);\n"
    "    float hDotB = dot(_Half,bitangent);\n"
    "    float A = sqrt( (Nu + 1.0) * (Nv + 1.0) );\n"
    "    float B = pow(_NdotH,((Nu * hdotT * hdotT + Nv * hDotB * hDotB) / (1.0 - (_NdotH * _NdotH)) ));\n"
    "    float HdotL = max(0.0, dot(_Half, _LightDir));\n"
    "    float C = 8.0 * KPI * HdotL * max(_NdotL,_VdotN);\n"
    "    return vec3((A * B) / C);\n"
    "}\n"
    "vec3 CalcLightInternal(vec3 LightDir,vec3 PxlWorldPos,vec3 PxlNormal,vec2 uv){\n"
    "    float Glow = texture2D(gMiscMap,uv).r;\n"
    "    float SpecularStrength = texture2D(gMiscMap,uv).g;\n"
    "    vec3 MaterialAlbedoTexture = texture2D(gDiffuseMap,uv).rgb;\n"
    "    vec3 LightDiffuseColor  = LightDataD.xyz;\n"
    "    vec3 LightSpecularColor = LightDataD.xyz * SpecularStrength;\n"
    "    vec3 TotalLight         = ConstantZeroVec3;\n"
    "    vec3 SpecularFactor     = ConstantZeroVec3;\n"
    "\n"
    "    float matIDandAO = texture2D(gNormalMap,uv).b;\n"
    "    highp int matID = int(floor(matIDandAO));\n"
    "    float ssaoValue = 1.0 - texture2D(gSSAOMap,uv).a;\n"
    "    float ao = (fract(matIDandAO)+0.0001) * ssaoValue;\n"//the 0.0001 makes up for the clamp in material class
    "    vec2 stuff = UnpackFloat16Into2Floats(texture2D(gNormalMap,uv).a);\n"
    "    float metalness = stuff.x;\n"
    "    float smoothness = stuff.y;\n"
    "\n"
    "    vec3 MaterialF0 = Unpack3FloatsInto1FloatUnsigned(materials[matID].r);\n"
    "    vec3 F0 = mix(MaterialF0, MaterialAlbedoTexture, vec3(metalness));\n"
    "    vec3 Frensel = F0;\n"
    "\n"
    "    float roughness = 1.0 - smoothness;\n"
    "    float alpha = roughness * roughness;\n"
    "\n"
    "    vec3 ViewDir = normalize(CameraPosition - PxlWorldPos);\n"
    "    vec3 Half = normalize(LightDir + ViewDir);\n"
    "    float NdotL = clamp(dot(PxlNormal, LightDir),0.0,1.0);\n"
    "    float NdotH = clamp(dot(PxlNormal, Half),0.0,1.0);\n"
    "    float VdotN = clamp(dot(ViewDir,PxlNormal),0.0,1.0);\n"
    "    float VdotH = clamp(dot(ViewDir,Half),0.0,1.0);\n"
    "\n"
    "    float MaterialTypeDiffuse = materials[matID].a;\n"
    "    float MaterialTypeSpecular = materials[matID].b;\n"
    "\n"
    "    if(MaterialTypeDiffuse == 1.0){\n"
    "        LightDiffuseColor *= DiffuseOrenNayar(ViewDir,LightDir,NdotL,VdotN,alpha,PxlNormal);\n"
    "    }else if(MaterialTypeDiffuse == 2.0){\n"
    "        LightDiffuseColor *= DiffuseAshikhminShirley(smoothness,MaterialAlbedoTexture,NdotL,VdotN);\n"
    "    }else if(MaterialTypeDiffuse == 3.0){\n"//this is minneart
    "        LightDiffuseColor *= pow(VdotN*NdotL,smoothness);\n"
    "    }\n"
    "\n"
    "    if(MaterialTypeSpecular == 1.0){\n"
    "        SpecularFactor = SpecularBlinnPhong(smoothness,NdotH);\n"
    "    }else if(MaterialTypeSpecular == 2.0){\n"
    "        SpecularFactor = SpecularPhong(smoothness,LightDir,PxlNormal,ViewDir);\n"
    "    }else if(MaterialTypeSpecular == 3.0){\n"
    "        SpecularFactor = SpecularGGX(Frensel,LightDir,Half,alpha,NdotH,F0,NdotL);\n"
    "    }else if(MaterialTypeSpecular == 4.0){\n"
    "        SpecularFactor = SpecularCookTorrance(Frensel,F0,VdotH,NdotH,alpha,VdotN,roughness,NdotL);\n"
    "    }else if(MaterialTypeSpecular == 5.0){\n"
    "        SpecularFactor = SpecularGaussian(NdotH,smoothness);\n"
    "    }else if(MaterialTypeSpecular == 6.0){\n"
    "        SpecularFactor = vec3(BeckmannDist(NdotH,alpha));\n"
    "    }else if(MaterialTypeSpecular == 7.0){\n"
    "        SpecularFactor = SpecularAshikhminShirley(PxlNormal,Half,NdotH,LightDir,NdotL,VdotN);\n"
    "    }\n"
    "    LightDiffuseColor *= LightDataA.y;\n"
    "    LightSpecularColor *= (SpecularFactor * LightDataA.z);\n"
    "\n"
    "    vec3 componentDiffuse = ConstantOneVec3 - Frensel;\n"
    "    componentDiffuse *= 1.0 - metalness;\n"
    "\n"
    "    TotalLight = (componentDiffuse * ao) * MaterialAlbedoTexture;\n"
    "    TotalLight /= KPI;\n"
    "    TotalLight += LightSpecularColor;\n"
    "    TotalLight *= (LightDiffuseColor * NdotL);\n"
    "\n"
    "    return max(vec3(Glow) * MaterialAlbedoTexture,TotalLight);\n"
    "}\n"
    "vec3 CalcPointLight(vec3 LightPos,vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){\n"
    "    vec3 RawDirection = LightPos - PxlWorldPos;\n"
    "    float Dist = length(RawDirection);\n"
    "    vec3 LightDir = RawDirection / Dist;\n"
    "    vec3 c = CalcLightInternal(LightDir, PxlWorldPos, PxlNormal, uv);\n"
    "    float attenuation = CalculateAttenuation(Dist,1.0);\n"
    "    return c * attenuation;\n"
    "}\n"
    "vec3 CalcSpotLight(vec3 SpotLightDir, vec3 LightPos,vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){\n"
    "    vec3 LightDir = normalize(LightPos - PxlWorldPos);\n"
    "    vec3 c = CalcPointLight(LightPos, PxlWorldPos, PxlNormal, uv);\n"
    "    float cosAngle = dot(LightDir, -SpotLightDir);\n"
    "    float spotEffect = smoothstep(LightDataE.y, LightDataE.x, cosAngle);\n"
    "    return c * spotEffect;\n"
    "}\n"
    "vec3 CalcRodLight(vec3 A, vec3 B,vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){\n"
    "    vec3 BMinusA = B - A;\n"
    "    vec3 CMinusA = PxlWorldPos - A;\n"
    "    float Dist = length(BMinusA);\n"
    "    vec3 _Normal = BMinusA / Dist;\n"
    "    float t = clamp(dot(CMinusA, _Normal / Dist),0.0,1.0);\n" //recheck this: normalize(BMinusA) / length(BMinusA)
    "    vec3 LightPos = A + t * BMinusA;\n"
    "    vec3 c = CalcPointLight(LightPos, PxlWorldPos, PxlNormal, uv);\n"
    "    return c;\n"
    "}\n"
    "void main(){\n"                      //windowX      //windowY
    "    vec2 uv = gl_FragCoord.xy / vec2(ScreenData.z,ScreenData.w);\n"
    "    vec3 PxlNormal = DecodeOctahedron(texture2D(gNormalMap, uv).rg);\n"
    //   this code helps performance, but we have the stencil test for that
    //"    if(distance(PxlNormal,ConstantOneVec3) < 0.01){\n"
    //"        return;\n"
    //"        discard;\n"
    //"    }\n"
    "    vec3 PxlPosition = GetWorldPosition(uv,CameraNear,CameraFar);\n"
    "\n"
    "    vec3 lightCalculation = ConstantZeroVec3;\n"
    "    vec3 LightPosition = vec3(LightDataC.yzw);\n"
    "    vec3 LightDirection = normalize(vec3(LightDataA.w,LightDataB.x,LightDataB.y));\n"
    "\n"
    "    if(LightDataD.w == 0.0){\n"
    "        lightCalculation = CalcLightInternal(normalize(LightPosition - PxlPosition),PxlPosition,PxlNormal,uv);\n"
    "    }else if(LightDataD.w == 1.0){\n"
    "        lightCalculation = CalcPointLight(LightPosition,PxlPosition,PxlNormal,uv);\n"
    "    }else if(LightDataD.w == 2.0){\n"
    "        lightCalculation = CalcLightInternal(LightDirection,PxlPosition,PxlNormal,uv);\n"
    "    }else if(LightDataD.w == 3.0){\n"
    "        lightCalculation = CalcSpotLight(LightDirection,LightPosition,PxlPosition,PxlNormal,uv);\n"
    "    }else if(LightDataD.w == 4.0){\n"
    "        lightCalculation = CalcRodLight(vec3(LightDataA.w,LightDataB.xy),LightDataC.yzw,PxlPosition,PxlNormal,uv);\n"
    "    }\n"
    "    gl_FragData[0].rgb = lightCalculation;\n"
    "}";

#pragma endregion

#pragma region LightingFragGI
epriv::EShaders::lighting_frag_gi =
    "#define MATERIAL_COUNT_LIMIT 255\n"
    "\n"
    "uniform sampler2D gDiffuseMap;\n"
    "uniform sampler2D gNormalMap;\n"
    "uniform sampler2D gDepthMap;\n"
    "uniform sampler2D gSSAOMap;\n"
    "uniform samplerCube irradianceMap;\n"
    "uniform samplerCube prefilterMap;\n"
    "uniform sampler2D brdfLUT;\n"
    "\n"
    "uniform vec4 ScreenData;\n" //x = GIContribution, y = gamma, z = winSize.x, w = winSize.y
    "uniform vec4 materials[MATERIAL_COUNT_LIMIT];\n"//r = MaterialF0Color (packed into float), g = baseSmoothness, b = specularModel, a = diffuseModel
    "\n"
    "varying vec2 texcoords;\n"
    "\n";
epriv::EShaders::lighting_frag_gi += epriv::EShaders::normals_octahedron_compression_functions;
epriv::EShaders::lighting_frag_gi += epriv::EShaders::float_into_2_floats;
epriv::EShaders::lighting_frag_gi +=
    "vec3 SchlickFrenselRoughness(float theta, vec3 _F0,float roughness){\n"
    "    vec3 ret = _F0 + (max(vec3(1.0 - roughness),_F0) - _F0) * pow(1.0 - theta,5.0);\n"
    "    return ret;\n"
    "}\n"
    "void main(){\n"
    //"  //vec2 uv = texcoords;\n" //this cannot be used for non fullscreen quad meshes
    "    vec2 uv = gl_FragCoord.xy / vec2(ScreenData.z,ScreenData.w);\n"
    "\n"
    "    vec3 PxlNormal = DecodeOctahedron(texture2D(gNormalMap, uv).rg);\n"
    //   this code helps performance, but we have the stencil test for that
    //"    if(distance(PxlNormal,ConstantOneVec3) < 0.01){\n"
    //"        return;\n"
    //"        discard;\n"
    //"    }\n"
    "    vec3 MaterialAlbedoTexture = texture2D(gDiffuseMap,uv).rgb;\n"
    "    vec3 PxlWorldPos = GetWorldPosition(uv,CameraNear,CameraFar);\n"
    "    vec3 ViewDir = normalize(CameraPosition - PxlWorldPos);\n"
    "    vec3 R = reflect(-ViewDir, PxlNormal);\n"
    "    float VdotN = max(0.0, dot(ViewDir,PxlNormal));\n"
    "    float matIDandAO = texture2D(gNormalMap,uv).b;\n"
    "    highp int index = int(floor(matIDandAO));\n"
    "    float ssaoValue = 1.0 - texture2D(gSSAOMap,uv).a;\n"
    "    float ao = (fract(matIDandAO)+0.0001) * ssaoValue;\n"//the 0.0001 makes up for the clamp in material class
    "    vec2 stuff = UnpackFloat16Into2Floats(texture2D(gNormalMap,uv).a);\n" //x is metalness, y is smoothness
    "    vec3 MaterialF0 = Unpack3FloatsInto1FloatUnsigned(materials[index].r);\n"
    "    vec3 F0 = mix(MaterialF0, MaterialAlbedoTexture, vec3(stuff.x));\n"
    "    vec3 Frensel = F0;\n"
    "    float roughness = 1.0 - stuff.y;\n"
    "    vec3 irradianceColor = textureCube(irradianceMap, PxlNormal).rgb;\n"
    "    vec3 kS = SchlickFrenselRoughness(VdotN,Frensel,roughness);\n"
    "    vec3 kD = ConstantOneVec3 - kS;\n"
    "    kD *= 1.0 - stuff.x;\n"
    "    vec3 GIContribution = Unpack3FloatsInto1FloatUnsigned(ScreenData.x);\n" //x = diffuse, y = specular, z = global
    "    vec3 GIDiffuse = irradianceColor * MaterialAlbedoTexture * kD * GIContribution.x;\n"
    "\n"
    "    const float MAX_REFLECTION_LOD = 5.0;\n"
    "    vec3 prefilteredColor = textureCubeLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;\n"
    "    vec2 brdf  = texture2D(brdfLUT, vec2(VdotN, roughness)).rg;\n"
    "    vec3 GISpecular = prefilteredColor * (kS * brdf.x + brdf.y) * GIContribution.y;\n"
    "\n"
    "    vec3 TotalIrradiance = (GIDiffuse + GISpecular) * ao;\n"
    "    TotalIrradiance = pow(TotalIrradiance, vec3(1.0 / ScreenData.y));\n" //ScreenData.y is gamma
    "    gl_FragColor += vec4(TotalIrradiance,1.0) * vec4(vec3(GIContribution.z),1.0);\n"
    "}";

#pragma endregion

}
