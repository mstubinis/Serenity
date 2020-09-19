#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/shaders/Shader.h>
#include <core/engine/shaders/ShaderHelper.h>
#include <core/engine/renderer/Renderer.h>

#include <core/engine/renderer/opengl/glsl/Common.h>
#include <core/engine/renderer/opengl/glsl/Compression.h>
#include <core/engine/renderer/opengl/glsl/VersionConversion.h>
#include <core/engine/renderer/opengl/glsl/Materials.h>
#include <core/engine/renderer/opengl/glsl/Lighting.h>
#include <core/engine/renderer/opengl/glsl/SSAOCode.h>
#include <core/engine/renderer/opengl/glsl/DepthOfFieldCode.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>

using namespace Engine;

using boost_stream_mapped_file = boost::iostreams::stream<boost::iostreams::mapped_file_source>;

//seems to be ok for now
constexpr const char* LogDepthFunctions =
    "vec3 GetWorldPosition(sampler2D inTexture, vec2 inUV, float inNear, float inFar){//generated\n"
    "    float depth = texture2D(inTexture, inUV).r;\n"
    "    float position_w = pow(2.0, depth * log2(inFar + 1.0)) - 1.0;\n"
    "    float a = inFar / (inFar - inNear);\n"
    "    float b = inFar * inNear / (inNear - inFar);\n"
    "    float linear = a + b / position_w;\n"
    "    vec4 wpos = CameraInvViewProj * (vec4(inUV, linear, 1.0) * 2.0 - 1.0);\n"
    "    return wpos.xyz / wpos.w;\n"
    "}\n"
    "vec3 GetViewPosition(sampler2D inTexture, vec2 inUV, float inNear, float inFar){//generated\n"
    "    float depth = texture2D(inTexture, inUV).r;\n"
    "    float position_w = pow(2.0, depth * log2(inFar + 1.0)) - 1.0;\n"
    "    float a = inFar / (inFar - inNear);\n"
    "    float b = inFar * inNear / (inNear - inFar);\n"
    "    float linear = a + b / position_w;\n"
    "    vec4 wpos = CameraInvProj * (vec4(inUV, linear, 1.0) * 2.0 - 1.0);\n"
    "    return wpos.xyz / wpos.w;\n"
    "}\n";

//this is working great right now, do not modify
constexpr const char* NormalDepthFunctions =
    "vec3 GetWorldPosition(sampler2D inTexture, vec2 inUV, float inNear, float inFar){//generated\n"
    "    float depth = texture2D(inTexture, inUV).r * 2.0 - 1.0;\n"
    "	 vec4 space = vec4(inUV * 2.0 - 1.0, depth, 1.0);\n"
    "	 space = CameraInvViewProj * space;\n"
    "	 return space.xyz / space.w;\n"
    "}\n"
    "vec3 GetViewPosition(sampler2D inTexture, vec2 inUV, float inNear, float inFar){//generated\n"
    "    float depth = texture2D(inTexture, inUV).x;\n"
    "    vec4 space = CameraInvProj * (vec4(inUV, depth, 1.0) * 2.0 - 1.0);\n"
    "    return space.xyz / space.w;\n"
    "}\n";

Shader::Shader(const std::string& filenameOrCode, ShaderType shaderType, bool fromFile) 
    : Resource{ ResourceType::Shader }
    , m_FileName{ filenameOrCode }
    , m_Type{ shaderType }
    , m_FromFile{ fromFile }
{
    if (fromFile) {
        setName(filenameOrCode);
        m_FileName = filenameOrCode;
        m_Code     = "";
    }else{
        setName("NULL");
        m_FileName = "";
        m_Code     = filenameOrCode;
    }
    priv::InternalShaderPublicInterface::ConvertCode(*this);
}
Shader::~Shader() {
}



