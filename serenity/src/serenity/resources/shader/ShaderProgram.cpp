
#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/resources/shader/ShaderHelper.h>
#include <serenity/resources/shader/Shader.h>

#include <serenity/scene/Camera.h>
#include <serenity/scene/Scene.h>
#include <serenity/system/Engine.h>
#include <serenity/renderer/opengl/UniformBufferObject.h>
#include <serenity/resources/Engine_Resources.h>
#include <array>
#include <filesystem>

using namespace Engine;
using namespace Engine::priv;

Handle ShaderProgram::Deferred = {};
Handle ShaderProgram::Forward  = {};
Handle ShaderProgram::Decal    = {};

// https://www.khronos.org/opengl/wiki/Shader_Compilation

struct ShaderTypeInfo {
    const char*  extension;
    GLuint       glType;
};
namespace {
    constexpr std::array<ShaderTypeInfo, ShaderType::_TOTAL> SHADER_TYPE_INFO{ {
        { ".vert", GL_VERTEX_SHADER },
        { ".frag", GL_FRAGMENT_SHADER },
        { ".geom", GL_GEOMETRY_SHADER },
        { ".comp", GL_COMPUTE_SHADER },         // (requires GL 4.3 or ARB_compute_shader)
        { ".tess", GL_TESS_CONTROL_SHADER },    // (requires GL 4.0 or ARB_tessellation_shader)
        { ".tess", GL_TESS_EVALUATION_SHADER }, // (requires GL 4.0 or ARB_tessellation_shader)
    } };
    constexpr ShaderProgram::BindFunc DEFAULT_SHADER_BIND_FUNC = [](ShaderProgram* shaderProgram) {
        Scene* scene = Engine::Resources::getCurrentScene();
        if (!scene) {
            return;
        }
        Camera* camera = scene->getActiveCamera();
        if (!camera) {
            return;
        }
        //yes this is needed. TODO: remove backwards support for opengl versions not supporting UBO's?
        if (!Engine::priv::OpenGLState::constants.supportsUBO()) {
            Engine::Renderer::sendUniformMatrix4Safe("CameraViewProj", camera->getViewProjection());
        }
    };

    void emplace_shader(std::vector<std::pair<Handle, ShaderType>>& shaders, Handle shaderHandle, ShaderType shaderType) {
        if (shaderHandle) {
            shaders.emplace_back(shaderHandle, shaderType);
        }
    }
    void compile_shader(Shader& shader, GLuint& GLShaderID, const char* sourceCode, uint32_t GLShaderType) {
        if (GLShaderID == 0) {
            GLShaderID = glCreateShader(GLShaderType);
            glShaderSource(GLShaderID, 1, &sourceCode, NULL);
            glCompileShader(GLShaderID);

            // Check Shader
            GLint shaderCompilationStatus = GL_FALSE;
            glGetShaderiv(GLShaderID, GL_COMPILE_STATUS, &shaderCompilationStatus);
            if (shaderCompilationStatus == GL_FALSE) {
                #ifndef ENGINE_PRODUCTION
                    //logging
                    int logLength = 0;
                    glGetShaderiv(GLShaderID, GL_INFO_LOG_LENGTH, &logLength);
                    if (logLength > 0) {
                        std::vector<char> infoErrors(logLength);
                        glGetShaderInfoLog(GLShaderID, GLsizei(infoErrors.size()), NULL, infoErrors.data());
                        ENGINE_PRODUCTION_LOG("Shader Log (" + ((shader.isFromFile() && !shader.getFilename().empty()) ? shader.getFilename() : shader.name()) + "): ");
                        ENGINE_PRODUCTION_LOG(infoErrors.data());
                    }
                #endif
                glDeleteShader(GLShaderID);
                GLShaderID = 0;
            }
        } else {

        }
    }
    void populate_uniform_table(GLuint shaderProgramID, ShaderProgram::UniformsContainer& uniformContainer) {
        GLint                   uniformCount;
        GLint                   uniformSize;
        GLenum                  uniformType;
        std::array<GLchar, 256> uniformName;
        GLsizei                 uniformLength;
        glGetProgramiv(shaderProgramID, GL_ACTIVE_UNIFORMS, &uniformCount);
        for (GLint i = 0; i < uniformCount; ++i) {
            glGetActiveUniform(shaderProgramID, GLuint(i), GLsizei(uniformName.size()), &uniformLength, &uniformSize, &uniformType, uniformName.data());
            if (uniformLength > 0) {
                GLint uniformLocationNum = glGetUniformLocation(shaderProgramID, uniformName.data());
                uniformContainer.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(std::string{ static_cast<char*>(uniformName.data()), size_t(uniformLength) }),
                    std::forward_as_tuple(uniformLocationNum)
                );
            }
        }
    }
    bool link_shaders_to_program(GLuint& shaderProgramID, const std::vector<std::pair<Handle, ShaderType>>& shaders, const char* fragmentSourceCode = "") {
        if (shaderProgramID == 0) {
            shaderProgramID = glCreateProgram();

            std::vector<GLuint> shaderIDs(shaders.size());
            for (size_t i = 0; i < shaderIDs.size(); ++i) {
                shaderIDs[i] = GLuint(shaders[i].first.get<Shader>()->getGLShaderID());
            }
            for (const GLuint shaderID : shaderIDs) {
                glAttachShader(shaderProgramID, shaderID);
            }
            GLint MAX_DRAW_BUFFERS;
            glGetIntegerv(GL_MAX_DRAW_BUFFERS, &MAX_DRAW_BUFFERS);
            for (GLint i = 0; i < MAX_DRAW_BUFFERS; ++i) {
                std::string outFragCol = "out vec4 FRAG_COL_" + std::to_string(i) + ";";
                if (ShaderHelper::sfind(fragmentSourceCode, outFragCol)) {
                    glBindFragDataLocation(shaderProgramID, i, std::string{ "FRAG_COL_" + std::to_string(i) }.c_str());
                }
            }
            glLinkProgram(shaderProgramID);
            for (const GLuint shaderID : shaderIDs) {
                glDetachShader(shaderProgramID, shaderID);
            }
            //for (const GLuint shaderID : shaderIDs) {
            //    glDeleteShader(shaderID);
            //}
            
            // Check the program
            GLint linkStatus = GL_FALSE;
            glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &linkStatus);
            #ifndef ENGINE_PRODUCTION
                //logging
                int logLength = 0;
                glGetProgramiv(shaderProgramID, GL_INFO_LOG_LENGTH, &logLength);
                if (logLength > 0) {
                    std::vector<char> programErrors(logLength);
                    glGetProgramInfoLog(shaderProgramID, GLsizei(programErrors.size()), NULL, programErrors.data());
                    ENGINE_PRODUCTION_LOG("ShaderProgram Log: ");
                    ENGINE_PRODUCTION_LOG(programErrors.data());
                }
            #endif
            if (linkStatus == GL_FALSE) {
                glDeleteProgram(shaderProgramID);
                shaderProgramID = 0;
            }
            return static_cast<bool>(linkStatus);
        } else {

        }
        return true;
    }
}

