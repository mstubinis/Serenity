#include "Engine_BuiltInShaders.h"

using namespace Engine;
using namespace std;

/*
GLSL Version      OpenGL Version
1.10              2.0
1.20              2.1
1.30              3.0
1.40              3.1
1.50              3.2
3.30              3.3
4.00              4.0
4.10              4.1
4.20              4.2
4.30              4.3
4.40              4.4
4.50              4.5
*/

#pragma region Declarations
string Shaders::Detail::ShadersManagement::version = "#version 120\n";
string Shaders::Detail::ShadersManagement::float_into_2_floats = "";
string Shaders::Detail::ShadersManagement::determinent_mat3 = "";
string Shaders::Detail::ShadersManagement::normals_octahedron_compression_functions = "";
string Shaders::Detail::ShadersManagement::reconstruct_log_depth_functions = "";
string Shaders::Detail::ShadersManagement::fullscreen_quad_vertex = "";
string Shaders::Detail::ShadersManagement::vertex_basic = "";
string Shaders::Detail::ShadersManagement::vertex_hud = "";
string Shaders::Detail::ShadersManagement::vertex_skybox = "";
string Shaders::Detail::ShadersManagement::smaa_vertex_1 = "";
string Shaders::Detail::ShadersManagement::smaa_frag_1 = "";
string Shaders::Detail::ShadersManagement::smaa_vertex_2 = "";
string Shaders::Detail::ShadersManagement::smaa_frag_2 = "";
string Shaders::Detail::ShadersManagement::smaa_vertex_3 = "";
string Shaders::Detail::ShadersManagement::smaa_frag_3 = "";
string Shaders::Detail::ShadersManagement::smaa_vertex_4 = "";
string Shaders::Detail::ShadersManagement::smaa_frag_4 = "";
string Shaders::Detail::ShadersManagement::fxaa_frag = "";
string Shaders::Detail::ShadersManagement::deferred_frag = "";
string Shaders::Detail::ShadersManagement::deferred_frag_hud = "";
string Shaders::Detail::ShadersManagement::deferred_frag_skybox = "";
string Shaders::Detail::ShadersManagement::copy_depth_frag = "";
string Shaders::Detail::ShadersManagement::cubemap_convolude_frag = "";
string Shaders::Detail::ShadersManagement::cubemap_prefilter_envmap_frag = "";
string Shaders::Detail::ShadersManagement::brdf_precompute = "";
string Shaders::Detail::ShadersManagement::ssao_frag = "";
string Shaders::Detail::ShadersManagement::hdr_frag = "";
string Shaders::Detail::ShadersManagement::godRays_frag = "";
string Shaders::Detail::ShadersManagement::blur_frag = "";
string Shaders::Detail::ShadersManagement::edge_frag = "";
string Shaders::Detail::ShadersManagement::final_frag = "";
string Shaders::Detail::ShadersManagement::lighting_frag = "";
string Shaders::Detail::ShadersManagement::lighting_frag_gi = "";
#pragma endregion

