
#include <serenity/renderer/opengl/glsl/Common.h>
#include <serenity/renderer/opengl/Extensions.h>
#include <serenity/resources/shader/ShaderHelper.h>
#include <serenity/resources/material/MaterialEnums.h>
#include <serenity/renderer/opengl/State.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>

void Engine::priv::opengl::glsl::Common::convert(std::string& code, uint32_t versionNumber) {

#pragma region constants
    if (ShaderHelper::lacksDefinition(code, "ConstantOneVec3", "const vec3 ConstantOneVec3")) {
        ShaderHelper::insertStringAtLine(code, "const vec3 ConstantOneVec3 = vec3(1.0,1.0,1.0);\n", 1);
    }
    if (ShaderHelper::lacksDefinition(code, "ConstantOneVec2", "const vec2 ConstantOneVec2")) {
        ShaderHelper::insertStringAtLine(code, "const vec2 ConstantOneVec2 = vec2(1.0,1.0);\n", 1);
    }
    if (ShaderHelper::lacksDefinition(code, "ConstantAlmostOneVec3", "const vec3 ConstantAlmostOneVec3")) {
        ShaderHelper::insertStringAtLine(code, "const vec3 ConstantAlmostOneVec3 = vec3(0.9999,0.9999,0.9999);\n", 1);
    }
    if (ShaderHelper::lacksDefinition(code, "ConstantAlmostOneVec2", "const vec2 ConstantAlmostOneVec2")) {
        ShaderHelper::insertStringAtLine(code, "const vec2 ConstantAlmostOneVec2 = vec2(0.9999,0.9999);\n", 1);
    }
    if (ShaderHelper::lacksDefinition(code, "ConstantZeroVec3", "const vec3 ConstantZeroVec3")) {
        ShaderHelper::insertStringAtLine(code, "const vec3 ConstantZeroVec3 = vec3(0.0,0.0,0.0);\n", 1);
    }
    if (ShaderHelper::lacksDefinition(code, "ConstantZeroVec2", "const vec2 ConstantZeroVec2")) {
        ShaderHelper::insertStringAtLine(code, "const vec2 ConstantZeroVec2 = vec2(0.0,0.0);\n", 1);
    }
    if (ShaderHelper::lacksDefinition(code, "KPI", "const float KPI")) {
        ShaderHelper::insertStringAtLine(code, "const float KPI = 3.1415926535898;\n", 1);
    }
#pragma endregion

#pragma region use sampler
    const bool is_bindless_supported = Engine::priv::OpenGLExtensions::isBindlessTexturesSupported();

    const std::array<const std::string, 4> types {
        "1D",
        "2D",
        "3D",
        "Cube",
    };
    auto lambda_sampler_add_code = [&](const std::string& type) {
        const auto samplerType = "sampler" + type;
        const auto funcName = "USE_SAMPLER_" + type;
        //if (is_bindless_supported) {
        //    ShaderHelper::insertStringAtLine(code, 
        //        samplerType + " " + funcName + "(uint64_t inHandle){ //generated\n"
        //        "    return " + samplerType + "(inHandle);\n"
        //        "}\n"
        //    , 1);
        //} else {
            ShaderHelper::insertStringAtLine(code, 
                samplerType + " " + funcName + "(" + samplerType + " inSampler) { //generated\n"
                "    return inSampler;\n"
                "}\n"
            , 1);
        //}
    };
    auto lambda_sampler_add_code_type = [&](const std::string& type) {
        if (is_bindless_supported) {
            boost::replace_all(code, "SAMPLER_TYPE_" + type, "sampler" + type);
        }else{
            boost::replace_all(code, "SAMPLER_TYPE_" + type, "sampler" + type);
        }
    };

    if (ShaderHelper::sfind(code, "USE_SAMPLER_")) {
        for (const auto& type : types) {
            if (ShaderHelper::sfind(code, "USE_SAMPLER_" + type) && !ShaderHelper::sfind(code, "sampler" + type + " USE_SAMPLER_" + type)) {
                lambda_sampler_add_code(type);
            }
        }
    }
    if (ShaderHelper::sfind(code, "SAMPLER_TYPE_")) {
        for (const auto& type : types) {
            if (ShaderHelper::sfind(code, "SAMPLER_TYPE_" + type)) {
                lambda_sampler_add_code_type(type);
            }
        }
    }
#pragma endregion

#pragma region Hammersley
    if (ShaderHelper::lacksDefinition(code, "HammersleySequence(", "vec2 HammersleySequence(")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec2 HammersleySequence(int i, int N){
    return vec2(float(i) / float(N), VanDerCorpus(i));
}
)", 1);
    }