void priv::InternalShaderPublicInterface::ConvertCode(Shader& shader) {
    //load initial code
    if (shader.m_FromFile) {
        std::string code = "";
        boost_stream_mapped_file str(shader.m_FileName);
        for (std::string line; std::getline(str, line, '\n');) {
            code += "\n" + line;
        }
        shader.m_Code = code;
    }
    //see if we actually have a version line
    std::string versionLine;
    std::istringstream str(shader.m_Code);
    if (ShaderHelper::sfind(shader.m_Code, "#version ")) {
        //use the found one
        while (std::getline(str, versionLine)) {
            if (ShaderHelper::sfind(versionLine, "#version ")) {
                break;
            }
        }
    }else{
        //generate one
        std::string core = "";
        if (Engine::priv::Renderer::GLSL_VERSION >= 330 && Engine::priv::Renderer::GLSL_VERSION < 440) {
            core = " core";
        }
        versionLine = "#version " + std::to_string(Engine::priv::Renderer::GLSL_VERSION) + core + "\n";
    }
    const unsigned int versionNumber = boost::lexical_cast<unsigned int>(std::regex_replace(versionLine, std::regex("([^0-9])"), ""));

    //common code
    opengl::glsl::Materials::convert(shader.m_Code, versionNumber, shader.m_Type);
    opengl::glsl::Lighting::convert(shader.m_Code, versionNumber, shader.m_Type);
    opengl::glsl::SSAOCode::convert(shader.m_Code, versionNumber, shader.m_Type);
    opengl::glsl::DepthOfFieldCode::convert(shader.m_Code, versionNumber, shader.m_Type);
    opengl::glsl::Compression::convert(shader.m_Code, versionNumber);
    

    //check for log depth - vertex
    if (ShaderHelper::sfind(shader.m_Code, "USE_LOG_DEPTH_VERTEX") && !ShaderHelper::sfind(shader.m_Code, "//USE_LOG_DEPTH_VERTEX") && shader.m_Type == ShaderType::Vertex) {
        boost::replace_all(shader.m_Code, "USE_LOG_DEPTH_VERTEX", "");
            #ifndef ENGINE_FORCE_NO_LOG_DEPTH
                ShaderHelper::insertStringAtLine(shader.m_Code, 
                    "\n"
                    "uniform float fcoeff;\n"
                    "flat varying float FC;\n"
                    "varying float logz_f;\n"
                    "\n"
                , 1);
                ShaderHelper::insertStringAtEndOfMainFunc(shader.m_Code, 
                    "\n"
                    "logz_f = 1.0 + gl_Position.w;\n"
                    "gl_Position.z = (log2(max(0.000001, logz_f)) * fcoeff - 1.0) * gl_Position.w;\n" //this line is optional i think... since gl_FragDepth may be written manually
                    "FC = fcoeff;\n"
                    "\n"
                );
            #endif
    }

    //check for view space normals from world (not sure if this is proper)
    if (ShaderHelper::sfind(shader.m_Code, "GetViewNormalsFromWorld(")) {
        if (!ShaderHelper::sfind(shader.m_Code, "vec4 GetViewNormalsFromWorld(")) {
            ShaderHelper::insertStringAtLine(shader.m_Code, 
                "\n"
                "vec3 GetViewNormalsFromWorld(vec3 worldNormals,mat4 camView){//generated\n"
                "    return (camView * vec4(worldNormals,0.0)).xyz;\n"
                "}\n"
            , 1);
        }
    }
    //check for world space normals from view (this works perfectly)
    if (ShaderHelper::sfind(shader.m_Code, "GetWorldNormalsFromView(")) {
        if (!ShaderHelper::sfind(shader.m_Code, "vec4 GetWorldNormalsFromView(")) {
            ShaderHelper::insertStringAtLine(shader.m_Code, 
                "\n"
                "vec3 GetWorldNormalsFromView(vec3 viewNormals,mat4 camView){//generated\n"
                "    return (transpose(camView) * vec4(viewNormals,0.0)).xyz;\n"
                "}\n"
            , 1);
        }
    }

    //check for log depth - fragment
    if (ShaderHelper::sfind(shader.m_Code, "USE_LOG_DEPTH_FRAGMENT") && !ShaderHelper::sfind(shader.m_Code, "//USE_LOG_DEPTH_FRAGMENT") && shader.m_Type == ShaderType::Fragment) {
        boost::replace_all(shader.m_Code, "USE_LOG_DEPTH_FRAGMENT", "");
            #ifndef ENGINE_FORCE_NO_LOG_DEPTH
                ShaderHelper::insertStringRightBeforeMainFunc(shader.m_Code, 
                    "\n"
                    "flat varying float FC;\n"
                    "varying float logz_f;\n"
                );
                ShaderHelper::insertStringAtEndOfMainFunc(shader.m_Code, 
                    "gl_FragDepth = log2(logz_f) * FC;\n"
                );
            #endif
        if (ShaderHelper::sfind(shader.m_Code, "GetWorldPosition(") || ShaderHelper::sfind(shader.m_Code, "GetViewPosition(")) {
            if (!ShaderHelper::sfind(shader.m_Code, "vec3 GetWorldPosition(")) {
                #ifndef ENGINE_FORCE_NO_LOG_DEPTH
                    ShaderHelper::insertStringRightAfterLineContent(shader.m_Code, LogDepthFunctions, "uniform SAMPLER_TYPE_2D gDepthMap;");
                #else
                    ShaderHelper::insertStringRightAfterLineContent(shader.m_Code, NormalDepthFunctions, "uniform SAMPLER_TYPE_2D gDepthMap;");
                #endif
            }
        }
    }else {
        if (ShaderHelper::sfind(shader.m_Code, "GetWorldPosition(") || ShaderHelper::sfind(shader.m_Code, "GetViewPosition(")) {
            if (!ShaderHelper::sfind(shader.m_Code, "vec3 GetWorldPosition(")) {
                if (ShaderHelper::sfind(shader.m_Code, "USE_LOG_DEPTH_FRAG_WORLD_POSITION") && !ShaderHelper::sfind(shader.m_Code, "//USE_LOG_DEPTH_FRAG_WORLD_POSITION") && shader.m_Type == ShaderType::Fragment) {
                    //log
                    boost::replace_all(shader.m_Code, "USE_LOG_DEPTH_FRAG_WORLD_POSITION", "");
                        #ifndef ENGINE_FORCE_NO_LOG_DEPTH
                            ShaderHelper::insertStringRightAfterLineContent(shader.m_Code, LogDepthFunctions, "uniform SAMPLER_TYPE_2D gDepthMap;");
                        #else
                            ShaderHelper::insertStringRightAfterLineContent(shader.m_Code, NormalDepthFunctions, "uniform SAMPLER_TYPE_2D gDepthMap;");
                        #endif
                }else{
                    #ifndef ENGINE_FORCE_NO_LOG_DEPTH
                        ShaderHelper::insertStringRightAfterLineContent(shader.m_Code, LogDepthFunctions, "uniform SAMPLER_TYPE_2D gDepthMap;");
                    #else
                        ShaderHelper::insertStringRightAfterLineContent(shader.m_Code, NormalDepthFunctions, "uniform SAMPLER_TYPE_2D gDepthMap;");
                    #endif
                }
            }
        }
    }
    opengl::glsl::Common::convert(shader.m_Code, versionNumber);
    opengl::glsl::VersionConversion::convert(shader.m_Code, versionNumber, shader.m_Type);

    //put the version on top as the last step :)
    shader.m_Code = versionLine + shader.m_Code;
}
