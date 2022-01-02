
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
float SSAOExecute(sampler2D depthMap, vec2 uvs, int numSamples, int noiseTextureSize, vec2 randomVec, float radius, vec3 position, vec3 normal, float intensity, float bias, float scl) {
    float res = 0.0;
    for (int i = 0; i < numSamples; ++i) {
        vec2 coord1 = reflect(SSAOPoisson[i] * noiseTextureSize, randomVec) * radius;
        vec2 coord2 = vec2(coord1.x * 0.707 - coord1.y * 0.707, coord1.x * 0.707 + coord1.y * 0.707);
        res += SSAOOcclude(USE_SAMPLER_2D(depthMap), uvs + (coord1 * 0.25), position, normal, intensity, bias, scl);
        res += SSAOOcclude(USE_SAMPLER_2D(depthMap), uvs + (coord2 * 0.50), position, normal, intensity, bias, scl);
        res += SSAOOcclude(USE_SAMPLER_2D(depthMap), uvs + (coord1 * 0.75), position, normal, intensity, bias, scl);
        res += SSAOOcclude(USE_SAMPLER_2D(depthMap), uvs + (coord2 * 1.00), position, normal, intensity, bias, scl);
    }
    return res / (numSamples * 4.0);
}
)", "void main(");
        }
#pragma endregion

#pragma region Occlude function
        if (ShaderHelper::lacksDefinition(code, "SSAOOcclude(", "float SSAOOcclude(")) {
            ShaderHelper::insertStringRightBeforeLineContent(code, R"(
float SSAOOcclude(sampler2D depthMap, vec2 offsetUV, vec3 origin, vec3 normal, float intensity, float bias, float scl) {
    vec3 ViewPos          = GetViewPosition(USE_SAMPLER_2D(depthMap), offsetUV, CameraNear, CameraFar);
    vec3 PositionOffset   = ViewPos - origin;
    float Length          = length(PositionOffset);
    vec3 VectorNormalized = PositionOffset / Length;
    float Dist            = Length * scl;
    float attenuation     = 1.0 / (1.0 + Dist);
    float angleMath       = max(0.0, dot(normal, VectorNormalized) - bias);
    return angleMath * attenuation * intensity;
}
)", "float SSAOExecute(");
        }
#pragma endregion

    }
}