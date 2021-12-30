#include <serenity/resources/shader/Shader.h>
#include <serenity/resources/shader/ShaderHelper.h>

#include <serenity/renderer/opengl/State.h>

#include <serenity/renderer/opengl/glsl/Common.h>
#include <serenity/renderer/opengl/glsl/Compression.h>
#include <serenity/renderer/opengl/glsl/DeferredFunctions.h>
#include <serenity/renderer/opengl/glsl/VersionConversion.h>
#include <serenity/renderer/opengl/glsl/Materials.h>
#include <serenity/renderer/opengl/glsl/Lighting.h>
#include <serenity/renderer/opengl/glsl/Shadows.h>
#include <serenity/renderer/opengl/glsl/SSAOCode.h>
#include <serenity/renderer/opengl/glsl/DepthOfFieldCode.h>

#include <serenity/system/Engine.h>
#include <serenity/editor/core/EditorCore.h>

#include <fstream>
#include <regex>
#include <filesystem>

namespace {
    uint32_t CREATED_SHADER_COUNT = 0;

    void load_code_from_file(const std::string& filename, std::string& code) {
        ASSERT(std::filesystem::is_regular_file(filename), "");
        std::ifstream filestream(filename);
        filestream.seekg(0, std::ios::end);
        size_t size = filestream.tellg();
        code.resize(size, ' ');
        filestream.seekg(0);
        filestream.read(code.data(), size);
    }
    std::string get_version_line(const std::string& code) {
        std::string versionLine;
        std::istringstream strstream{ code };
        if (Engine::priv::ShaderHelper::sfind(code, "#version ")) {
            //use the found one
            while (std::getline(strstream, versionLine)) {
                if (Engine::priv::ShaderHelper::sfind(versionLine, "#version ")) {
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
        return versionLine;
    }
    void convert_code(std::string& shaderCode, Shader& shader) {
        //get / generate a version line
        const std::string versionLine = get_version_line(shaderCode);

        const uint32_t versionNumber = uint32_t(std::stoi(std::regex_replace(versionLine, std::regex("([^0-9])"), "")));

        Engine::priv::opengl::glsl::Materials::convert(shaderCode, versionNumber, shader.getType());
        Engine::priv::opengl::glsl::Lighting::convert(shaderCode, versionNumber, shader.getType());
        Engine::priv::opengl::glsl::Shadows::convert(shaderCode, versionNumber, shader.getType());
        Engine::priv::opengl::glsl::SSAOCode::convert(shaderCode, versionNumber, shader.getType());
        Engine::priv::opengl::glsl::Compression::convert(shaderCode, versionNumber, shader.getType());
        Engine::priv::opengl::glsl::DepthOfFieldCode::convert(shaderCode, versionNumber, shader.getType());
        Engine::priv::opengl::glsl::DeferredFunctions::convert(shaderCode, versionNumber, shader.getType());
        Engine::priv::opengl::glsl::Common::convert(shaderCode, versionNumber, shader.getType());
        Engine::priv::opengl::glsl::VersionConversion::convert(shaderCode, versionNumber, shader.getType());

        //put the version on top as the last step
        shaderCode = versionLine + shaderCode;
    }
    void internal_unload(bool isLoaded, GLuint& GLShaderID, Shader& shader) {
        if (isLoaded) {
            if (GLShaderID) {
                glDeleteShader(GLShaderID);
                GLShaderID = 0;
            }
            shader.Resource::unload();
        }
    }
    void send_code_to_editor(std::string& shaderCode, Shader& shader) {
        if (Engine::priv::Core::m_Engine->m_Editor.isEnabled()) {
            Engine::priv::Core::m_Engine->m_Editor.addShaderData(shader, shaderCode);
        }
    }
}

Shader::Shader(std::string_view filenameOrCode, ShaderType shaderType)
    : Resource    { ResourceType::Shader }
    , m_FileName  { filenameOrCode }
    , m_ShaderType{ shaderType }
{
    if (std::filesystem::is_regular_file(m_FileName)) {
        setName(m_FileName);
    } else {
        setName("Shader " + std::to_string(CREATED_SHADER_COUNT++));
    }
    load();
}
Shader::Shader(Shader&& other) noexcept 
    : Resource{ std::move(other) }
    , m_FileName  { std::exchange(other.m_FileName, std::string{}) }
    , m_Code      { std::exchange(other.m_Code, std::string{}) }
    , m_ShaderType{ std::move(other.m_ShaderType) }
    , m_GLShaderID{ std::exchange(other.m_GLShaderID, 0) }
{}
Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        Resource::operator=(std::move(other));
        m_FileName   = std::exchange(other.m_FileName, std::string{});
        m_Code       = std::exchange(other.m_Code, std::string{});
        m_ShaderType = std::move(other.m_ShaderType);
        m_GLShaderID = std::exchange(other.m_GLShaderID, 0);
    }
    return *this;
}
Shader::~Shader() {
    unload();
}

void Shader::load(const std::string& code) {
    unload();
    if (!m_IsLoaded && !code.empty()) {
        m_Code = code;
        Resource::load();
    }
}
void Shader::load() {
    if (!m_IsLoaded || m_Code.empty()) {
        if (std::filesystem::is_regular_file(m_FileName)) {
            //load initial code from file
            load_code_from_file(m_FileName, m_Code);
        } else {
            ASSERT(!m_FileName.empty(), "");
            m_Code = std::move(m_FileName);
            m_FileName.clear();
        }
        convert_code(m_Code, *this);
        send_code_to_editor(m_Code, *this);
        Resource::load();
    }
}
void Shader::unload() {
    Shader::orphan();
    internal_unload(m_IsLoaded, m_GLShaderID, *this);
}
void Shader::orphan(bool force) {
    if (!m_FileName.empty()) {
        m_Code.clear();
    } else if (force) {
        m_Code.clear();
        internal_unload(true, m_GLShaderID, *this);
    }
}