
#include <serenity/resources/shader/Shader.h>
#include <serenity/resources/shader/ShaderHelper.h>
#include <serenity/renderer/Renderer.h>

#include <serenity/renderer/opengl/glsl/Common.h>
#include <serenity/renderer/opengl/glsl/Compression.h>
#include <serenity/renderer/opengl/glsl/DeferredFunctions.h>
#include <serenity/renderer/opengl/glsl/VersionConversion.h>
#include <serenity/renderer/opengl/glsl/Materials.h>
#include <serenity/renderer/opengl/glsl/Lighting.h>
#include <serenity/renderer/opengl/glsl/Shadows.h>
#include <serenity/renderer/opengl/glsl/SSAOCode.h>
#include <serenity/renderer/opengl/glsl/DepthOfFieldCode.h>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <regex>

using boost_stream_mapped_file = boost::iostreams::stream<boost::iostreams::mapped_file_source>;

Shader::Shader(std::string_view filenameOrCode, ShaderType shaderType, bool fromFile)
    : Resource    { ResourceType::Shader }
    , m_FileName  { filenameOrCode }
    , m_ShaderType{ shaderType }
    , m_FromFile  { fromFile }
{
    if (fromFile) {
        setName(filenameOrCode);
        m_FileName = filenameOrCode;
    } else {
        setName("N/A");
        m_Code     = filenameOrCode;
    }
    Engine::priv::PublicShader::ConvertCode(*this);
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

void Engine::priv::PublicShader::ConvertCode(Shader& shader) {
    //load initial code
    if (shader.m_FromFile) {
        std::string code;
        boost_stream_mapped_file strm(shader.m_FileName);
        std::string line;
        for (; std::getline(strm, line, '\n');) {
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
    } else {
        //generate one
        std::string core;
        if (Engine::priv::OpenGLState::constants.GLSL_VERSION >= 330 && Engine::priv::OpenGLState::constants.GLSL_VERSION < 440) {
            core = " core";
        }
        versionLine = "#version " + std::to_string(Engine::priv::OpenGLState::constants.GLSL_VERSION) + core + "\n";
    }
    const auto versionNumber = static_cast<uint32_t>(std::stoi(std::regex_replace(versionLine, std::regex("([^0-9])"), "")));

    Engine::priv::opengl::glsl::Materials::convert(shader.m_Code, versionNumber, shader.m_ShaderType);
    Engine::priv::opengl::glsl::Lighting::convert(shader.m_Code, versionNumber, shader.m_ShaderType);
    Engine::priv::opengl::glsl::Shadows::convert(shader.m_Code, versionNumber, shader.m_ShaderType);
    Engine::priv::opengl::glsl::SSAOCode::convert(shader.m_Code, versionNumber, shader.m_ShaderType);
    Engine::priv::opengl::glsl::Compression::convert(shader.m_Code, versionNumber, shader.m_ShaderType);
    Engine::priv::opengl::glsl::DepthOfFieldCode::convert(shader.m_Code, versionNumber, shader.m_ShaderType);
    Engine::priv::opengl::glsl::DeferredFunctions::convert(shader.m_Code, versionNumber, shader.m_ShaderType);
    Engine::priv::opengl::glsl::Common::convert(shader.m_Code, versionNumber, shader.m_ShaderType);
    Engine::priv::opengl::glsl::VersionConversion::convert(shader.m_Code, versionNumber, shader.m_ShaderType);

    //put the version on top as the last step :)
    shader.m_Code = versionLine + shader.m_Code;
}
