#include <core/engine/shaders/Shader.h>
#include <core/engine/shaders/ShaderHelper.h>
#include <core/engine/renderer/Engine_Renderer.h>

#include <core/engine/renderer/opengl/glsl/Common.h>
#include <core/engine/renderer/opengl/glsl/Compression.h>
#include <core/engine/renderer/opengl/glsl/VersionConversion.h>
#include <core/engine/renderer/opengl/glsl/Materials.h>
#include <core/engine/renderer/opengl/glsl/Lighting.h>
#include <core/engine/renderer/opengl/glsl/SSAOCode.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>

#include <regex>
#include <iostream>

using namespace std;
using namespace Engine;
using namespace Engine::epriv;

typedef boost::iostreams::stream<boost::iostreams::mapped_file_source> boost_stream_mapped_file;

//seems to be ok for now
string getLogDepthFunctions() {
    string res = "\n"
        "vec3 GetWorldPosition(vec2 _uv,float _near, float _far){//generated\n"
        "    float depth = texture2D(gDepthMap, _uv).r;\n"
        "    float position_w = pow(2.0, depth * log2(_far + 1.0)) - 1.0;\n"
        "    float a = _far / (_far - _near);\n"
        "    float b = _far * _near / (_near - _far);\n"
        "    float linear = a + b / position_w;\n"
        "    vec4 wpos = CameraInvViewProj * (vec4(_uv, linear, 1.0) * 2.0 - 1.0);\n"
        "    return wpos.xyz / wpos.w;\n"
        "}\n"
        "vec3 GetViewPosition(vec2 _uv,float _near, float _far){//generated\n"
        "    float depth = texture2D(gDepthMap, _uv).r;\n"
        "    float position_w = pow(2.0, depth * log2(_far + 1.0)) - 1.0;\n"
        "    float a = _far / (_far - _near);\n"
        "    float b = _far * _near / (_near - _far);\n"
        "    float linear = a + b / position_w;\n"
        "    vec4 wpos = CameraInvProj * (vec4(_uv, linear, 1.0) * 2.0 - 1.0);\n"
        "    return wpos.xyz / wpos.w;\n"
        "}\n";
    return res;
}
//this is working great right now, do not modify
string getNormalDepthFunctions() {
    string res = "\n"
        "vec3 GetWorldPosition(vec2 _uv,float _near, float _far){//generated\n"
        "    float depth = texture2D(gDepthMap, _uv).r * 2.0 - 1.0;\n"
        "	 vec4 space = vec4(_uv * 2.0 - 1.0, depth, 1.0);\n"
        "	 space = CameraInvViewProj * space;\n"
        "	 return space.xyz / space.w;\n"
        "}\n"
        "vec3 GetViewPosition(vec2 _uv,float _near, float _far){//generated\n"
        "    float depth = texture2D(gDepthMap, _uv).x;\n"
        "    vec4 space = CameraInvProj * (vec4(_uv, depth, 1.0) * 2.0 - 1.0);\n"
        "    return space.xyz / space.w;\n"
        "}\n";
    return res;
}


