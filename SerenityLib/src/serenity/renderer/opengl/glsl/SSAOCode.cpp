
#include <serenity/renderer/opengl/glsl/SSAOCode.h>

#include <serenity/renderer/opengl/Extensions.h>
#include <serenity/resources/shader/ShaderHelper.h>
#include <serenity/resources/material/MaterialEnums.h>

#include <boost/algorithm/string/replace.hpp>

void Engine::priv::opengl::glsl::SSAOCode::convert(std::string& code, uint32_t versionNumber, ShaderType shaderType) {
    if (shaderType == ShaderType::Fragment) {

#pragma region SSAO function
        if (ShaderHelper::lacksDefinition(code, "SSAOExecute(", "float SSAOExecute(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
float SSAOExecute(sampler2D inDepthSampler, vec2 in_uvs, int in_numSamples, int in_noiseTextureSize, vec2 in_randomVector, float in_radius, vec3 in_position, vec3 in_normals, float in_intensity, float in_bias, float in_scale){ //generated
    float res = 0.0;
    for (int i = 0; i < in_numSamples; ++i) {
        vec2 coord1 = reflect(SSAOPoisson[i].xy * vec2(in_noiseTextureSize), in_randomVector) * in_radius;
        vec2 coord2 = vec2(coord1.x * 0.707 - coord1.y * 0.707, coord1.x * 0.707 + coord1.y * 0.707);
        res += SSAOOcclude(inDepthSampler, in_uvs + (coord1 * 0.25), in_position, in_normals, in_intensity, in_bias, in_scale);
        res += SSAOOcclude(inDepthSampler, in_uvs + (coord2 * 0.50), in_position, in_normals, in_intensity, in_bias, in_scale);
        res += SSAOOcclude(inDepthSampler, in_uvs + (coord1 * 0.75), in_position, in_normals, in_intensity, in_bias, in_scale);
        res += SSAOOcclude(inDepthSampler, in_uvs + coord2,          in_position, in_normals, in_intensity, in_bias, in_scale);
    }
    res /= (in_numSamples * 4.0);
    return res;
}
)", "void main(");
        }
#pragma endregion

#pragma region Occlude function
        if (ShaderHelper::lacksDefinition(code, "SSAOOcclude(", "float SSAOOcclude(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
float SSAOOcclude(sampler2D inDepthSampler, vec2 in_offsetUV, vec3 in_origin, vec3 in_normal, in float in_intensity, in float in_bias, in float in_scale){//generated
    vec3 ViewPos          = GetViewPosition(inDepthSampler, in_offsetUV, CameraNear, CameraFar);
    vec3 PositionOffset   = ViewPos - in_origin;
    float Length          = length(PositionOffset);
    vec3 VectorNormalized = PositionOffset / Length;
    float Dist            = Length * in_scale;
    float attenuation     = 1.0 / (1.0 + Dist);
    float angleMath       = max(0.0, dot(in_normal, VectorNormalized) - in_bias);
    return angleMath * attenuation * in_intensity;
}
)", "float SSAOExecute(");
        }
#pragma endregion

#pragma region Poisson Data
        if (ShaderHelper::lacksDefinition(code, "SSAOPoisson[", "vec2 SSAOPoisson[")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
const vec2 SSAOPoisson[16] = vec2[](
    vec2(1.0, 0.0), vec2(-1.0, 0.0), vec2(0.0, 1.0), vec2(0.0, -1.0),
    vec2(-0.707, 0.707), vec2(0.707, -0.707), vec2(-0.707, -0.707), vec2(0.707, 0.707),
    vec2(-0.375, 0.927), vec2(0.375, -0.927), vec2(-0.375, -0.927), vec2(0.375, 0.927),
    vec2(-0.927, 0.375), vec2(0.927, -0.375), vec2(-0.927, -0.375), vec2(0.927, 0.375)
);
)", "float SSAOOcclude(");
        }
#pragma endregion
    }
}