#pragma endregion

#pragma region VanDerCorpus
    if (ShaderHelper::lacksDefinition(code, "VanDerCorpus(", "float VanDerCorpus(")){
        //2.3283064365386963e-10 = 0x100000000
        const auto version = Engine::priv::OpenGLState::getHighestGLSLVersion();
        std::string vanDerCorpus;
        if (!version.empty() && std::stoi(version) >= 130) {
            vanDerCorpus = R"(
float VanDerCorpus(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}
)";
        } else {
            vanDerCorpus = R"(
float VanDerCorpus(int n){
    float invBase = 0.5;
    float denom   = 1.0;
    float result  = 0.0;
    for (int i = 0; i < 32; ++i) {
        if (n > 0) {
            denom    = mod(float(n), 2.0);
            result  += denom * invBase;
            invBase *= 0.5;
            n        = int(float(n) * 0.5);
        }
    }
    return result;
}
)";
        }
        ShaderHelper::insertStringAtLine(code, std::move(vanDerCorpus), 1);
    }
#pragma endregion

#pragma region normal map
    if (ShaderHelper::sfind(code, "CalcBumpedNormal(") || ShaderHelper::sfind(code, "CalcBumpedNormalCompressed(") || ShaderHelper::sfind(code, "CalcBumpedNormalLOD(") || ShaderHelper::sfind(code, "CalcBumpedNormalCompressedLOD(")) {
        if (!ShaderHelper::sfind(code, "vec3 CalcBumpedNormal(")) {
            if (ShaderHelper::sfind(code, "varying mat3 TBN;")) {
                boost::replace_all(code, "varying mat3 TBN;", "");
            }
            ShaderHelper::insertStringAtLine(code, R"(
varying mat3 TBN;
vec3 CalcBumpedNormal(vec2 inUVs, sampler2D inTexture){
    vec3 t_ = (texture2D(inTexture, inUVs).xyz) * 2.0 - 1.0;
    return normalize(TBN * t_);
}
vec3 CalcBumpedNormalCompressed(vec2 inUVs, sampler2D inTexture){
    vec2 t_ = (texture2D(inTexture, inUVs).yx) * 2.0 - 1.0;
    float z_ = sqrt(1.0 - t_.x * t_.x - t_.y * t_.y);
    vec3 normal = vec3(t_.xy, z_);
    return normalize(TBN * normal);
}
vec3 CalcBumpedNormalLOD(vec2 inUVs, sampler2D inTexture, in float lod){
    vec3 t_ = (texture2DLod(inTexture, inUVs, lod).xyz) * 2.0 - 1.0;
    return normalize(TBN * t_);
}
vec3 CalcBumpedNormalCompressedLOD(vec2 inUVs, sampler2D inTexture, in float lod){
    vec2 t_ = (texture2DLod(inTexture, inUVs, lod).yx) * 2.0 - .0;
    float z_ = sqrt(1.0 - t_.x * t_.x - t_.y * t_.y);
    vec3 normal = vec3(t_.xy, z_);
    return normalize(TBN * normal);
}
vec3 CalcBumpedNormal(vec2 inUVs, sampler2D inTexture, float scale){
    vec3 t_ = mix(vec3(0.0, 0.0, 1.0), (texture2D(inTexture, inUVs).xyz) * 2.0 - 1.0, scale);
    return normalize(TBN * t_);
}
vec3 CalcBumpedNormalCompressed(vec2 inUVs, sampler2D inTexture, float scale){
    vec2 t_ = (texture2D(inTexture, inUVs).yx) * 2.0 - 1.0;
    float z_ = sqrt(1.0 - t_.x * t_.x - t_.y * t_.y);
    vec3 normal = mix(vec3(0.0, 0.0, 1.0), vec3(t_.xy, z_), scale);
    return normalize(TBN * normal);
}
vec3 CalcBumpedNormalLOD(vec2 inUVs, sampler2D inTexture, in float lod, float scale){
    vec3 t_ = mix(vec3(0.0, 0.0, 1.0), (texture2DLod(inTexture, inUVs, lod).xyz) * 2.0 - 1.0, scale);
    return normalize(TBN * t_);
}
vec3 CalcBumpedNormalCompressedLOD(vec2 inUVs, sampler2D inTexture, in float lod, float scale){
    vec2 t_ = (texture2DLod(inTexture, inUVs, lod).yx) * 2.0 - 1.0;
    float z_ = sqrt(1.0 - t_.x * t_.x - t_.y * t_.y);
    vec3 normal = mix(vec3(0.0, 0.0, 1.0), vec3(t_.xy, z_), scale);
    return normalize(TBN * normal);
}
)"
            , 1);
        }
    }
