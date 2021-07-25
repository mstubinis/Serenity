
#include <serenity/renderer/opengl/glsl/DeferredFunctions.h>
#include <serenity/resources/shader/ShaderHelper.h>
#include <serenity/resources/material/MaterialEnums.h>

#include <boost/algorithm/string/replace.hpp>


//seems to be ok for now
constexpr auto LogDepthFunctions = R"(
vec3 GetWorldPosition(sampler2D inTexture, vec2 inUV, float inNear, float inFar){ //generated
    float depth = texture2D(inTexture, inUV).r;
    float position_w = pow(2.0, depth * log2(inFar + 1.0)) - 1.0;
    float a = inFar / (inFar - inNear);
    float b = inFar * inNear / (inNear - inFar);
    float linear = a + b / position_w;
    vec4 wpos = CameraInvViewProj * (vec4(inUV, linear, 1.0) * 2.0 - 1.0);
    return wpos.xyz / wpos.w;
}
vec3 GetViewPosition(sampler2D inTexture, vec2 inUV, float inNear, float inFar){ //generated
    float depth = texture2D(inTexture, inUV).r;
    float position_w = pow(2.0, depth * log2(inFar + 1.0)) - 1.0;
    float a = inFar / (inFar - inNear);
    float b = inFar * inNear / (inNear - inFar);
    float linear = a + b / position_w;
    vec4 vpos = CameraInvProj * (vec4(inUV, linear, 1.0) * 2.0 - 1.0);
    return vpos.xyz / vpos.w;
};
)";
//this is working great right now, do not modify
constexpr auto NormalDepthFunctions = R"(
vec3 GetWorldPosition(sampler2D inTexture, vec2 inUV, float inNear, float inFar){ //generated
    float depth = texture2D(inTexture, inUV).r;
    vec4 wpos = CameraInvViewProj * (vec4(inUV, depth, 1.0) * 2.0 - 1.0);
    return wpos.xyz / wpos.w;
}
vec3 GetViewPosition(sampler2D inTexture, vec2 inUV, float inNear, float inFar){ //generated
    float depth = texture2D(inTexture, inUV).r;
    vec4 vpos = CameraInvProj * (vec4(inUV, depth, 1.0) * 2.0 - 1.0);
    return vpos.xyz / vpos.w;
};
)";

void Engine::priv::opengl::glsl::DeferredFunctions::convert(std::string& code, uint32_t versionNumber, ShaderType shaderType) {

    //check for log depth - vertex
    if (ShaderHelper::sfind(code, "USE_LOG_DEPTH_VERTEX") && !ShaderHelper::sfind(code, "//USE_LOG_DEPTH_VERTEX") && shaderType == ShaderType::Vertex) {
        boost::replace_all(code, "USE_LOG_DEPTH_VERTEX", "");
#ifndef ENGINE_FORCE_NO_LOG_DEPTH
        ShaderHelper::insertStringAtLine(code, R"(
flat varying float FC;
varying float logz_f;
)", 1);
        ShaderHelper::insertStringAtEndOfMainFunc(code, R"(
logz_f = 1.0 + gl_Position.w;
gl_Position.z = (log2(max(0.000001, logz_f)) * LogFCoefficient - 1.0) * gl_Position.w;
FC = LogFCoefficient;
)");
#endif
    }

    //check for view space normals from world (not sure if this is proper)
    if (ShaderHelper::sfind(code, "GetViewNormalsFromWorld(")) {
        if (!ShaderHelper::sfind(code, "vec4 GetViewNormalsFromWorld(")) {
            ShaderHelper::insertStringAtLine(code, R"(
vec3 GetViewNormalsFromWorld(vec3 worldNormals, mat4 camView){
    return (camView * vec4(worldNormals, 0.0)).xyz;
}
)", 1);
        }
    }
    //check for world space normals from view (this works perfectly)
    if (ShaderHelper::sfind(code, "GetWorldNormalsFromView(")) {
        if (!ShaderHelper::sfind(code, "vec4 GetWorldNormalsFromView(")) {
            ShaderHelper::insertStringAtLine(code, R"(
vec3 GetWorldNormalsFromView(vec3 viewNormals, mat4 camView){ //generated
    return (transpose(camView) * vec4(viewNormals, 0.0)).xyz;
}
)", 1);
        }
    }

    //check for log depth - fragment
    if (ShaderHelper::sfind(code, "USE_LOG_DEPTH_FRAGMENT") && !ShaderHelper::sfind(code, "//USE_LOG_DEPTH_FRAGMENT") && shaderType == ShaderType::Fragment) {
        boost::replace_all(code, "USE_LOG_DEPTH_FRAGMENT", "");
#ifndef ENGINE_FORCE_NO_LOG_DEPTH
        ShaderHelper::insertStringRightBeforeMainFunc(code, R"(
flat varying float FC;
varying float logz_f;
)");
        ShaderHelper::insertStringAtEndOfMainFunc(code, R"(
gl_FragDepth = log2(logz_f) * FC;
)");
#endif
    }
    if (ShaderHelper::sfind(code, "GetWorldPosition(") || ShaderHelper::sfind(code, "GetViewPosition(")) {
        if (!ShaderHelper::sfind(code, "vec3 GetWorldPosition(")) {
#ifndef ENGINE_FORCE_NO_LOG_DEPTH
            ShaderHelper::insertStringRightAfterLineContent(code, LogDepthFunctions, "uniform SAMPLER_TYPE_2D gDepthMap;");
#else
            ShaderHelper::insertStringRightAfterLineContent(code, NormalDepthFunctions, "uniform SAMPLER_TYPE_2D gDepthMap;");
#endif
        }
    }
}