void Shaders::Detail::ShadersManagement::init(){

#pragma region Functions
Shaders::Detail::ShadersManagement::float_into_2_floats = 
    "\n"
    "float Pack2FloatIntoFloat16(float x,float y){\n"
    "    x = clamp(x,0.001,0.99);\n"
    "    y = clamp(y,0.001,0.99);\n"
    "    float _x = (x + 1.0) * 0.5;\n"
    "    float _y = (y + 1.0) * 0.5;\n"
    "    return floor(_x * 100.0) + _y;\n"
    "}\n"
    "vec2 UnpackFloat16Into2Floats(float i){\n"
    "    vec2 res;\n"
    "    res.y = i - floor(i);\n"
    "    res.x = (i - res.y) / 100.0;\n"
    "    res.x = (res.x - 0.5) * 2.0;\n"
    "    res.y = (res.y - 0.5) * 2.0;\n"
    "    return res;\n"
    "}\n"
    "float Pack2FloatIntoFloat32(float x,float y){\n"
    "    x = clamp(x,0.001,0.99);\n"
    "    y = clamp(y,0.001,0.99);\n"
    "    float _x = (x + 1.0) * 0.5;\n"
    "    float _y = (y + 1.0) * 0.5;\n"
    "    return floor(_x * 1000.0) + _y;\n"
    "}\n"
    "vec2 UnpackFloat32Into2Floats(float i){\n"
    "    vec2 res;\n"
    "    res.y = i - floor(i);\n"
    "    res.x = (i - res.y) / 1000.0;\n"
    "    res.x = (res.x - 0.5) * 2.0;\n"
    "    res.y = (res.y - 0.5) * 2.0;\n"
    "    return res;\n"
    "}\n"
    "\n";
Shaders::Detail::ShadersManagement::determinent_mat3 = 
    "\n"
    "float det(mat3 m){\n"
    "    return m[0][0]*(m[1][1]*m[2][2]-m[2][1]*m[1][2])-m[1][0]*(m[0][1]*m[2][2]-m[2][1]*m[0][2])+m[2][0]*(m[0][1]*m[1][2]-m[1][1]*m[0][2]);\n"
    "}\n"
    "\n";
Shaders::Detail::ShadersManagement::reconstruct_log_depth_functions = 
    "\n"
    "vec3 reconstruct_world_pos(vec2 _uv,float _near, float _far){\n"
    "    float log_depth = texture2D(gDepthMap, _uv).r;\n"
    "    float regularDepth = pow(_far + 1.0, log_depth) - 1.0;\n"//log to regular depth
    "\n"  //linearize regular depth
    "    float a = _far / (_far - _near);\n"
    "    float b = _far * _near / (_near - _far);\n"
    "    float linearDepth = (a + b / regularDepth);\n"
    "\n"
    "    vec4 clipSpace = vec4(_uv,linearDepth, 1.0) * 2.0 - 1.0;\n"
    "\n"
    "    \n"//world space it!
    "    vec4 wpos = invVP * clipSpace;\n"
    "    return wpos.xyz / wpos.w;\n"
    "}\n"
    "\n"
    "vec3 reconstruct_view_pos(vec2 _uv,float _near, float _far){\n"
    "    float log_depth = texture2D(gDepthMap, _uv).r;\n"
    "    float regularDepth = pow(_far + 1.0, log_depth) - 1.0;\n"//log to regular depth
    "\n"  //linearize regular depth
    "    float a = _far / (_far - _near);\n"
    "    float b = _far * _near / (_near - _far);\n"
    "    float linearDepth = (a + b / regularDepth);\n"
    "\n"
    "    vec4 clipSpace = invP * vec4(_uv,linearDepth, 1.0) * 2.0 - 1.0;\n"
    "    return clipSpace.xyz / clipSpace.w;\n"
    "}\n"
    "\n";

Shaders::Detail::ShadersManagement::normals_octahedron_compression_functions = 
    "\n"
    "float Round(float x){\n"
    "    return x < 0.0 ? int(x - 0.5) : int(x + 0.5);\n"
    "}\n"
    "vec2 sign_not_zero(vec2 v) {\n"
    "    return vec2(v.x >= 0 ? 1.0 : -1.0,v.y >= 0 ? 1.0 : -1.0);\n"
    "}\n"
    "vec2 EncodeOctahedron(vec3 v) {\n"
    "    if(v.r > 0.9999 && v.g > 0.9999 && v.b > 0.9999)\n"
    "        return vec2(1.0);\n"
    "	 v.xy /= dot(abs(v), vec3(1.0));\n"
    "	 return mix(v.xy, (1.0 - abs(v.yx)) * sign_not_zero(v.xy), step(v.z, 0.0));\n"
    "}\n"
    "vec3 DecodeOctahedron(vec2 n) {\n"
    "    if(n.r > 0.9999 && n.g > 0.9999)\n"
    "        return vec3(1.0);\n"
    "	 vec3 v = vec3(n.xy, 1.0 - abs(n.x) - abs(n.y));\n"
    "	 if (v.z < 0) v.xy = (1.0 - abs(v.yx)) * sign_not_zero(v.xy);\n"
    "	 return normalize(v);\n" //figure this shit out...
    "}\n"
    "vec2 EncodeSpherical(vec3 n){\n"
    "    if(n.r > 0.9999 && n.g > 0.9999 && n.b > 0.9999)\n"
    "        return vec2(1.0);\n"
    "    vec2 encN;\n"
    "    encN.x = atan( n.x, n.y ) * 1.0 / 3.1415926535898;\n"
    "    encN.y = n.z;\n"
    "    encN = encN * 0.5 + 0.5;\n"
    "    return encN;\n"
    "}\n"
    "vec3 DecodeSpherical(vec2 encN){\n"
    "    if(encN.r > 0.9999 && encN.g > 0.9999)\n"
    "        return vec3(1.0);\n"
    "    vec2 ang = encN * 2.0 - 1.0;\n"
    "    vec2 scth;\n"
    "    float ang2 = ang.x * 3.1415926535898;\n"
    "    scth.x = sin(ang2);\n"
    "    scth.y = cos(ang2);\n"
    "    vec2 scphi = vec2( sqrt( 1.0 - ang.y * ang.y ), ang.y );\n"
    "    vec3 n;\n"
    "    n.x = scth.y * scphi.x;\n"
    "    n.y = scth.x * scphi.x;\n"
    "    n.z = scphi.y;\n"
    "    return normalize(n);\n"
    "}\n"
    "vec2 EncodeStereographic(vec3 n){\n"
    "    if(n.r > 0.9999 && n.g > 0.9999 && n.b > 0.9999)\n"
    "        return vec2(1.0);\n"
    "    float scale = 1.7777777777;\n"
    "    vec2 enc = n.xy / (n.z+1.0);\n"
    "    enc /= scale;\n"
    "    enc = enc*0.5+0.5;\n"
    "    return enc;\n"
    "}\n"
    "vec3 DecodeStereographic(vec2 enc){\n"
    "    if(enc.r > 0.9999 && enc.g > 0.9999)\n"
    "        return vec3(1.0);\n"
    "    float scale = 1.7777777777;\n"
    "    vec3 nn = vec3(enc.xy,1.0)*vec3(2.0*scale,2.0*scale,0.0) + vec3(-scale,-scale,1.0);\n"
    "    float g = 2.0 / dot(nn.xyz,nn.xyz);\n"
    "    vec3 n;\n"
    "    n.xy = g*nn.xy;\n"
    "    n.z = g-1.0;\n"
    "    return normalize(n);\n"
    "}\n"
    "vec2 EncodeXYRestoreZ(vec3 n){\n"
    "    if(n.r > 0.9999 && n.g > 0.9999 && n.b > 0.9999){\n"
    "        return vec2(1.0);\n"
    "    }\n"
    "    vec2 enc = vec2(0.0);\n"
    "    enc = vec2(0.5) * n.xy + vec2( 1.0, 1.0 );\n"
    "    enc.x *= n.z < 0.0 ? -1.0 : 1.0;\n"
    "    return enc;\n"
    "}\n"
    "vec3 DecodeXYRestoreZ(vec2 n){\n"
    "    if(n.r > 0.9999 && n.g > 0.9999){\n"
    "        return vec3(1.0);\n"
    "    }\n"
    "    vec3 enc = vec3(0.0);\n"
    "    enc.x = 2.0 * abs(n.r) - 1.0;\n"
    "    enc.y = 2.0 * abs(n.g) - 1.0;\n"
    "    enc.z = n.r < 0.0 ? -1.0 : 1.0;\n"
    "    enc.z *= sqrt(abs(1.0 - enc.x * enc.x - enc.y * enc.y));\n"
    "    return normalize(enc);\n"
    "}\n"
    "\n";

#pragma endregion


#pragma region FullscreenQuadVertex
Shaders::Detail::ShadersManagement::fullscreen_quad_vertex = Shaders::Detail::ShadersManagement::version + 
    "\n"
    "uniform mat4 VP;\n"
    "uniform mat4 Model;\n"
    "uniform vec2 VertexShaderData;\n" //x = outercutoff, y = radius
    "uniform float SpotLight;\n"
    "\n"
    "void main(void){\n"
    "    vec4 vert = gl_Vertex;\n"
    "    mat4 MVP = VP * Model;\n"
    "    if(SpotLight > 0.99){\n"
    "        float opposite = tan(VertexShaderData.x*0.5) * VertexShaderData.y;\n" //outerCutoff might need to be in degrees?
    "        vert.xy *= vec2(opposite/VertexShaderData.y);\n" //might need to switch around x,y,z to fit GL's coordinate system
    "    }\n"
    "    gl_TexCoord[0] = gl_MultiTexCoord0;\n"
    "    gl_Position = MVP * vert;\n"
    "}";
#pragma endregion

  
#pragma region VertexBasic
Shaders::Detail::ShadersManagement::vertex_basic = Shaders::Detail::ShadersManagement::version + 
    "\n"
    "attribute vec3 position;\n"
    "attribute float uv;\n"
    "attribute vec4 normal;\n" //Order is ZYXW
    "attribute vec4 binormal;\n"//Order is ZYXW
    "attribute vec4 tangent;\n"//Order is ZYXW
    "attribute vec4 BoneIDs;\n"
    "attribute vec4 Weights;\n"
    "\n"
    "uniform mat4 VP;\n"
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
    "varying float logz_f;\n"
    "varying float FC_2_f;\n"
    "uniform float fcoeff;\n"
    "\n";
Shaders::Detail::ShadersManagement::vertex_basic += Shaders::Detail::ShadersManagement::float_into_2_floats;
Shaders::Detail::ShadersManagement::vertex_basic +=
    "void main(void){\n"
    "    mat4 BoneTransform = mat4(1.0);\n"
    "    if(AnimationPlaying == 1.0){\n"
    "        BoneTransform  = gBones[int(BoneIDs.x)] * Weights.x;\n"
    "        BoneTransform += gBones[int(BoneIDs.y)] * Weights.y;\n"
    "        BoneTransform += gBones[int(BoneIDs.z)] * Weights.z;\n"
    "        BoneTransform += gBones[int(BoneIDs.w)] * Weights.w;\n"
    "    }\n"
    "    vec4 PosTrans = BoneTransform * vec4(position, 1.0);\n"
    "    vec3 NormalTrans = (BoneTransform * vec4(normal.zyx, 0.0)).xyz;\n"//Order is ZYXW so to bring it to XYZ we need to use ZYX
    "    vec3 BinormalTrans = (BoneTransform * vec4(binormal.zyx, 0.0)).xyz;\n"//Order is ZYXW so to bring it to XYZ we need to use ZYX
    "    vec3 TangentTrans = (BoneTransform * vec4(tangent.zyx, 0.0)).xyz;\n"//Order is ZYXW so to bring it to XYZ we need to use ZYX
    "\n"
    "    mat4 MVP = VP * Model;\n"
    "\n"
    "    gl_Position = MVP * PosTrans;\n"
    "\n"
    "    Normals = normalize(NormalMatrix * NormalTrans);\n"
    "    vec3 Binormals = normalize(NormalMatrix * BinormalTrans);\n"
    "    vec3 Tangents = normalize(NormalMatrix * TangentTrans);\n"
    "\n"
    "    TBN = mat3(Tangents,Binormals,Normals);\n"
    "\n"
    "    WorldPosition = (Model * PosTrans).xyz;\n"
    "\n"
    "    UV = UnpackFloat32Into2Floats(uv);\n"
    "    logz_f = 1.0 + gl_Position.w;\n"
    "    gl_Position.z = (log2(max(1e-6, logz_f)) * fcoeff - 1.0) * gl_Position.w;\n"
    "    FC_2_f = fcoeff * 0.5;\n"
    "}";

#pragma endregion

#pragma region VertexHUD
Shaders::Detail::ShadersManagement::vertex_hud = Shaders::Detail::ShadersManagement::version + 
    "\n"
    "attribute vec3 position;\n"
    "attribute float uv;\n"
    "\n"
    "uniform mat4 VP;\n"
    "uniform mat4 Model;\n"
    "varying vec2 UV;\n";
Shaders::Detail::ShadersManagement::vertex_hud += Shaders::Detail::ShadersManagement::float_into_2_floats;
Shaders::Detail::ShadersManagement::vertex_hud +=
    "void main(void){\n"
    "    mat4 MVP = VP * Model;\n"
    "    UV = UnpackFloat32Into2Floats(uv);\n"
    "    gl_Position = MVP * vec4(position, 1.0);\n"
    "    gl_TexCoord[6] = gl_Position;\n"
    "}";

#pragma endregion

#pragma region VertexSkybox
Shaders::Detail::ShadersManagement::vertex_skybox = Shaders::Detail::ShadersManagement::version + 
    "\n"
    "attribute vec3 position;\n"
    "uniform mat4 VP;\n"
    "varying vec3 UV;\n"
    "void main(void){\n"
    "    UV = position;\n"
    "    gl_Position = VP * vec4(position, 1.0);\n"
    "    gl_Position.z = gl_Position.w;\n"
    "}";
#pragma endregion

#pragma region CubemapConvoludeFrag
Shaders::Detail::ShadersManagement::cubemap_convolude_frag = Shaders::Detail::ShadersManagement::version + 
    "\n"
    "varying vec3 UV;\n"
    "uniform samplerCube cubemap;\n"
    "const float PI = 3.14159265;\n"
    "void main(void){\n"
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
    "    irradiance = PI * irradiance * (1.0 / float(nrSamples));\n"
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
Shaders::Detail::ShadersManagement::cubemap_prefilter_envmap_frag = Shaders::Detail::ShadersManagement::version + 
    "\n"
    "varying vec3 UV;\n"
    "uniform samplerCube cubemap;\n"
    "uniform float roughness;\n"
	"uniform float a2;\n"
    "uniform float PiFourDividedByResSquaredTimesSix;\n"
    "uniform int NUM_SAMPLES;\n"
    "const float PI = 3.14159265;\n"
    "const float PI2 = 6.283185;\n"
    "float DistributionGGX(vec3 Half){\n"
    "    float NdotH2 = Half.z*Half.z;\n"
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
    "            invBase = invBase / 2.0;\n"
    "            n = int(float(n) / 2.0);\n"
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
    "void main(void){\n"
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

Shaders::Detail::ShadersManagement::brdf_precompute = Shaders::Detail::ShadersManagement::version +
    "\n"
    "const float PI2 = 6.283185;\n"
    "uniform int NUM_SAMPLES;\n"
    "float VanDerCorpus(int n, int base){\n"
    "    float invBase = 1.0 / float(base);\n"
    "    float denom   = 1.0;\n"
    "    float result  = 0.0;\n"
    "    for(int i = 0; i < 32; ++i){\n"
    "        if(n > 0){\n"
    "            denom = mod(float(n), 2.0);\n"
    "            result += denom * invBase;\n"
    "            invBase = invBase / 2.0;\n"
    "            n = int(float(n) / 2.0);\n"
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
    "    V.x = sqrt(1.0 - NdotV*NdotV);\n"
    "    V.y = 0.0;\n"
    "    V.z = NdotV;\n"
    "    float A = 0.0;\n"
    "    float B = 0.0;\n"
    "    vec3 N = vec3(0.0, 0.0, 1.0);\n"
    "    float a = roughness*roughness;\n"
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
    "void main(void){\n"
    "    vec2 uv = gl_TexCoord[0].st;\n"
    "    vec2 integratedBRDF = IntegrateBRDF(uv.x, uv.y);\n"
    "    gl_FragColor.rg = integratedBRDF;\n"
    "}";

#pragma endregion

#pragma region FXAA
Shaders::Detail::ShadersManagement::fxaa_frag = Shaders::Detail::ShadersManagement::version + 
    "#define FXAA_REDUCE_MIN (1.0/128.0)\n"
    "#define FXAA_REDUCE_MUL (1.0/8.0)\n"
    "#define FXAA_SPAN_MAX 8.0\n"
    "uniform sampler2D sampler0;\n"
    "uniform sampler2D depthTexture;\n"
    "uniform vec2 resolution;\n"
    "void main(void){\n"
    "   vec2 uv = gl_TexCoord[0].st;\n"
    "   float depth = texture2D(depthTexture,uv);\n"
    "   if(depth >= 0.99999){\n"
    "       gl_FragColor = texture2D(sampler0, uv);\n"
    "       return;\n"
    "   }\n"
    "   vec2 inverse_resolution = vec2(1.0/resolution.x, 1.0/resolution.y);\n"
    "   vec3 rgbNW = texture2D(sampler0, uv + (vec2(-1.0,-1.0)) * inverse_resolution).xyz;\n"
    "   vec3 rgbNE = texture2D(sampler0, uv + (vec2(1.0,-1.0)) * inverse_resolution).xyz;\n"
    "   vec3 rgbSW = texture2D(sampler0, uv + (vec2(-1.0,1.0)) * inverse_resolution).xyz;\n"
    "   vec3 rgbSE = texture2D(sampler0, uv + (vec2(1.0,1.0)) * inverse_resolution).xyz;\n"
    "   vec3 rgbM  = texture2D(sampler0, uv).xyz;\n"
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
    "   dir = min(vec2(FXAA_SPAN_MAX,FXAA_SPAN_MAX),max(vec2(-FXAA_SPAN_MAX,-FXAA_SPAN_MAX),dir * rcpDirMin)) * inverse_resolution;\n"
    "   vec3 rgbA = 0.5 * (texture2D(sampler0, uv   + dir * (1.0/3.0 - 0.5)).xyz + texture2D(sampler0,uv + dir * (2.0/3.0 - 0.5)).xyz);\n"
    "   vec3 rgbB = rgbA * 0.5 + 0.25 * (texture2D(sampler0,uv + dir * - 0.5).xyz + texture2D(sampler0,uv + dir * 0.5).xyz);\n"
    "   float lumaB = dot(rgbB,luma);\n"
    "   if((lumaB < lumaMin) || (lumaB > lumaMax)){\n"
    "       gl_FragColor = vec4(rgbA,1.0);\n"
    "   }\n"
    "   else{\n"
    "       gl_FragColor = vec4(rgbB,1.0);\n"
    "   }\n"
    "}";
#pragma endregion

#pragma region DeferredFrag
Shaders::Detail::ShadersManagement::deferred_frag = Shaders::Detail::ShadersManagement::version + 
    "\n"
    "uniform sampler2D DiffuseTexture;\n"
    "uniform sampler2D NormalTexture;\n"
    "uniform sampler2D GlowTexture;\n"
    "uniform sampler2D SpecularTexture;\n"
    "\n"
    "uniform sampler2D AOTexture;\n"
    "uniform sampler2D MetalnessTexture;\n"
    "uniform sampler2D SmoothnessTexture;\n"
    "uniform float     BaseAO;\n"
    "uniform float     BaseMetalness;\n"
    "uniform float     BaseSmoothness;\n"
    "\n"
    "uniform samplerCube ReflectionTexture;\n"
    "uniform sampler2D   ReflectionTextureMap;\n"
    "uniform samplerCube RefractionTexture;\n"
    "uniform sampler2D   RefractionTextureMap;\n"
    "\n"
    "uniform float       CubemapMixFactor;\n"
    "uniform float       RefractiveIndex;\n"
    "uniform vec3        CameraPosition;\n"
    "\n"
    "uniform float BaseGlow;\n"
    "uniform float matID;\n"
    "\n"
    "uniform int Shadeless;\n"
    "uniform int HasGodsRays;\n"
    "\n"
    "uniform vec3 FirstConditionals;\n" //x = diffuse  y = normals    z = glow
    "uniform vec3 SecondConditionals;\n" //x = specular y = ao z = metal
    "uniform vec3 ThirdConditionals;\n" //x = smoothness y = reflection z = refraction
    "\n"
    "uniform vec4 Object_Color;\n"
    "uniform vec3 Gods_Rays_Color;\n"
    "\n"
    "varying vec3 WorldPosition;\n"
    "varying vec2 UV;\n"
    "varying vec3 Normals;\n"
    "varying mat3 TBN;\n"
    "\n"
    "varying float FC_2_f;\n"
    "varying float logz_f;\n"
    "\n";
Shaders::Detail::ShadersManagement::deferred_frag += Shaders::Detail::ShadersManagement::float_into_2_floats;
Shaders::Detail::ShadersManagement::deferred_frag += Shaders::Detail::ShadersManagement::normals_octahedron_compression_functions;
Shaders::Detail::ShadersManagement::deferred_frag +=
    "vec4 PaintersAlgorithm(vec4 paint, vec4 canvas){\n"
    "    vec4 r = vec4(0.0);\n"
    "    float Alpha = paint.a + canvas.a * (1.0 - paint.a);\n"
    "    r.r = (paint.r * paint.a + canvas.r * canvas.a * (1.0-paint.a)) / Alpha;\n"
    "    r.g = (paint.g * paint.a + canvas.g * canvas.a * (1.0-paint.a)) / Alpha;\n"
    "    r.b = (paint.b * paint.a + canvas.b * canvas.a * (1.0-paint.a)) / Alpha;\n"
    "    r.a = Alpha;\n"
    "    return r;\n"
    "}\n"
    "vec4 Reflection(vec4 d, vec3 cpos, vec3 n, vec3 wpos){\n"
    "    vec4 r = vec4(0.0);\n"
    "    r = textureCube(ReflectionTexture,reflect(n,normalize(cpos - wpos))) * texture2D(ReflectionTextureMap,UV).r;\n"
    "    r.a *= CubemapMixFactor;\n"
    "    r = PaintersAlgorithm(r,d);\n"
    "    return r;\n"
    "}\n"
    "vec4 Refraction(vec4 d, vec3 cpos, vec3 n, vec3 wpos){\n"
    "    vec4 r = vec4(0.0);\n"
    "    r = textureCube(RefractionTexture,refract(n,normalize(cpos - wpos),1.0 / RefractiveIndex)) * texture2D(RefractionTextureMap,UV).r;\n"
    "    r.a *= CubemapMixFactor;\n"
    "    r = PaintersAlgorithm(r,d);\n"
    "    return r;\n"
    "}\n"
    "vec3 CalcBumpedNormal(void){\n"
    "    vec3 normTexture = texture2D(NormalTexture, UV).xyz * 2.0 - 1.0;\n"
    "    return TBN * normalize(normTexture);\n"
    "}\n"
    "void main(void){\n"
    "    gl_FragData[0] = Object_Color;\n"
    "    gl_FragData[1].rg = EncodeOctahedron(normalize(Normals));\n"
    "    gl_FragData[2].r = BaseGlow;\n"
    "    gl_FragData[2].g = 1.0;\n"
    "\n"
    "    float ao = BaseAO;\n"
    "    float metalness = BaseMetalness;\n"
    "    float smoothness = BaseSmoothness;\n"
    "\n"
    "    if(SecondConditionals.y > 0.5){\n"
    "        ao *= texture2D(AOTexture, UV).r;\n"
    "    }\n"
    "    if(SecondConditionals.z > 0.5){\n"
    "        metalness *= texture2D(MetalnessTexture, UV).r;\n"
    "    }\n"
    "    if(ThirdConditionals.x > 0.5){\n"
    "        smoothness *= texture2D(SmoothnessTexture, UV).r;\n"
    "    }\n"
    "    gl_FragData[1].b = matID;\n"
    "    gl_FragData[1].a = Pack2FloatIntoFloat16(metalness,smoothness);\n"
    "    if(FirstConditionals.x > 0.5){ gl_FragData[0] *= texture2D(DiffuseTexture, UV); }\n"
    "    if(FirstConditionals.y > 0.5){ gl_FragData[1].rg = EncodeOctahedron(CalcBumpedNormal()); }\n"
    "    if(ThirdConditionals.y > 0.5){\n"
    "        gl_FragData[0] = Reflection(gl_FragData[0],CameraPosition,gl_FragData[1].rgb,WorldPosition);\n"
    "    }\n"
    "    if(ThirdConditionals.z > 0.5){\n"
    "        gl_FragData[0] = Refraction(gl_FragData[0],CameraPosition,gl_FragData[1].rgb,WorldPosition);\n"
    "    }\n"
    "    if(Shadeless == 0){\n"
    "        if(FirstConditionals.z > 0.5){ \n"
    "            gl_FragData[2].r += texture2D(GlowTexture, UV).r; \n"
    "        }\n"
    "        if(SecondConditionals.x > 0.5){ \n"
    "            gl_FragData[2].g = texture2D(SpecularTexture, UV).r; \n"
    "        }\n"
    "    }\n"
    "    else{\n"
    "        gl_FragData[1].rg = vec2(1.0); \n"
    "    }\n"
    "    if(HasGodsRays == 1){\n"
    "        gl_FragData[3] = (texture2D(DiffuseTexture, UV) * vec4(Gods_Rays_Color,1.0))*0.5;\n"
    "    }\n"
    "    gl_FragDepth = log2(logz_f) * FC_2_f;\n"
    "}";

#pragma endregion

#pragma region DeferredFragHUD
Shaders::Detail::ShadersManagement::deferred_frag_hud = Shaders::Detail::ShadersManagement::version + 
    "\n"
    "uniform sampler2D DiffuseTexture;\n"
    "uniform int DiffuseTextureEnabled;\n"
    "uniform vec4 Object_Color;\n"
    "varying vec2 UV;\n"
    "void main(void){\n"
    "    gl_FragColor = Object_Color;\n"
    "    if(DiffuseTextureEnabled == 1){\n"
    "        gl_FragColor *= texture2D(DiffuseTexture, UV);\n"
    "    }\n"
    "}";

#pragma endregion

#pragma region DeferredFragSkybox
Shaders::Detail::ShadersManagement::deferred_frag_skybox = Shaders::Detail::ShadersManagement::version + 
    "\n"
    "uniform samplerCube Texture;\n"
    "varying vec3 UV;\n"
    "varying vec3 WorldPosition;\n"
    "void main(void){\n"
    "    gl_FragData[0] = textureCube(Texture, UV);\n"
    "    gl_FragData[1].rg = vec2(1.0);\n"
    "    gl_FragData[2].r = 0.0;\n"
    "    gl_FragData[2].b = 0.0;\n"
    "}";

#pragma endregion

#pragma region CopyDepthFrag
Shaders::Detail::ShadersManagement::copy_depth_frag = Shaders::Detail::ShadersManagement::version + 
    "\n"
    "uniform sampler2D gDepthMap;\n"
    "\n"
    "void main(void){\n"
    "    vec2 uv = gl_TexCoord[0].st;\n"
    "    gl_FragDepth = texture2D(gDepthMap,uv);\n"
    "}";
#pragma endregion

#pragma region SSAO
Shaders::Detail::ShadersManagement::ssao_frag = Shaders::Detail::ShadersManagement::version + 
    "\n"
    "uniform sampler2D gNormalMap;\n"
    "uniform sampler2D gRandomMap;\n"
    "uniform sampler2D gMiscMap;\n"
    "uniform sampler2D gLightMap;\n"
    "uniform sampler2D gDepthMap;\n"
    "\n"
    "uniform int doSSAO;\n"
    "uniform int doBloom;\n"
    "uniform float fbufferDivisor;\n"
    "\n"
    "uniform vec3 CameraPosition;\n"
    "\n"
    "uniform vec4 SSAOInfo; //x - radius | y - intensity | z - bias | w - scale\n"
    "\n"
    "uniform int Samples;\n"
    "uniform int NoiseTextureSize;\n"
    "\n"
    "uniform vec2 poisson[32];\n"
    "\n"
    "uniform mat4 invVP;\n"
    "uniform mat4 invP;\n"
    "uniform float nearz;\n"
    "uniform float farz;\n"
    "\n";
Shaders::Detail::ShadersManagement::ssao_frag += Shaders::Detail::ShadersManagement::reconstruct_log_depth_functions;
Shaders::Detail::ShadersManagement::ssao_frag += Shaders::Detail::ShadersManagement::normals_octahedron_compression_functions;
Shaders::Detail::ShadersManagement::ssao_frag +=
    "float occlude(vec2 uv, vec2 offsetUV, vec3 origin, vec3 normal){\n"
    "    vec3 diff = reconstruct_world_pos(uv + offsetUV,nearz,farz) - origin;\n"
    "    vec3 vec = normalize(diff);\n"
    "    float dist = length(diff) * SSAOInfo.w;\n"
    "    return max(0.0, dot(normal,vec) - SSAOInfo.z) * (1.0 / (1.0 + dist)) * SSAOInfo.y;\n"
    "}\n"
    "void main(void){\n"
    "    vec2 uv = gl_TexCoord[0].st * (1.0 / fbufferDivisor);\n"
    "    vec3 worldPosition = reconstruct_world_pos(uv,nearz,farz);\n"
    "    vec3 normal = DecodeOctahedron(texture2D(gNormalMap, uv).rg);\n"
    "    vec2 randomVector = normalize(texture2D(gRandomMap, gl_TexCoord[0].st / NoiseTextureSize).xy * 2.0 - 1.0);\n"
    "\n"
    "    float dist = distance(worldPosition, CameraPosition) + 0.0001; //cuz we dont like divide by zeros ;)\n"
    "    float rad = max(0.35,SSAOInfo.x / dist); //not having max 0.35, etc will make this behave very badly when zoomed far out\n"
    "\n"
    "    if(doSSAO == 1){\n"
    "        if(normal.r > 0.9999 && normal.g > 0.9999 && normal.b > 0.9999){ gl_FragColor.a = 1.0; }\n"
    "        else{\n"
    "            float occlusion = 0.0;\n"
    "            for (int i = 0; i < Samples; ++i) {\n"
    "               vec2 coord1 = reflect(poisson[i], randomVector)*rad;\n"
    "               vec2 coord2 = vec2(coord1.x*0.707 - coord1.y*0.707, coord1.x*0.707 + coord1.y*0.707);\n"
    "               occlusion += occlude(uv, coord1 * 0.25, worldPosition, normal);\n"
    "               occlusion += occlude(uv, coord2 * 0.50, worldPosition, normal);\n"
    "               occlusion += occlude(uv, coord1 * 0.75, worldPosition, normal);\n"
    "               occlusion += occlude(uv, coord2, worldPosition, normal);\n"
    "           }\n"
    "           occlusion /= (Samples*4.0);\n"
    "           gl_FragColor.a = 1.0-occlusion;\n"
    "       }\n"
    "    }\n"
    "    else{\n"
    "        gl_FragColor.a = 1.0;\n"
    "    }\n"
    "    if(doBloom == 1){\n"
    "        float Glow = texture2D(gMiscMap,uv).r;\n"
    "        vec3 lighting = texture2D(gLightMap,uv).rgb;\n"
    "        float brightness = dot(lighting, vec3(0.2126, 0.7152, 0.0722));\n"
    "        if(brightness > 1.3 || Glow > 0.01f){\n"
    "            float brightScale = log(brightness) * 0.7;\n"
    "            float glowScale = Glow * 0.73;\n"
    "            float scale = max(glowScale, brightScale);\n"
    "            gl_FragColor.rgb = (lighting * scale * scale);\n"
    "        }\n"
    "    }\n"
    "    else{\n"
    "        gl_FragColor.rgb = vec3(0.0);\n"
    "    }\n"
    "}";

#pragma endregion

#pragma region HDR
Shaders::Detail::ShadersManagement::hdr_frag = Shaders::Detail::ShadersManagement::version + 
    "\n"
    "uniform sampler2D lightingBuffer;\n"
    "uniform sampler2D bloomBuffer;\n"
    "uniform sampler2D gDiffuseMap;\n"
    "uniform sampler2D gNormalMap;\n"
    "uniform int HasLighting;\n"
    "\n"
    "uniform vec4 HDRInfo; // exposure | HasHDR | HasBloom | HDRAlgorithm\n"
    "\n"
    "vec3 uncharted(vec3 x,float a,float b,float c,float d,float e,float f){\n"
    "    return vec3(((x*(a*x+c*b)+d*e)/(x*(a*x+b)+d*f))-e/f);\n"
    "}\n"
    "\n";
Shaders::Detail::ShadersManagement::hdr_frag += Shaders::Detail::ShadersManagement::normals_octahedron_compression_functions;
Shaders::Detail::ShadersManagement::hdr_frag +=
    "void main(void){\n"
    "    vec2 uv = gl_TexCoord[0].st;\n"
    "    vec3 diffuse = texture2D(gDiffuseMap,uv).rgb;\n"
    "    vec3 lighting = texture2D(lightingBuffer, uv).rgb;\n"
    "    vec3 bloom = texture2D(bloomBuffer, uv).rgb;\n"
    "    vec3 normals = DecodeOctahedron(texture2D(gNormalMap,uv).rg);\n"
    "\n"
    "    if(normals.r > 0.999 && normals.g > 0.999 && normals.b > 0.999 || HasLighting == 0){\n"
    "        lighting = diffuse;\n"
    "    }\n"
    "    else if(HDRInfo.z == 1.0){\n"
    "        lighting += (diffuse * bloom);\n"
    "    }\n"
    "\n"
    "    if(HDRInfo.y == 1.0){\n"
    "        if(HDRInfo.w == 0.0){ // Reinhard tone mapping\n"
    "            lighting = lighting / (lighting + vec3(1.0));\n"
    "        }\n"
    "        else if(HDRInfo.w == 1.0){ //Filmic tone mapping\n"
    "            vec3 x = max(vec3(0), lighting - vec3(0.004));\n"
    "            lighting = (x * (vec3(6.2) * x + vec3(0.5))) / (x * (vec3(6.2) * x + vec3(1.7)) + vec3(0.06));\n"
    "        }\n"
    "        else if(HDRInfo.w == 2.0){ // Exposure tone mapping\n"
    "            lighting = vec3(1.0) - exp(-lighting * HDRInfo.x);\n"
    "        }\n"
    "        else if(HDRInfo.w == 3.0){ // Uncharted tone mapping\n"
    "            float A = 0.15; float B = 0.5; float C = 0.1; float D = 0.2; float E = 0.02; float F = 0.3; float W = 11.2;\n"
    "            lighting = HDRInfo.x * uncharted(lighting,A,B,C,D,E,F);\n"
    "            vec3 white = 1.0 / uncharted( vec3(W),A,B,C,D,E,F );\n"
    "            lighting *= white;\n"
    "        }\n"
    "    }\n"
    "    gl_FragColor = vec4(lighting, 1.0);\n"
    "}";

#pragma endregion

#pragma region Blur
Shaders::Detail::ShadersManagement::blur_frag = Shaders::Detail::ShadersManagement::version + 
    "\n"
    "uniform sampler2D texture;\n"
    "uniform vec4 RGBA;\n"
    "uniform vec2 HV;\n"
    "uniform float fbufferDivisor;\n"
    "\n"
    "vec2 offset[14];\n"
    "\n"
    "uniform float radius;\n"
    "uniform vec4 strengthModifier;\n"
    "\n"
    "float weights[7] = float[](0.028,0.024,0.020,0.016,0.012,0.008,0.004);\n"
    "\n"
    "float gauss[7] = float[](\n"
    "    0.0044299121055113265,\n"
    "    0.00895781211794,\n"
    "    0.0215963866053,\n"
    "    0.0443683338718,\n"
    "    0.0776744219933,\n"
    "    0.115876621105,\n"
    "    0.147308056121\n"
    ");\n"
    "void main(void){\n"
    "    for(int i = 0; i < 7; i++){\n"
    "        offset[i] = vec2(-weights[i] * radius * HV.x, -weights[i] * radius * HV.y);\n"
    "        offset[13-i] = vec2(weights[i] * radius * HV.x, weights[i] * radius * HV.y);\n"
    "    }\n"
    "    vec4 sum = vec4(0.0);\n"
    "    vec2 uv = gl_TexCoord[0].st * (1.0 / fbufferDivisor);\n"
    "\n"
    "    float strengthR = max(1.0, radius * strengthModifier.r);\n"
    "    float strengthG = max(1.0, radius * strengthModifier.g);\n"
    "    float strengthB = max(1.0, radius * strengthModifier.b);\n"
    "    float strengthA = max(1.0, radius * strengthModifier.a);\n"
    "    vec4 strength = vec4(strengthR,strengthG,strengthB,strengthA);\n"
    "    for(int i = 0; i < 7; i++){\n"
    "        sum += texture2D(texture, uv + offset[i])    * gauss[i] * strength;\n"
    "        sum += texture2D(texture, uv + offset[13-i]) * gauss[i] * strength;\n"
    "    }\n"
    "    sum.r += texture2D(texture, uv ).r * 0.159576912161 * strengthR;\n"
    "    sum.g += texture2D(texture, uv ).g * 0.159576912161 * strengthG;\n"
    "    sum.b += texture2D(texture, uv ).b * 0.159576912161 * strengthB;\n"
    "    sum.a += texture2D(texture, uv ).a * 0.159576912161 * strengthA;\n"
    "\n"
    "    if(RGBA.x == 1.0) gl_FragColor.r = sum.r;\n"
    "    if(RGBA.y == 1.0) gl_FragColor.g = sum.g;\n"
    "    if(RGBA.z == 1.0) gl_FragColor.b = sum.b;\n"
    "    if(RGBA.w == 1.0) gl_FragColor.a = sum.a;\n"
    "}";

#pragma endregion

#pragma region GodRays
Shaders::Detail::ShadersManagement::godRays_frag = Shaders::Detail::ShadersManagement::version + 
    "\n"
    "uniform vec4 RaysInfo; //exposure | decay | density | weight\n"
    "\n"
    "uniform vec2 lightPositionOnScreen;\n"
    "uniform sampler2D firstPass;\n"
    "uniform int samples;\n"
    "uniform float fbufferDivisor;\n"
    "\n"
    "uniform int behind;\n"
    "uniform float alpha;\n"
    "\n"
    "void main(void){\n"
    "    if(behind == 0){\n"
    "        vec2 uv = gl_TexCoord[0].st * (1.0 / fbufferDivisor);\n"
    "        vec2 deltaUV = vec2(uv - lightPositionOnScreen);\n"
    "        deltaUV *= 1.0 /  float(samples) * RaysInfo.z;\n"
    "\n"
    "        float illuminationDecay = 1.0;\n"
    "        vec4 totalColor = vec4(0.0);\n"
    "        for(int i=0; i < samples; i++){\n"
    "            uv -= deltaUV / 2.0;\n"
    "            vec4 sample = texture2D(firstPass,uv);\n"
    "            sample *= illuminationDecay * RaysInfo.w;\n"
    "            totalColor += sample;\n"
    "            illuminationDecay *= RaysInfo.y;\n"
    "        }\n"
    "        gl_FragColor = totalColor * alpha;\n"
    "        gl_FragColor *= RaysInfo.x;\n"
    "    }\n"
    "}";

#pragma endregion

#pragma region EdgeDetect
Shaders::Detail::ShadersManagement::edge_frag = Shaders::Detail::ShadersManagement::version + 
    "\n"
    "uniform vec2 gScreenSize;\n"
    "uniform sampler2D texture;\n"
    "uniform float radius;\n"
    "\n"
    "float threshold(float thr1,float thr2 ,float val) {\n"
    "    if (val < thr1) { return 0.0; }\n"
    "    if (val > thr2) { return 1.0; }\n"
    "    return val;\n"
    "}\n"
    "// averaged pixel intensity from 3 color channels\n"
    "float avg_intensity(vec4 pix) {\n"
    "    return (pix.r + pix.g + pix.b)/3.0;\n"
    "}\n"
    "vec4 get_pixel(vec2 coords,float dx,float dy) {\n"
    "    float value = texture2D(texture,coords + vec2(dx, dy)).r;\n"
    "    return vec4(value);\n"
    "}\n"
    "float IsEdge(vec2 coords){\n"
    "    float dxtex = 1.0 / gScreenSize.x;\n"
    "    float dytex = 1.0 / gScreenSize.y;\n"
    "    float pix[9];\n"
    "    int k = -1;\n"
    "    float delta;\n"
    "    // read neighboring pixel intensities\n"
    "    for (int i=-1; i<2; i++) {\n"
    "        for(int j=-1; j<2; j++) {\n"
    "            k++;\n"
    "            pix[k] = avg_intensity(get_pixel(coords,float(i)*dxtex,float(j)*dytex));\n"
    "        }\n"
    "    }\n"
    "    // average color differences around neighboring pixels\n"
    "    delta = (abs(pix[1]-pix[7])+abs(pix[5]-pix[3]) +abs(pix[0]-pix[8])+abs(pix[2]-pix[6]))/2.0;\n"
    "    return threshold(0.15,0.6,clamp(1.8*delta,0.0,1.0));\n"
    "}\n"
    "void main(void){\n"
    "    vec2 uv = gl_TexCoord[0].st;\n"
    "    float edge = IsEdge(gl_TexCoord[0].xy);\n"
    "    gl_FragColor = vec4(edge);\n"
    "}";
#pragma endregion

#pragma region FinalFrag
Shaders::Detail::ShadersManagement::final_frag = Shaders::Detail::ShadersManagement::version + 
    "\n"
    "uniform sampler2D gDiffuseMap;\n"
    "uniform sampler2D gLightMap;\n"
    "uniform sampler2D gMiscMap;\n"
    "uniform sampler2D gGodsRaysMap;\n"
    "uniform sampler2D gBloomMap;\n"
    "uniform sampler2D gNormalMap;\n"
    "\n"
    "uniform int HasSSAO;\n"
    "uniform int HasLighting;\n"
    "uniform int HasHDR;\n"
    "\n"
    "uniform float gamma;\n"
    "\n";
Shaders::Detail::ShadersManagement::final_frag += Shaders::Detail::ShadersManagement::float_into_2_floats;
Shaders::Detail::ShadersManagement::final_frag += Shaders::Detail::ShadersManagement::normals_octahedron_compression_functions;
Shaders::Detail::ShadersManagement::final_frag +=
    "void main(void){\n"
    "    vec2 uv = gl_TexCoord[0].st;\n"
    "    vec3 diffuse = texture2D(gDiffuseMap, uv).rgb;\n"
    "    vec3 rays = texture2D(gGodsRaysMap,uv).rgb;\n"
    "    vec3 hdr = texture2D(gMiscMap,uv).rgb;\n"
    "    vec3 lighting = texture2D(gLightMap, uv).rgb;\n"
    "    vec3 normal = DecodeOctahedron(texture2D(gNormalMap, uv).rg);\n"
    "\n"
    "    vec2 stuff = UnpackFloat16Into2Floats(texture2D(gNormalMap, uv).a);\n"
    "    lighting = hdr;\n"
    "    if(HasSSAO == 1){ \n"
    "        float brightness = dot(lighting, vec3(0.2126, 0.7152, 0.0722));\n"
    "        float ssao = texture2D(gBloomMap,uv).a + 0.0001;\n"
    "        brightness = min(1.0,pow(brightness,0.125));\n"
    "        lighting *= max(brightness, ssao);\n"
    "    }\n"
    "    lighting += rays;\n"
    "    //lighting = pow(lighting, vec3(1.0 / gamma));\n"
    "    gl_FragColor = vec4(lighting,1.0);\n"
    "}";

#pragma endregion

#pragma region LightingFrag
Shaders::Detail::ShadersManagement::lighting_frag = Shaders::Detail::ShadersManagement::version + 
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
    "\n"
    "uniform vec4 ScreenData;\n" //x = near, y = far, z = winSize.x, w = winSize.y
    "uniform vec4 CamPosGamma;\n" //x = camX, y = camY, z = camZ, w = monitorGamma
    "uniform vec4 materials[MATERIAL_COUNT_LIMIT]; //r = UNUSED, g = specPower, b = specularModel, a = diffuseModel\n"
    "\n"
    "uniform mat4 VP;\n"
    "uniform mat4 invVP;\n"
    "uniform mat4 invP;\n"
    "\n";
Shaders::Detail::ShadersManagement::lighting_frag += Shaders::Detail::ShadersManagement::float_into_2_floats;
Shaders::Detail::ShadersManagement::lighting_frag += Shaders::Detail::ShadersManagement::reconstruct_log_depth_functions;
Shaders::Detail::ShadersManagement::lighting_frag += Shaders::Detail::ShadersManagement::normals_octahedron_compression_functions;
Shaders::Detail::ShadersManagement::lighting_frag +=
    "float BeckmannDist(float cos2a, float _alpha, float pi){\n"
    "    float b = (1.0 - cos2a) / (cos2a * _alpha);\n"
    "    return (exp(-b)) / (pi * _alpha * cos2a * cos2a);\n"
    "}\n"
    "float GGXDist(float NdotHSquared, float alphaSquared,float pi){\n"
    "    float denom = (NdotHSquared * (alphaSquared - 1.0) + 1.0);\n"
    "    denom = pi * denom * denom;\n"
    "    return alphaSquared / denom;\n"
    "}\n"
    "float GeometrySchlickGGX(float NdotV, float a){\n"
    "    float k = a / 8.0;\n"
    "    float denom = NdotV * (1.0 - k) + k;\n"
    "    return NdotV / denom;\n"
    "}\n"
    "vec3 SchlickFrensel(float theta, vec3 _F0){\n"
    "    vec3 ret = _F0 + (vec3(1.0) - _F0) * pow(1.0 - theta,5.0);\n"
    "    return ret;\n"
    "}\n"
    "float CalculateAttenuation(float Dist,float LightRadius){\n"
    "   float attenuation =  0.0;\n"
    "   if(LightDataE.z == 0.0){\n" //constant
    "       attenuation = 1.0 / max(1.0 , LightDataB.z);\n"
    "   }\n"
    "   else if(LightDataE.z == 1.0){\n" //distance
    "       attenuation = 1.0 / max(1.0 , Dist);\n"
    "   }\n"
    "   else if(LightDataE.z == 2.0){\n" //distance squared
    "       attenuation = 1.0 / max(1.0 , Dist * Dist);\n"
    "   }\n"
    "   else if(LightDataE.z == 3.0){\n" //constant linear exponent
    "       attenuation = 1.0 / max(1.0 , LightDataB.z + (LightDataB.w * Dist) + (LightDataC.x * Dist * Dist));\n"
    "   }\n"
    "   else if(LightDataE.z == 4.0){\n" //distance radius squared
    "       attenuation = 1.0 / max(1.0 ,pow((Dist / LightRadius) + 1.0,2.0));\n"
    "   }\n"
    "   else if(LightDataE.z == 5.0){\n" //spherical quadratic
    "       attenuation = 1.0 / max(1.0 ,1.0 + ((2.0 / LightRadius) * Dist) + (1.0 / (LightRadius*LightRadius))*(Dist*Dist) );\n"
    "   }\n"
    "   return attenuation;\n"
    "}\n"
    "vec3 CalcLightInternal(vec3 LightDir,vec3 PxlWorldPos,vec3 PxlNormal,vec2 uv){\n"
    "    float Glow = texture2D(gMiscMap,uv).r;\n"
    "    float SpecularStrength = texture2D(gMiscMap,uv).g;\n"
    "    vec3 MaterialAlbedoTexture = texture2D(gDiffuseMap,uv).rgb;\n"
    "    if(PxlNormal.r > 0.9999 && PxlNormal.g > 0.9999 && PxlNormal.b > 0.9999){\n"
    "        return vec3(0.0);\n"
    "    }\n"
    "    vec3 LightDiffuseColor  = LightDataD.xyz;\n"
    "    vec3 LightSpecularColor = LightDataD.xyz * SpecularStrength;\n"
    "    vec3 TotalLight         = vec3(0.0);\n"
    "    vec3 SpecularFactor     = vec3(0.0);\n"
    "\n"
    "    float kPi = 3.14159265;\n"
    "    highp int index = int(texture2D(gNormalMap,uv).b);\n"
    "    vec2 stuff = UnpackFloat16Into2Floats(texture2D(gNormalMap,uv).a);\n"
    "    float metalness = stuff.x;\n"
    "    float smoothness = stuff.y;\n"
    "\n"
    "    vec3 F0 = mix(vec3(0.04), MaterialAlbedoTexture, vec3(metalness));\n"
    "    vec3 Frensel = F0;\n"
    "\n"
    "    float roughness = 1.0 - smoothness;\n"
    "    float alpha = roughness * roughness;\n"
    "\n"
    "    vec3 ViewDir = normalize(CamPosGamma.xyz - PxlWorldPos);\n"
    "    vec3 Half = normalize(LightDir + ViewDir);\n"
    "    float NdotL = max(0.0, dot(PxlNormal, LightDir));\n"
    "    float NdotH = max(0.0, dot(PxlNormal, Half));\n"
    "    float VdotN = max(0.0, dot(ViewDir,PxlNormal));\n"
    "    float VdotH = max(0.0, dot(ViewDir,Half));\n"
    "\n"
    "    if(materials[index].a == 0.0){//this is lambert\n"
    "    }\n"
    "    else if(materials[index].a == 1.0){//this is oren-nayar\n"
    "        float thetaR = acos(VdotN);\n"
    "        float thetaI = acos(NdotL);\n"
    "        float A = 1.0 - 0.5 * (alpha / (alpha + 0.33));\n"
    "        float B = 0.45 * (alpha / (alpha + 0.09));\n"
    "        float gamma = dot(ViewDir - PxlNormal * VdotN, LightDir - PxlNormal * NdotL);\n"
    "        LightDiffuseColor *= (cos(thetaI)) * (A + (B * max(0.0,cos(gamma)) * sin(max(thetaI,thetaR)) * tan(min(thetaI,thetaR))));\n"
    "    }\n"
    "    else if(materials[index].a == 2.0){//this is ashikhmin-shirley\n"
    "        float s = clamp(smoothness,0.01,0.76);\n" //this lighting model has to have some form of roughness in it to look good. cant be 1.0
    "        vec3 A = (28.0 * MaterialAlbedoTexture) / vec3(23.0 * kPi);\n"
    "        //float B = (1.0 - (s * LightDataA.z));\n"
    "        float B = 1.0 - s;\n"
    "        float C = (1.0 - pow((1.0 - (NdotL / 2.0)),5.0));\n"
    "        float D = (1.0 - pow((1.0 - (VdotN / 2.0)),5.0));\n"
    "        LightDiffuseColor *= A * B * C * D;\n"
    "        LightDiffuseColor *= kPi;\n" //i know this isnt proper, but the diffuse component is *way* too dark otherwise...
    "     }\n"
    "     else if(materials[index].a == 3.0){//this is minneart\n"
    "        LightDiffuseColor *= pow(VdotN*NdotL,smoothness);\n"
    "    }\n"
    "\n"
    "    if(materials[index].b == 0.0){\n" // this is blinn phong (non-physical)
    "        float gloss = exp2(10.0 * smoothness + 1.0);\n"
    "        float kS = (8.0 + gloss ) / (8.0 * kPi);\n"
    "        SpecularFactor = vec3(kS * pow(NdotH, gloss));\n"
    "    }\n"
    "    else if(materials[index].b == 1.0){\n" //this is phong (non-physical)
    "        float gloss = exp2(10.0 * smoothness + 1.0);\n"
    "        float kS = (2.0 + gloss ) / (2.0 * kPi);\n"
    "        vec3 Reflect = reflect(-LightDir, PxlNormal);\n"
    "        float VdotR = max(0.0, dot(ViewDir,Reflect));\n"
    "        SpecularFactor = vec3(kS * pow(VdotR, gloss));\n"
    "    }\n"
    "    else if(materials[index].b == 2.0){\n" //this is GGX (physical)
    "        float LdotH = max(0.0, dot(LightDir,Half));\n"
    "        float alphaSqr = alpha * alpha;\n"
    "        float denom = NdotH * NdotH * (alphaSqr - 1.0) + 1.0;\n"
    "        float D = alphaSqr / (kPi * denom * denom);\n"
    "        Frensel = SchlickFrensel(LdotH,F0);\n"
    "        float k = 0.5 * alpha;\n"
    "        float k2 = k * k;\n"
    "        SpecularFactor = max(vec3(0.0), (NdotL * D * Frensel / (LdotH*LdotH*(1.0-k2)+k2)) );\n"
    "     }\n"
    "     else if(materials[index].b == 3.0){\n" //this is Cook-Torrance (physical)
    "         Frensel = SchlickFrensel(VdotH,F0);\n"
    "         float NDF = GGXDist(NdotH*NdotH,alpha*alpha,kPi);\n"
    "         //float G = min(1.0, min(  (2.0 * NdotH * VdotN) / VdotH  ,  (2.0 * NdotH * NdotL) / VdotH  ));\n"
    "         float a = (roughness + 1.0) * (roughness + 1.0);\n"
    "         float G = GeometrySchlickGGX(VdotN,a) * GeometrySchlickGGX(NdotL,a);\n"
    "         vec3 Top = NDF * Frensel * G;\n"
    "         float Bottom = max(4.0 * VdotN * NdotL,0.0);\n"
    "         SpecularFactor = Top / (Bottom + 0.0001);\n"
    "    }\n"
    "    else if(materials[index].b == 4.0){\n" //this is gaussian (physical)
    "        float b = acos(NdotH);\n" //this might also be cos. find out
    "        float fin = b / smoothness;\n"
    "        SpecularFactor = vec3(exp(-fin*fin));\n"
    "    }\n"
    "    else if(materials[index].b == 5.0){\n" //this is beckmann (physical)
    "        SpecularFactor = vec3(BeckmannDist(NdotH,alpha,kPi));\n"
    "    }\n"
    "    else if(materials[index].b == 6.0){\n" //this is ashikhmin-shirley (physical)
    "        //make these controllable uniforms\n"
    "        const float Nu = 1000.0;\n"
    "        const float Nv = 1000.0;\n"
    "\n"
    "        vec3 epsilon = vec3(1.0,0.0,0.0);\n"
    "        vec3 tangent = normalize(cross(PxlNormal,epsilon));\n"
    "        vec3 bitangent = normalize(cross(PxlNormal,tangent));\n"
    "        float hdotT = dot(Half,tangent);\n"
    "        float hDotB = dot(Half,bitangent);\n"
    "        float A = sqrt( (Nu + 1.0) * (Nv + 1.0) );\n"
    "        float B = pow(NdotH,((Nu * hdotT * hdotT + Nv * hDotB * hDotB) / (1.0 - (NdotH * NdotH)) ));\n"
    "        float HdotL = max(0.0, dot(Half, LightDir));\n"
    "        float C = 8.0 * kPi * HdotL * max(NdotL,VdotN);\n"
    "        SpecularFactor = vec3((A * B) / C);\n"
    "    }\n"
    "    LightDiffuseColor *= LightDataA.y;\n"
    "    LightSpecularColor *= (SpecularFactor * LightDataA.z);\n"
    "\n"
    "    vec3 kS = Frensel;\n"
    "    vec3 kD = vec3(1.0) - kS;\n"
    "    kD *= 1.0 - metalness;\n"
    "    vec3 FinalLight = (kD * MaterialAlbedoTexture  / kPi + LightSpecularColor) * LightDiffuseColor * NdotL;\n"
    "    TotalLight = FinalLight;\n"
    "    return max(vec3(Glow) * MaterialAlbedoTexture,TotalLight);\n"
    "}\n"
    "vec3 CalcPointLight(vec3 LightPos,vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){\n"
    "    vec3 LightDir = normalize(LightPos - PxlWorldPos);\n"
    "    float Dist = length(LightPos - PxlWorldPos);\n"
    "    vec3 c = CalcLightInternal(LightDir, PxlWorldPos, PxlNormal, uv);\n"
    "    float attenuation = CalculateAttenuation(Dist,1.0);\n"
    "    return c * attenuation;\n"
    "}\n"
    "vec3 CalcSpotLight(vec3 SpotLightDir, vec3 LightPos,vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){\n"
    "    vec3 LightDir = normalize(LightPos - PxlWorldPos);\n"
    "    vec3 c = CalcPointLight(LightPos, PxlWorldPos, PxlNormal, uv);\n"
    "    float cosAngle = dot(LightDir, -SpotLightDir);\n"
    "    float spotEffect = smoothstep(LightDataE.y, LightDataE.x, cosAngle);   \n"
    "    return c * spotEffect;\n"
    "}\n"
    "vec3 CalcRodLight(vec3 A, vec3 B,vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){\n"
	"    vec3 BMinusA = B - A;\n"
	"    vec3 CMinusA = PxlWorldPos - A;\n"
	"    vec3 N = normalize(BMinusA);\n"
	"    float t = clamp(dot(CMinusA, N / length(BMinusA)),0.0,1.0);\n"
	"    vec3 LightPos = A + t * BMinusA;\n"
    "    vec3 c = CalcPointLight(LightPos, PxlWorldPos, PxlNormal, uv);\n"
    "    return c;\n"
    "}\n"
    "void main(void){\n"
    "    //vec2 uv = gl_TexCoord[0].st; //this cannot be used for point light mesh\n"
    "    vec2 uv = gl_FragCoord.xy / vec2(ScreenData.z,ScreenData.w);\n"
    "\n"
    "    vec3 PxlPosition = reconstruct_world_pos(uv,ScreenData.x,ScreenData.y);\n"
    "    vec3 PxlNormal = DecodeOctahedron(texture2D(gNormalMap, uv).rg);\n"
    "\n"
    "    vec3 lightCalculation = vec3(0.0);\n"
    "    vec3 LightPosition = vec3(LightDataC.yzw);\n"
    "    vec3 LightDirection = normalize(vec3(LightDataA.w,LightDataB.x,LightDataB.y));\n"
    "\n"
    "    if(LightDataD.w == 0.0){\n"
    "        lightCalculation = CalcLightInternal(normalize(LightPosition - PxlPosition),PxlPosition,PxlNormal,uv);\n"
    "    }\n"
    "    else if(LightDataD.w == 1.0){\n"
    "        lightCalculation = CalcPointLight(LightPosition,PxlPosition,PxlNormal,uv);\n"
    "    }\n"
    "    else if(LightDataD.w == 2.0){\n"
    "        lightCalculation = CalcLightInternal(LightDirection,PxlPosition,PxlNormal,uv);\n"
    "    }\n"
    "    else if(LightDataD.w == 3.0){\n"
    "        lightCalculation = CalcSpotLight(LightDirection,LightPosition,PxlPosition,PxlNormal,uv);\n"
    "    }\n"
    "    else if(LightDataD.w == 4.0){\n"
	"        lightCalculation = CalcRodLight(vec3(LightDataA.w,LightDataB.xy),LightDataC.yzw,PxlPosition,PxlNormal,uv);\n"
    "    }\n"
    "    gl_FragData[0].rgb = lightCalculation;\n"
    "}";

#pragma endregion

#pragma region LightingFragGI
Shaders::Detail::ShadersManagement::lighting_frag_gi = Shaders::Detail::ShadersManagement::version + 
    "#define MATERIAL_COUNT_LIMIT 255\n"
    "\n"
    "uniform sampler2D gDiffuseMap;\n"
    "uniform sampler2D gNormalMap;\n"
    "uniform sampler2D gDepthMap;\n"
    "uniform samplerCube irradianceMap;\n"
    "uniform samplerCube prefilterMap;\n"
    "uniform sampler2D brdfLUT;\n"
    "\n"
    "uniform vec4 CamPosGamma;\n" //x = camX, y = camY, z = camZ, w = monitorGamma
    "uniform vec4 ScreenData;\n" //x = near, y = far, z = winSize.x, w = winSize.y
    "uniform vec4 materials[MATERIAL_COUNT_LIMIT]; //r = UNUSED, g = specPower, b = specularModel, a = diffuseModel\n"
    "uniform mat4 VP;\n"
    "uniform mat4 invVP;\n"
    "uniform mat4 invP;\n"
    "\n";
Shaders::Detail::ShadersManagement::lighting_frag_gi += Shaders::Detail::ShadersManagement::float_into_2_floats;
Shaders::Detail::ShadersManagement::lighting_frag_gi += Shaders::Detail::ShadersManagement::reconstruct_log_depth_functions;
Shaders::Detail::ShadersManagement::lighting_frag_gi += Shaders::Detail::ShadersManagement::normals_octahedron_compression_functions;
Shaders::Detail::ShadersManagement::lighting_frag_gi +=
    "vec3 SchlickFrenselRoughness(float theta, vec3 _F0,float roughness){\n"
    "    vec3 ret = _F0 + (max(vec3(1.0 - roughness),_F0) - _F0) * pow(1.0 - theta,5.0);\n"
    "    return ret;\n"
    "}\n"
    "void main(void){\n"
    "    //vec2 uv = gl_TexCoord[0].st; //this cannot be used for point light mesh\n"
    "    vec2 uv = gl_FragCoord.xy / vec2(ScreenData.z,ScreenData.w);\n"
    "\n"
    "    vec3 MaterialAlbedoTexture = texture2D(gDiffuseMap,uv).rgb;\n"
    "    vec3 PxlWorldPos = reconstruct_world_pos(uv,ScreenData.x,ScreenData.y);\n"
    "    vec3 PxlNormal = DecodeOctahedron(texture2D(gNormalMap, uv).rg);\n"
    "\n"
    "    vec3 ViewDir = normalize(CamPosGamma.xyz - PxlWorldPos);\n"
    "    vec3 R = reflect(-ViewDir, PxlNormal);\n"
    "    float VdotN = max(0.0, dot(ViewDir,PxlNormal));\n"
    "    float kPi = 3.14159265;\n"
    "    vec2 stuff = UnpackFloat16Into2Floats(texture2D(gNormalMap,uv).a);\n"
    "    float metalness = stuff.x;\n"
    "    float smoothness = stuff.y;\n"
    "    vec3 F0 = mix(vec3(0.04), MaterialAlbedoTexture, vec3(metalness));\n"
    "    vec3 Frensel = F0;\n"
    "    float roughness = 1.0 - smoothness;\n"
    "    vec3 GIDiffuse = textureCube(irradianceMap, PxlNormal).rgb;\n"
    "    vec3 kS1 = SchlickFrenselRoughness(VdotN,Frensel,roughness);\n"
    "    vec3 kD1 = vec3(1.0) - kS1;\n"
    "    kD1 *= 1.0 - metalness;\n"
    "    vec3 AmbientIrradiance = GIDiffuse * MaterialAlbedoTexture;\n"
    "\n"
    "    const float MAX_REFLECTION_LOD = 5.0;\n"
    "    vec3 prefilteredColor = textureCubeLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;\n"
    "    vec2 brdf  = texture2D(brdfLUT, vec2(VdotN, roughness)).rg;\n"
    "    vec3 GISpecular = prefilteredColor * (kS1 * brdf.x + brdf.y);\n"
    "\n"
    "    vec3 TotalIrradiance = (kD1 * AmbientIrradiance + GISpecular);\n" //* ao
    "    gl_FragColor += vec4(TotalIrradiance,1.0);\n"
    "}";

#pragma endregion

}