#pragma endregion

#pragma region painters algorithm
    if (ShaderHelper::lacksDefinition(code, "PaintersAlgorithm(", "vec4 PaintersAlgorithm(")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec4 PaintersAlgorithm(vec4 paint, vec4 canvas){
    float alpha = paint.a + canvas.a * (1.0 - paint.a);
    vec4 ret = ((paint * paint.a + canvas * canvas.a * (1.0 - paint.a)) / alpha);
    ret.a = alpha;
    return ret;
}
)", 1);
    }
#pragma endregion

#pragma region invert color
    if (ShaderHelper::lacksDefinition(code, "InvertColor(", "vec4 InvertColor(")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec4 InvertColor(vec4 color){
    return vec4(vec4(1.0) - color);
}
)", 1);
    }
#pragma endregion

#pragma region invert color 255
    if (ShaderHelper::lacksDefinition(code, "InvertColor255(", "vec4 InvertColor255(")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec4 InvertColor255(vec4 color){
    return vec4(vec4(255.0) - color);
}
)", 1);
    }
#pragma endregion

#pragma region range to 255
    if (ShaderHelper::lacksDefinition(code, "RangeTo255(", "vec4 RangeTo255(")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec4 RangeTo255(vec4 color){
    return color * 255.0;
}
)", 1);
    }
#pragma endregion

#pragma region range to 1
    if (ShaderHelper::lacksDefinition(code, "RangeTo1(", "vec4 RangeTo1(")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec4 RangeTo1(vec4 color){
    return color * 0.00392156862;
}
)", 1);
    }
#pragma endregion