ShaderProgram::ShaderProgram(std::string_view shaderProgramName, Handle vertexShaderHandle, Handle fragmentShaderHandle)
    : Resource{ ResourceType::ShaderProgram, shaderProgramName }
{
    ShaderProgramParameters parameters;
    parameters.vertexShaderHandle   = vertexShaderHandle;
    parameters.fragmentShaderHandle = fragmentShaderHandle;
    internal_init(shaderProgramName, parameters);
}
ShaderProgram::ShaderProgram(std::string_view shaderProgramName, Handle vertexShaderHandle, Handle fragmentShaderHandle, Handle geometryShaderHandle)
    : Resource{ ResourceType::ShaderProgram, shaderProgramName }
{
    ShaderProgramParameters parameters;
    parameters.vertexShaderHandle   = vertexShaderHandle;
    parameters.fragmentShaderHandle = fragmentShaderHandle;
    parameters.geometryShaderHandle = geometryShaderHandle;
    internal_init(shaderProgramName, parameters);
}
ShaderProgram::ShaderProgram(std::string_view shaderProgramName, std::string_view vertexShaderFileOrContent, std::string_view fragmentShaderFileOrContent) 
    : Resource{ ResourceType::ShaderProgram, shaderProgramName }
{
    ShaderProgramParameters parameters;
    parameters.vertexShaderHandle   = Engine::Resources::loadShader(vertexShaderFileOrContent, ShaderType::Vertex);
    parameters.fragmentShaderHandle = Engine::Resources::loadShader(fragmentShaderFileOrContent, ShaderType::Fragment);
    internal_init(shaderProgramName, parameters);
}
ShaderProgram::ShaderProgram(std::string_view shaderProgramName, const ShaderProgramParameters& parameters)
    : Resource{ ResourceType::ShaderProgram, shaderProgramName }
{
    internal_init(shaderProgramName, parameters);
}
void ShaderProgram::internal_init(std::string_view inName, const ShaderProgramParameters& parameters) {
    emplace_shader(m_Shaders, parameters.vertexShaderHandle, ShaderType::Vertex);
    emplace_shader(m_Shaders, parameters.fragmentShaderHandle, ShaderType::Fragment);
    emplace_shader(m_Shaders, parameters.geometryShaderHandle, ShaderType::Geometry);
    emplace_shader(m_Shaders, parameters.computeShaderHandle, ShaderType::Compute);
    emplace_shader(m_Shaders, parameters.tesselationControlShaderHandle, ShaderType::TessellationControl);
    emplace_shader(m_Shaders, parameters.tesselationEvaluationShaderHandle, ShaderType::TessellationEvaluation);

    setName(inName);
    setCustomBindFunctor(DEFAULT_SHADER_BIND_FUNC);
    load();
}
ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept 
    : Resource{ std::move(other) }
    , m_CustomBindFunctor { std::move(other.m_CustomBindFunctor) }
    , m_UniformLocations  { std::move(other.m_UniformLocations) }
    , m_AttachedUBOs      { std::move(other.m_AttachedUBOs) }
    , m_Shaders           { std::move(other.m_Shaders) }
    , m_ShaderProgram     { std::exchange(other.m_ShaderProgram, 0U) }
    , m_LoadedCPU         { std::exchange(other.m_LoadedCPU, false) }
    , m_LoadedGPU         { std::exchange(other.m_LoadedGPU, false) }
{}
ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept {
    if (this != &other) {
        Resource::operator=(std::move(other));
        m_CustomBindFunctor = std::move(other.m_CustomBindFunctor);
        m_UniformLocations  = std::move(other.m_UniformLocations);
        m_AttachedUBOs      = std::move(other.m_AttachedUBOs);
        m_Shaders           = std::move(other.m_Shaders);
        m_ShaderProgram     = std::exchange(other.m_ShaderProgram, 0U);
        m_LoadedCPU         = std::exchange(other.m_LoadedCPU, false);
        m_LoadedGPU         = std::exchange(other.m_LoadedGPU, false);
    }
    return *this;
}
ShaderProgram::~ShaderProgram(){ 
    unload(); 
}
void PublicShaderProgram::LoadCPU(ShaderProgram& shaderProgram){
    PublicShaderProgram::UnloadCPU(shaderProgram);
    if (!shaderProgram.m_LoadedCPU) {
        shaderProgram.m_LoadedCPU = true;
    }
}
void PublicShaderProgram::LoadGPU(ShaderProgram& shaderProgram, bool dispatchEventUnloaded){
    PublicShaderProgram::UnloadGPU(shaderProgram);
    if (!shaderProgram.m_LoadedGPU) {
        const char* FragmentCode = "";
        for (const auto& [shaderHandle, shaderType] : shaderProgram.m_Shaders) {
            Shader* shader = shaderHandle.get<Shader>();
            if (shader) {
                compile_shader(*shader, shader->m_GLShaderID, shader->m_Code.c_str(), SHADER_TYPE_INFO[shaderType].glType);
                if (shaderType == ShaderType::Fragment) {
                    FragmentCode = shader->m_Code.c_str();
                }
            }
        }
        bool linkStatus = link_shaders_to_program(shaderProgram.m_ShaderProgram, shaderProgram.m_Shaders, FragmentCode);
        if (linkStatus != GL_FALSE) {
            populate_uniform_table(shaderProgram.m_ShaderProgram, shaderProgram.m_UniformLocations);
            //plug valid UBOs into this shader program
            auto GetAllUBOs = Engine::Resources::GetAllResourcesOfType<UniformBufferObject>();
            for (const auto& ubo : GetAllUBOs) {
                ubo->attachToShaderProgram(shaderProgram);
            }
        }
        shaderProgram.m_LoadedGPU = true;
    }
    shaderProgram.Resource::load(dispatchEventUnloaded);
}
void PublicShaderProgram::UnloadCPU(ShaderProgram& shaderProgram, bool dispatchEventLoaded) {
    if (shaderProgram.m_LoadedCPU) {
        shaderProgram.m_LoadedCPU = false;
    }
    shaderProgram.Resource::unload(dispatchEventLoaded);
}
void PublicShaderProgram::UnloadGPU(ShaderProgram& shaderProgram) {
    if (shaderProgram.m_LoadedGPU) {
        shaderProgram.m_UniformLocations.clear();
        shaderProgram.m_AttachedUBOs.clear();
        if (shaderProgram.m_ShaderProgram) {
            glDeleteProgram(shaderProgram.m_ShaderProgram);
            shaderProgram.m_ShaderProgram = 0;
        }
        shaderProgram.m_LoadedGPU = false;
    }
}
void ShaderProgram::load(bool dispatchEventLoaded) {
    if(!isLoaded()){
        PublicShaderProgram::LoadCPU(*this);
        PublicShaderProgram::LoadGPU(*this, dispatchEventLoaded);

        //Resource::load(dispatchEventLoaded);
    }
}
void ShaderProgram::unload(bool dispatchEventUnloaded) {
    if(isLoaded()){
        PublicShaderProgram::UnloadGPU(*this);
        PublicShaderProgram::UnloadCPU(*this, dispatchEventUnloaded);

        //Resource::unload(dispatchEventUnloaded);
    }
}