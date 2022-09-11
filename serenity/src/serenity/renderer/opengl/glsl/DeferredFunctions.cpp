
#include <serenity/renderer/opengl/glsl/DeferredFunctions.h>
#include <serenity/resources/shader/ShaderHelper.h>
#include <serenity/resources/material/MaterialEnums.h>

#include <boost/algorithm/string/replace.hpp>

// https://gamedev.stackexchange.com/questions/164494/world-position-reconstruction-from-depth-fails-when-viewport-size-does-not-match
// https://stackoverflow.com/questions/11277501/how-to-recover-view-space-position-given-view-space-depth-value-and-ndc-xy
namespace {

    constexpr auto CALC_CLIPSPACE_XY = R"(
vec2 CalculateClipSpaceXY(vec2 inUV){
    vec2 res = inUV;
    vec2 factor = ScreenInfo.zw / ScreenInfo.xy;
    vec2 A = vec2(1.0) - factor;
    res -= A * 0.5;
    res *= 1.0 / factor;
    return res;
}
)";

    constexpr auto LINEARIZE_LOG_DEPTH = R"(
float LinearizeLogDepth(float inDepth, float inNear, float inFar) {
    float position_w = pow(2.0, inDepth * log2(inFar + 1.0)) - 1.0;
    float a = inFar / (inFar - inNear);
    float b = inFar * inNear / (inNear - inFar);
    float linear = a + b / position_w;
    return linear;
}
)";
    constexpr auto LINEARIZE_LOG_DEPTH_NO_OP = R"(
float LinearizeLogDepth(float inDepth, float inNear, float inFar) {
    return inDepth;
}
)";


    constexpr auto GET_WORLD_AND_VIEW_POSITIONS = R"(
vec3 GetWorldPosition(sampler2D inTexture, vec2 inUV, float inNear, float inFar) {
    float depth = LinearizeLogDepth(texture2D(inTexture, inUV).r, inNear, inFar);
    vec4 wpos = CameraInvViewProj * vec4(CalculateClipSpaceXY(inUV) * 2.0 - vec2(1.0), depth * 2.0 - 1.0, 1.0);
    return wpos.xyz / wpos.w;
}
vec3 GetViewPosition(sampler2D inTexture, vec2 inUV, float inNear, float inFar) {
    float depth = LinearizeLogDepth(texture2D(inTexture, inUV).r, inNear, inFar);
    vec4 vpos = CameraInvProj * vec4(CalculateClipSpaceXY(inUV) * 2.0 - vec2(1.0), depth * 2.0 - 1.0, 1.0);
    return vpos.xyz / vpos.w;
};
)";
}




void Engine::priv::opengl::glsl::DeferredFunctions::convert(std::string& code, uint32_t versionNumber, ShaderType shaderType) {

    //check for view space normals from world (not sure if this is proper)
    if (ShaderHelper::sfind(code, "GetViewNormalsFromWorld(") && !ShaderHelper::sfind(code, "vec4 GetViewNormalsFromWorld(")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec3 GetViewNormalsFromWorld(vec3 worldNormals, mat4 camView) {
    return (camView * vec4(worldNormals, 0.0)).xyz;
}
)", 1);
    }
    //check for world space normals from view (this works perfectly)
    if (ShaderHelper::sfind(code, "GetWorldNormalsFromView(") && !ShaderHelper::sfind(code, "vec4 GetWorldNormalsFromView(")) {
        ShaderHelper::insertStringAtLine(code, R"(
vec3 GetWorldNormalsFromView(vec3 viewNormals, mat4 camView) {
    return (transpose(camView) * vec4(viewNormals, 0.0)).xyz;
}
)", 1);
    }


    //check for log depth - vertex
    if (ShaderHelper::sfind(code, "USE_LOG_DEPTH_VERTEX") && shaderType == ShaderType::Vertex) {
        boost::replace_all(code, "USE_LOG_DEPTH_VERTEX", "");
#if !defined(ENGINE_FORCE_NO_LOG_DEPTH)
        ShaderHelper::insertStringAtLine(code, R"(varying float VARYING_LOG_Z_F;)", 1);
        ShaderHelper::insertStringAtEndOfMainFunc(code, R"(
    VARYING_LOG_Z_F = 1.0 + gl_Position.w;
    gl_Position.z = (log2(max(0.000001, VARYING_LOG_Z_F)) * LogFCoefficient - 1.0) * gl_Position.w;
)");
#endif
    }

    //check for log depth - fragment
    if (ShaderHelper::sfind(code, "USE_LOG_DEPTH_FRAGMENT") && shaderType == ShaderType::Fragment) {
        boost::replace_all(code, "USE_LOG_DEPTH_FRAGMENT", "");
#if !defined(ENGINE_FORCE_NO_LOG_DEPTH)
        ShaderHelper::insertStringRightBeforeMainFunc(code, R"(varying float VARYING_LOG_Z_F;)");
        ShaderHelper::insertStringAtEndOfMainFunc(code, R"(    gl_FragDepth = log2(VARYING_LOG_Z_F) * LogFCoefficient;)");
#endif
    }
    if (ShaderHelper::sfind(code, "GetWorldPosition(") || ShaderHelper::sfind(code, "GetViewPosition(") && !ShaderHelper::sfind(code, "vec3 GetWorldPosition(")) {
        ShaderHelper::insertStringRightAfterLineContent(code, GET_WORLD_AND_VIEW_POSITIONS, "uniform SAMPLER_TYPE_2D gDepthMap;");

#if !defined(ENGINE_FORCE_NO_LOG_DEPTH)
        ShaderHelper::insertStringRightAfterLineContent(code, LINEARIZE_LOG_DEPTH, "uniform SAMPLER_TYPE_2D gDepthMap;");
#else
        ShaderHelper::insertStringRightAfterLineContent(code, LINEARIZE_LOG_DEPTH_NO_OP, "uniform SAMPLER_TYPE_2D gDepthMap;");
#endif

        ShaderHelper::insertStringRightAfterLineContent(code, CALC_CLIPSPACE_XY, "uniform SAMPLER_TYPE_2D gDepthMap;");
    }
}