#include <serenity/resources/shader/Shader.h>
#include <serenity/resources/shader/ShaderHelper.h>

#include <serenity/renderer/opengl/APIStateOpenGL.h>

#include <serenity/renderer/opengl/glsl/Common.h>
#include <serenity/renderer/opengl/glsl/Compression.h>
#include <serenity/renderer/opengl/glsl/DeferredFunctions.h>
#include <serenity/renderer/opengl/glsl/VersionConversion.h>
#include <serenity/renderer/opengl/glsl/Materials.h>
#include <serenity/renderer/opengl/glsl/Lighting.h>
#include <serenity/renderer/opengl/glsl/Shadows.h>
#include <serenity/renderer/opengl/glsl/DepthOfFieldCode.h>

#include <serenity/system/EngineIncludes.h>
#include <serenity/editor/core/EditorCore.h>

#include <fstream>
#include <regex>
#include <filesystem>

namespace {
    uint32_t CREATED_SHADER_COUNT = 0;

    void send_code_to_editor(std::string& shaderCode, Shader& shader) {
        if (Engine::getEditor().isEnabled()) {
            Engine::getEditor().addShaderData(shader, shaderCode);
        }
    }
    void load_code_from_file(const std::string& filename, std::string& code) {
        assert(std::filesystem::is_regular_file(filename));
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
            if (Engine::priv::APIState<Engine::priv::OpenGL>::getConstants().GLSL_VERSION >= 330 && Engine::priv::APIState<Engine::priv::OpenGL>::getConstants().GLSL_VERSION < 440) {
                core = " core";
            }
            versionLine = "#version " + std::to_string(Engine::priv::APIState<Engine::priv::OpenGL>::getConstants().GLSL_VERSION) + core + "\n";
        }
        return versionLine;
    }
    bool prune_code_of_comments(std::string& shaderCode) {
        std::string copy;
        copy.reserve(shaderCode.size());
        bool anythingChanged = false;
        //TODO: prune all commented code
        bool isCommentBlock = false;
        bool isCommentLine = false;
        auto itr = std::next(std::begin(shaderCode));
        copy += *std::prev(itr);
        for (; itr != std::end(shaderCode); ++itr) {
            const char prevChar = *std::prev(itr);
            const char currChar = *itr;
            if (prevChar == '/' && currChar == '/') {
                isCommentLine = true;
                copy.back() = ' ';
                anythingChanged = true;
            } else if (currChar == '\n') {
                isCommentLine = false;
            }
            if (!isCommentLine && (prevChar == '/' && currChar == '*')) {
                isCommentBlock = true;
                copy.back() = ' ';
                anythingChanged = true;
            } else if (prevChar == '*' && currChar == '/') {
                isCommentBlock = false;
            }
            if (!isCommentLine && !isCommentBlock) {
                copy += currChar;
            }
        }
        shaderCode = std::move(copy);
        return anythingChanged;
    }
    void convert_code(std::string& shaderCode, Shader& shader) {
        bool codeWasPruned = prune_code_of_comments(shaderCode);


        //get / generate a version line
        const std::string versionLine = get_version_line(shaderCode);

        const uint32_t versionNumber = uint32_t(std::stoi(std::regex_replace(versionLine, std::regex("([^0-9])"), "")));

        Engine::priv::opengl::glsl::Materials::convert(shaderCode, versionNumber, shader.getType());
        Engine::priv::opengl::glsl::Lighting::convert(shaderCode, versionNumber, shader.getType());
        Engine::priv::opengl::glsl::Shadows::convert(shaderCode, versionNumber, shader.getType());
        Engine::priv::opengl::glsl::Compression::convert(shaderCode, versionNumber, shader.getType());
        Engine::priv::opengl::glsl::DepthOfFieldCode::convert(shaderCode, versionNumber, shader.getType());
        Engine::priv::opengl::glsl::DeferredFunctions::convert(shaderCode, versionNumber, shader.getType());
        Engine::priv::opengl::glsl::Common::convert(shaderCode, versionNumber, shader.getType());
        Engine::priv::opengl::glsl::VersionConversion::convert(shaderCode, versionNumber, shader.getType());

        //put the version on top as the last step
        shaderCode = versionLine + shaderCode;
    }
    void internal_load(bool isLoaded, std::string& shaderCode, std::string& filename, Shader& shader, bool dispatchEventLoaded) {
        if (!isLoaded || shaderCode.empty()) {
            if (std::filesystem::is_regular_file(filename)) {
                //load initial code from file
                load_code_from_file(filename, shaderCode);
            }
            else {
                assert(!filename.empty());
                shaderCode = std::move(filename);
                filename.clear();
            }
            convert_code(shaderCode, shader);
            send_code_to_editor(shaderCode, shader);
            shader.load(dispatchEventLoaded);
        }
    }
    void internal_unload(GLuint& GLShaderID, Shader& shader, bool doDispatchEvent) {
        if (GLShaderID) {
            glDeleteShader(GLShaderID);
            GLShaderID = 0;
        }
        shader.unload(doDispatchEvent);
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


    internal_load(m_IsLoaded, m_Code, m_FileName, *this, true);
}
Shader::Shader(Shader&& other) noexcept 
    : Resource{ std::move(other) }
    , m_FileName  { std::move(other.m_FileName) }
    , m_Code      { std::move(other.m_Code) }
    , m_ShaderType{ std::move(other.m_ShaderType) }
    , m_GLShaderID{ std::exchange(other.m_GLShaderID, 0) }
{}
Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        Resource::operator=(std::move(other));
        m_FileName   = std::move(other.m_FileName);
        m_Code       = std::move(other.m_Code);
        m_ShaderType = std::move(other.m_ShaderType);
        m_GLShaderID = std::exchange(other.m_GLShaderID, 0);
    }
    return *this;
}
Shader::~Shader() {
    orphan();
    internal_unload(m_GLShaderID, *this, true);
}

void Shader::loadCode(const std::string& code) {
    orphan();
    internal_unload(m_GLShaderID, *this, true);
    m_Code = code;
    internal_load(m_IsLoaded, m_Code, m_FileName, *this, true);
}
void Shader::orphan(bool force) {
    if (!m_FileName.empty()) {
        m_Code.clear();
    } else if (force) {
        m_Code.clear();
        internal_unload(m_GLShaderID, *this, true);
    }
}