Shader::Shader(const string& filenameOrCode, const ShaderType::Type& shaderType, const bool& fromFile) {
    m_FileName = filenameOrCode;
    m_Type = shaderType;
    m_FromFile = fromFile;
    if (fromFile) {
        setName(filenameOrCode);
        m_FileName = filenameOrCode;
        m_Code = "";
    }else{
        setName("NULL");
        m_FileName = "";
        m_Code = filenameOrCode;
    }
    convertCode();
}
Shader::~Shader() {
}
const ShaderType::Type& Shader::type() const {
    return m_Type;
}
const string& Shader::data() const {
    return m_Code;
}
const bool& Shader::fromFile() const {
    return m_FromFile;
}
void Shader::convertCode() {
    //load initial code
    string code = "";
    if (m_FromFile) {
        boost_stream_mapped_file str(m_FileName);
        for (string line; getline(str, line, '\n');) {
            code += "\n" + line;
        }
        m_Code = code;
    }

    //see if we actually have a version line
    string versionLine;
    istringstream str(m_Code);
    if (ShaderHelper::sfind(m_Code, "#version ")) {
        //use the found one
        while (getline(str, versionLine)) {
            if (ShaderHelper::sfind(versionLine, "#version ")) {
                break;
            }
        }
    }else{
        //generate one
        string core = "";
        if (RenderManager::GLSL_VERSION >= 330)
            core = " core";
        versionLine = "#version " + to_string(RenderManager::GLSL_VERSION) + core + "\n";
        ShaderHelper::insertStringAtLine(m_Code, versionLine, 0);
    }
    const uint versionNumber = boost::lexical_cast<uint>(regex_replace(versionLine, regex("([^0-9])"), ""));

    //common code
    opengl::glsl::Materials::convert(m_Code, versionNumber, m_Type);
    opengl::glsl::Lighting::convert(m_Code, versionNumber, m_Type);
    opengl::glsl::SSAOCode::convert(m_Code, versionNumber, m_Type);
    opengl::glsl::Compression::convert(m_Code, versionNumber);
    opengl::glsl::Common::convert(m_Code, versionNumber);


    //check for log depth - vertex
    if (ShaderHelper::sfind(m_Code, "USE_LOG_DEPTH_VERTEX") && !ShaderHelper::sfind(m_Code, "//USE_LOG_DEPTH_VERTEX") && m_Type == ShaderType::Vertex) {
        boost::replace_all(m_Code, "USE_LOG_DEPTH_VERTEX", "");
#ifndef ENGINE_FORCE_NO_LOG_DEPTH
        string log_vertex_code = "\n"
            "uniform float fcoeff;\n"
            "flat varying float FC;\n"
            "varying float logz_f;\n"
            "\n";
        ShaderHelper::insertStringAtLine(m_Code, log_vertex_code, 1);
        log_vertex_code = "\n"
            "logz_f = 1.0 + gl_Position.w;\n"
            "gl_Position.z = (log2(max(0.000001, logz_f)) * fcoeff - 1.0) * gl_Position.w;\n" //this line is optional i think... since gl_FragDepth may be written manually
            "FC = fcoeff;\n"
            "\n";
        ShaderHelper::insertStringAtEndOfMainFunc(m_Code, log_vertex_code);
#endif
    }

    //check for view space normals from world (not sure if this is proper)
    if (ShaderHelper::sfind(m_Code, "GetViewNormalsFromWorld(")) {
        if (!ShaderHelper::sfind(m_Code, "vec4 GetViewNormalsFromWorld(")) {
            string viewNormals = "\n"
                "vec3 GetViewNormalsFromWorld(vec3 worldNormals,mat4 camView){//generated\n"
                "    return (camView * vec4(worldNormals,0.0)).xyz;\n"
                "}\n";
            ShaderHelper::insertStringAtLine(m_Code, viewNormals, 1);
        }
    }
    //check for world space normals from view (this works perfectly)
    if (ShaderHelper::sfind(m_Code, "GetWorldNormalsFromView(")) {
        if (!ShaderHelper::sfind(m_Code, "vec4 GetWorldNormalsFromView(")) {
            string viewNormals = "\n"
                "vec3 GetWorldNormalsFromView(vec3 viewNormals,mat4 camView){//generated\n"
                "    return (transpose(camView) * vec4(viewNormals,0.0)).xyz;\n"
                "}\n";
            ShaderHelper::insertStringAtLine(m_Code, viewNormals, 1);
        }
    }

    //check for log depth - fragment
    if (ShaderHelper::sfind(m_Code, "USE_LOG_DEPTH_FRAGMENT") && !ShaderHelper::sfind(m_Code, "//USE_LOG_DEPTH_FRAGMENT") && m_Type == ShaderType::Fragment) {
        boost::replace_all(m_Code, "USE_LOG_DEPTH_FRAGMENT", "");
#ifndef ENGINE_FORCE_NO_LOG_DEPTH
        string log_frag_code = "\n"
            "flat varying float FC;\n"
            "varying float logz_f;\n"
            "\n";
        ShaderHelper::insertStringAtLine(m_Code, log_frag_code, 1);
        log_frag_code = "\n"
            "gl_FragDepth = log2(logz_f) * FC;\n"
            "\n";
        ShaderHelper::insertStringAtEndOfMainFunc(m_Code, log_frag_code);
#endif
        if (ShaderHelper::sfind(m_Code, "GetWorldPosition(") || ShaderHelper::sfind(m_Code, "GetViewPosition(")) {
            if (!ShaderHelper::sfind(m_Code, "vec3 GetWorldPosition(")) {
#ifndef ENGINE_FORCE_NO_LOG_DEPTH
                ShaderHelper::insertStringRightAfterLineContent(m_Code, getLogDepthFunctions(), "uniform sampler2D gDepthMap;");
#else
                ShaderHelper::insertStringRightAfterLineContent(m_Code, getNormalDepthFunctions(), "uniform sampler2D gDepthMap;");
#endif
            }
        }
    }else{
        if (ShaderHelper::sfind(m_Code, "GetWorldPosition(") || ShaderHelper::sfind(m_Code, "GetViewPosition(")) {
            if (!ShaderHelper::sfind(m_Code, "vec3 GetWorldPosition(")) {
                if (ShaderHelper::sfind(m_Code, "USE_LOG_DEPTH_FRAG_WORLD_POSITION") && !ShaderHelper::sfind(m_Code, "//USE_LOG_DEPTH_FRAG_WORLD_POSITION") && m_Type == ShaderType::Fragment) {
                    //log
                    boost::replace_all(m_Code, "USE_LOG_DEPTH_FRAG_WORLD_POSITION", "");
#ifndef ENGINE_FORCE_NO_LOG_DEPTH
                    ShaderHelper::insertStringRightAfterLineContent(m_Code, getLogDepthFunctions(), "uniform sampler2D gDepthMap;");
#else
                    ShaderHelper::insertStringRightAfterLineContent(m_Code, getNormalDepthFunctions(), "uniform sampler2D gDepthMap;");
#endif
                }else{
#ifndef ENGINE_FORCE_NO_LOG_DEPTH
                    ShaderHelper::insertStringRightAfterLineContent(m_Code, getLogDepthFunctions(), "uniform sampler2D gDepthMap;");
#else
                    ShaderHelper::insertStringRightAfterLineContent(m_Code, getNormalDepthFunctions(), "uniform sampler2D gDepthMap;");
#endif
                }
            }
        }
    }
    opengl::glsl::VersionConversion::convert(m_Code, versionNumber, m_Type);
}