#pragma region UBO
    if (ShaderHelper::sfind(code, "CameraView") || ShaderHelper::sfind(code, "CameraProj") || ShaderHelper::sfind(code, "CameraViewProj") ||
    ShaderHelper::sfind(code, "CameraPosition") || ShaderHelper::sfind(code, "CameraInvView") || ShaderHelper::sfind(code, "CameraInvProj") ||
    ShaderHelper::sfind(code, "CameraInvViewProj") || ShaderHelper::sfind(code, "CameraNear") || ShaderHelper::sfind(code, "CameraFar") ||
    ShaderHelper::sfind(code, "CameraInfo1") || ShaderHelper::sfind(code, "CameraInfo2") || ShaderHelper::sfind(code, "CameraViewVector") ||
    ShaderHelper::sfind(code, "CameraRealPosition") || ShaderHelper::sfind(code, "CameraInfo3") || ShaderHelper::sfind(code, "ScreenInfo") || 
    ShaderHelper::sfind(code, "RendererInfo1") || ShaderHelper::sfind(code, "RendererInfo2") || ShaderHelper::sfind(code, "LogFCoefficient")) {
        if (versionNumber >= 140) { //UBO only supported at 140 or above
            if (!ShaderHelper::sfind(code, "layout (std140) uniform Camera //generated")) {
                ShaderHelper::insertStringAtLine(code, R"(
layout (std140) uniform Camera //generated
{
    mat4 CameraView;
    mat4 CameraProj;
    mat4 CameraViewProj;
    mat4 CameraInvView;
    mat4 CameraInvProj;
    mat4 CameraInvViewProj;
    vec4 CameraInfo1;
    vec4 CameraInfo2;
    vec4 CameraInfo3;
    vec4 ScreenInfo;
    vec4 RendererInfo1;
    vec4 RendererInfo2;
};
vec3 CameraPosition = CameraInfo1.xyz;
vec3 CameraViewVector = CameraInfo2.xyz;
vec3 CameraRealPosition = CameraInfo3.xyz;
float CameraNear = CameraInfo1.w;
float CameraFar = CameraInfo2.w;
float LogFCoefficient = CameraInfo3.w;
)", 1);
            }
        }else{ //no UBO's, just add a bunch of uniforms
            if (!ShaderHelper::sfind(code, "uniform mat4 CameraView; //generated")) {
                ShaderHelper::insertStringAtLine(code, R"(
uniform mat4 CameraView; //generated;
uniform mat4 CameraProj;
uniform mat4 CameraViewProj;
uniform mat4 CameraInvView;
uniform mat4 CameraInvProj;
uniform mat4 CameraInvViewProj;
uniform vec4 CameraInfo1;
uniform vec4 CameraInfo2;
uniform vec4 CameraInfo3;
uniform vec4 ScreenInfo;
uniform vec4 RendererInfo1;
uniform vec4 RendererInfo2;
vec3 CameraPosition = CameraInfo1.xyz;
vec3 CameraViewVector = CameraInfo2.xyz;
vec3 CameraRealPosition = CameraInfo3.xyz;
float CameraNear = CameraInfo1.w;
float CameraFar = CameraInfo2.w;
float LogFCoefficient = CameraInfo3.w;
)", 1);
            }
        }
    }
#pragma endregion

#pragma region material constants
    if (ShaderHelper::sfind(code, "USE_MAX_MATERIAL_LAYERS_PER_COMPONENT") && !ShaderHelper::sfind(code, "//USE_MAX_MATERIAL_LAYERS_PER_COMPONENT")) {
        boost::replace_all(code, "USE_MAX_MATERIAL_LAYERS_PER_COMPONENT", "#define MAX_MATERIAL_LAYERS_PER_COMPONENT " + std::to_string(MAX_MATERIAL_LAYERS_PER_COMPONENT) + '\n');
    }
    if (ShaderHelper::sfind(code, "USE_MAX_MATERIAL_COMPONENTS") && !ShaderHelper::sfind(code, "//USE_MAX_MATERIAL_COMPONENTS")) {
        boost::replace_all(code, "USE_MAX_MATERIAL_COMPONENTS", "#define MAX_MATERIAL_COMPONENTS " + std::to_string(MAX_MATERIAL_COMPONENTS) + '\n');
    }
    if (ShaderHelper::sfind(code, "USE_MAX_LIGHTS_PER_PASS") && !ShaderHelper::sfind(code, "//USE_MAX_LIGHTS_PER_PASS")) {
        boost::replace_all(code, "USE_MAX_LIGHTS_PER_PASS", "#define MAX_LIGHTS_PER_PASS " + std::to_string(MAX_MATERIAL_COMPONENTS) + '\n');
    }
#pragma endregion
}