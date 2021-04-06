
#include <serenity/resources/shader/Shader.h>
#include <serenity/resources/shader/ShaderHelper.h>
#include <serenity/renderer/Renderer.h>

#include <serenity/renderer/opengl/glsl/Common.h>
#include <serenity/renderer/opengl/glsl/Compression.h>
#include <serenity/renderer/opengl/glsl/VersionConversion.h>
#include <serenity/renderer/opengl/glsl/Materials.h>
#include <serenity/renderer/opengl/glsl/Lighting.h>
#include <serenity/renderer/opengl/glsl/SSAOCode.h>
#include <serenity/renderer/opengl/glsl/DepthOfFieldCode.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>
#include <regex>

using namespace Engine;

using boost_stream_mapped_file = boost::iostreams::stream<boost::iostreams::mapped_file_source>;

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
Shader::Shader(std::string_view filenameOrCode, ShaderType shaderType, bool fromFile)
    : Resource    { ResourceType::Shader }
    , m_FileName  { filenameOrCode }
    , m_ShaderType{ shaderType }
    , m_FromFile  { fromFile }
{
    if (fromFile) {
        setName(filenameOrCode);
        m_FileName = filenameOrCode;
    }else{
        setName("NULL");
        m_Code     = filenameOrCode;
    }
    priv::PublicShader::ConvertCode(*this);
}
Shader::Shader(Shader&& other) noexcept 
    : Resource{ std::move(other) }
    , m_ShaderType{ std::move(other.m_ShaderType) }
    , m_FromFile  { std::move(other.m_FromFile) }
    , m_FileName  { std::move(other.m_FileName) }
    , m_Code      { std::move(other.m_Code) }
{}
Shader& Shader::operator=(Shader&& other) noexcept {
    Resource::operator=(std::move(other));
    m_ShaderType = std::move(other.m_ShaderType);
    m_FromFile   = std::move(other.m_FromFile);
    m_FileName   = std::move(other.m_FileName);
    m_Code       = std::move(other.m_Code);
    return *this;
}

void priv::PublicShader::ConvertCode(Shader& shader) {
    //load initial code
    if (shader.m_FromFile) {
        std::string code;
        boost_stream_mapped_file str(shader.m_FileName);
        for (std::string line; std::getline(str, line, '\n');) {
            code += "\n" + line;
        }
        shader.m_Code = code;
    }
    //see if we actually have a version line
    std::string versionLine;
    std::istringstream strstream{ shader.m_Code };
    if (ShaderHelper::sfind(shader.m_Code, "#version ")) {
        //use the found one
        while (std::getline(strstream, versionLine)) {
            if (ShaderHelper::sfind(versionLine, "#version ")) {
                break;
            }
        }
    }else{
        //generate one
        std::string core;
        if (Engine::priv::OpenGLState::constants.GLSL_VERSION >= 330 && Engine::priv::OpenGLState::constants.GLSL_VERSION < 440) {
            core = " core";
        }
        versionLine = "#version " + std::to_string(Engine::priv::OpenGLState::constants.GLSL_VERSION) + core + "\n";
    }
    const uint32_t versionNumber = boost::lexical_cast<uint32_t>(std::regex_replace(versionLine, std::regex("([^0-9])"), ""));

    //common code
    opengl::glsl::Materials::convert(shader.m_Code, versionNumber, shader.m_ShaderType);
    opengl::glsl::Lighting::convert(shader.m_Code, versionNumber, shader.m_ShaderType);
    opengl::glsl::SSAOCode::convert(shader.m_Code, versionNumber, shader.m_ShaderType);
    opengl::glsl::Compression::convert(shader.m_Code, versionNumber);
    opengl::glsl::DepthOfFieldCode::convert(shader.m_Code, versionNumber, shader.m_ShaderType);
    

    //check for log depth - vertex
    if (ShaderHelper::sfind(shader.m_Code, "USE_LOG_DEPTH_VERTEX") && !ShaderHelper::sfind(shader.m_Code, "//USE_LOG_DEPTH_VERTEX") && shader.m_ShaderType == ShaderType::Vertex) {
        boost::replace_all(shader.m_Code, "USE_LOG_DEPTH_VERTEX", "");
            #ifndef ENGINE_FORCE_NO_LOG_DEPTH
                ShaderHelper::insertStringAtLine(shader.m_Code, R"(
flat varying float FC;
varying float logz_f;
)", 1);
                ShaderHelper::insertStringAtEndOfMainFunc(shader.m_Code, R"(
logz_f = 1.0 + gl_Position.w;
gl_Position.z = (log2(max(0.000001, logz_f)) * LogFCoefficient - 1.0) * gl_Position.w;
FC = LogFCoefficient;
)");
            #endif
    }

    //check for view space normals from world (not sure if this is proper)
    if (ShaderHelper::sfind(shader.m_Code, "GetViewNormalsFromWorld(")) {
        if (!ShaderHelper::sfind(shader.m_Code, "vec4 GetViewNormalsFromWorld(")) {
            ShaderHelper::insertStringAtLine(shader.m_Code, R"(
vec3 GetViewNormalsFromWorld(vec3 worldNormals, mat4 camView){
    return (camView * vec4(worldNormals, 0.0)).xyz;
}
)", 1);
        }
    }
    //check for world space normals from view (this works perfectly)
    if (ShaderHelper::sfind(shader.m_Code, "GetWorldNormalsFromView(")) {
        if (!ShaderHelper::sfind(shader.m_Code, "vec4 GetWorldNormalsFromView(")) {
            ShaderHelper::insertStringAtLine(shader.m_Code, R"(
vec3 GetWorldNormalsFromView(vec3 viewNormals, mat4 camView){ //generated
    return (transpose(camView) * vec4(viewNormals, 0.0)).xyz;
}
)", 1);
        }
    }

    //check for log depth - fragment
    if (ShaderHelper::sfind(shader.m_Code, "USE_LOG_DEPTH_FRAGMENT") && !ShaderHelper::sfind(shader.m_Code, "//USE_LOG_DEPTH_FRAGMENT") && shader.m_ShaderType == ShaderType::Fragment) {
        boost::replace_all(shader.m_Code, "USE_LOG_DEPTH_FRAGMENT", "");
        #ifndef ENGINE_FORCE_NO_LOG_DEPTH
            ShaderHelper::insertStringRightBeforeMainFunc(shader.m_Code, R"(
flat varying float FC;
varying float logz_f;
)");
            ShaderHelper::insertStringAtEndOfMainFunc(shader.m_Code, R"(
gl_FragDepth = log2(logz_f) * FC;
)");
        #endif
    }
    if (ShaderHelper::sfind(shader.m_Code, "GetWorldPosition(") || ShaderHelper::sfind(shader.m_Code, "GetViewPosition(")) {
        if (!ShaderHelper::sfind(shader.m_Code, "vec3 GetWorldPosition(")) {
            #ifndef ENGINE_FORCE_NO_LOG_DEPTH
                ShaderHelper::insertStringRightAfterLineContent(shader.m_Code, LogDepthFunctions, "uniform SAMPLER_TYPE_2D gDepthMap;");
            #else
                ShaderHelper::insertStringRightAfterLineContent(shader.m_Code, NormalDepthFunctions, "uniform SAMPLER_TYPE_2D gDepthMap;");
            #endif
}
        }
    opengl::glsl::Common::convert(shader.m_Code, versionNumber);
    opengl::glsl::VersionConversion::convert(shader.m_Code, versionNumber, shader.m_ShaderType);

    //put the version on top as the last step :)
    shader.m_Code = versionLine + shader.m_Code;
}
