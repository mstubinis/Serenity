
#include <serenity/renderer/opengl/glsl/Shadows.h>
#include <serenity/lights/Lights.h>

#include <serenity/renderer/opengl/Extensions.h>
#include <serenity/resources/shader/ShaderHelper.h>

#include <boost/algorithm/string/replace.hpp>

void Engine::priv::opengl::glsl::Shadows::convert(std::string& code, uint32_t versionNumber, ShaderType shaderType) {
    if (ShaderHelper::lacksDefinition(code, "ShadowCalculationLightingShader(", "float ShadowCalculationLightingShader(")) {
        std::string shadowCode = R"(
float ShadowCalculationLightingShader(vec3 PxlViewPos, vec3 LightDir, vec3 PxlNormal, vec4 ShadowPxlWorldPos) {
    float shadow = 1.0;
    for (int i = 0; i < NUM_CASCADES; i++) {
        if (-PxlViewPos.z <= -uCascadeEndClipSpace[i]) {
            vec4 FragPosLightSpace = uLightMatrix[i] * ShadowPxlWorldPos;
            shadow = ShadowCalculation(i, FragPosLightSpace, PxlNormal, LightDir);
            if (i < NUM_CASCADES - 1) {
                float fade = clamp((1.0 - PxlViewPos.z / uCascadeEndClipSpace[i]) / 0.05, 0.0, 1.0);
                if (fade < 1.0) {
                    vec4 NextFragPosLightSpace = uLightMatrix[i + 1] * ShadowPxlWorldPos;
                    float nextShadow = ShadowCalculation(i + 1, NextFragPosLightSpace, PxlNormal, LightDir);
                    shadow = mix(nextShadow, shadow, fade);
                }
            }
            break;
        }
    }
    return shadow;
}
)";
        ShaderHelper::insertStringRightBeforeLineContent(code, std::move(shadowCode), "vec3 CalcLightInternal"); //works for both CalcLightInternal( and CalcLightInternalBasic(
    }


    if (ShaderHelper::lacksDefinition(code, "ShadowCalculation(", "float ShadowCalculation(")) {
        std::string shadowCode = "const int NUM_CASCADES = " + std::to_string(int(DIRECTIONAL_LIGHT_NUM_CASCADING_SHADOW_MAPS)) + ";\n";
        shadowCode += R"(
uniform sampler2D uShadowTexture[NUM_CASCADES];
uniform float uCascadeEndClipSpace[NUM_CASCADES];
uniform int uShadowEnabled;
uniform vec2 uShadowTexelSize;
float ShadowCalculation(int inCascadeIndex, vec4 inFragPosLightSpace, vec3 inNormal, vec3 inLightDir){
    if (uShadowEnabled == 0) {
        return 1.0;
    }
    vec3 projCoords = inFragPosLightSpace.xyz / inFragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0) {
        return 1.0;
    }
    float bias = max((0.002 + (inCascadeIndex * 0.001515)) * (1.0 - dot(inNormal, inLightDir)), 0.0002);
    float shadow = SampleShadowLinearPCF(USE_SAMPLER_2D(uShadowTexture[inCascadeIndex]), projCoords.xy, projCoords.z - bias, uShadowTexelSize);
    return shadow;
}
)";
        ShaderHelper::insertStringAtLine(code, std::move(shadowCode), 1);
    }
    // DO NOT make these constants uniforms, that will force dynamic branching on these loops and kill performance
    if (ShaderHelper::lacksDefinition(code, "SampleShadowLinearPCF(", "float SampleShadowLinearPCF(")) {
        ShaderHelper::insertStringAtLine(code, R"(
float SampleShadowLinearPCF(sampler2D shadowMap, vec2 uvs, float compare, vec2 texelSize){
    const float NUM_SAMPLES = 3.0;
    const float SAMPLES_START = (NUM_SAMPLES - 1.0) / 2.0;
    const float NUM_SAMPLES_SQUARED = NUM_SAMPLES * NUM_SAMPLES;
    float result = 0.0;
    for (float y = -SAMPLES_START; y <= SAMPLES_START; y += 1.0) {
        for (float x = -SAMPLES_START; x <= SAMPLES_START; x += 1.0) {
            vec2 uvsOffset = vec2(x, y) * texelSize;
            result += SampleShadowLinear(shadowMap, uvs + uvsOffset, compare, texelSize);
        }
    }
    return result / NUM_SAMPLES_SQUARED;
}
)", 1);
    }
    if (ShaderHelper::lacksDefinition(code, "SampleShadowLinear(", "float SampleShadowLinear(")) {
        ShaderHelper::insertStringAtLine(code, R"(
float SampleShadowLinear(sampler2D shadowMap, vec2 uvs, float compare, vec2 texelSize){
    vec2 pixelPosition = uvs / texelSize + vec2(0.5);
    vec2 fractPart = fract(pixelPosition);
    vec2 startTexel = (pixelPosition - fractPart) * texelSize;

    float blTexel = SampleShadow(shadowMap, startTexel, compare);
    float brTexel = SampleShadow(shadowMap, startTexel + vec2(texelSize.x, 0.0), compare);
    float tlTexel = SampleShadow(shadowMap, startTexel + vec2(0.0, texelSize.y), compare);
    float trTexel = SampleShadow(shadowMap, startTexel + texelSize, compare);

    float mixA = mix(blTexel, tlTexel, fractPart.y);
    float mixB = mix(brTexel, trTexel, fractPart.y);

    return mix(mixA, mixB, fractPart.x);
}
)", 1);
    }
    if (ShaderHelper::lacksDefinition(code, "SampleShadow(", "float SampleShadow(")) {
        ShaderHelper::insertStringAtLine(code, R"(
float SampleShadow(sampler2D shadowMap, vec2 uvs, float compare){
    return step(compare, texture2D(shadowMap, uvs).r);
}
)", 1);
    }
}