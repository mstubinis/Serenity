
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

#include <serenity/threading/ThreadingModule.h>

#include <serenity/renderer/opengl/APIStateOpenGL.h>

Handle ShaderProgram::Deferred = {};
Handle ShaderProgram::Forward  = {};
Handle ShaderProgram::Decal    = {};

// https://www.khronos.org/opengl/wiki/Shader_Compilation

struct ShaderTypeInfo {
    const char*  extension;
    GLuint       glType;
};
struct ShaderProgramParameters {
    Handle vertexShaderHandle;
    Handle fragmentShaderHandle;
    Handle geometryShaderHandle;
    Handle computeShaderHandle;
    Handle tesselationControlShaderHandle;
    Handle tesselationEvaluationShaderHandle;
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
    constexpr auto DEFAULT_SHADER_BIND_FUNC = [](ShaderProgram* shaderProgram) {
        Scene* scene = Engine::Resources::getCurrentScene();
        //yes this is needed. TODO: remove backwards support for opengl versions not supporting UBO's?
        if (!Engine::priv::APIState<Engine::priv::OpenGL>::supportsUBO()) {
            if (scene && scene->getActiveCamera()) {
                Engine::Renderer::sendUniformMatrix4Safe("CameraViewProj", scene->getActiveCamera()->getViewProjection());
            }
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
    void populate_uniform_table(GLuint shaderProgramID, Engine::unordered_string_map<std::string, int32_t>& uniformContainer) {
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
                if (Engine::priv::ShaderHelper::sfind(fragmentSourceCode, outFragCol)) {
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


class ShaderProgram::Impl {
    public:
        static void Load(ShaderProgram& program, bool dispatchEventLoaded) {
            if (!program.isLoaded()) {
                Engine::priv::PublicShaderProgram::LoadCPU(program);
                Engine::priv::PublicShaderProgram::LoadGPU(program, dispatchEventLoaded);
            }
        }
        static void Unload(ShaderProgram& program, bool dispatchEventUnloaded) {
            if (program.isLoaded()) {
                Engine::priv::PublicShaderProgram::UnloadGPU(program);
                Engine::priv::PublicShaderProgram::UnloadCPU(program, dispatchEventUnloaded);
            }
        }
        static void internal_init(ShaderProgram& program, std::string_view inName, const ShaderProgramParameters& parameters) {
            emplace_shader(program.m_Shaders, parameters.vertexShaderHandle, ShaderType::Vertex);
            emplace_shader(program.m_Shaders, parameters.fragmentShaderHandle, ShaderType::Fragment);
            emplace_shader(program.m_Shaders, parameters.geometryShaderHandle, ShaderType::Geometry);
            emplace_shader(program.m_Shaders, parameters.computeShaderHandle, ShaderType::Compute);
            emplace_shader(program.m_Shaders, parameters.tesselationControlShaderHandle, ShaderType::TessellationControl);
            emplace_shader(program.m_Shaders, parameters.tesselationEvaluationShaderHandle, ShaderType::TessellationEvaluation);

            program.setName(std::string{ inName });
            program.setCustomBindFunctor(DEFAULT_SHADER_BIND_FUNC);
            Impl::Load(program, true);
        }
};

#pragma region ShaderProgram

ShaderProgram::ShaderProgram(std::string_view shaderProgramName, Handle vertexShaderHandle, Handle fragmentShaderHandle)
    : Resource{ ResourceType::ShaderProgram, shaderProgramName }
{
    ShaderProgramParameters parameters;
    parameters.vertexShaderHandle   = vertexShaderHandle;
    parameters.fragmentShaderHandle = fragmentShaderHandle;
    Impl::internal_init(*this, shaderProgramName, parameters);
}
ShaderProgram::ShaderProgram(std::string_view shaderProgramName, Handle vertexShaderHandle, Handle fragmentShaderHandle, Handle geometryShaderHandle)
    : Resource{ ResourceType::ShaderProgram, shaderProgramName }
{
    ShaderProgramParameters parameters;
    parameters.vertexShaderHandle   = vertexShaderHandle;
    parameters.fragmentShaderHandle = fragmentShaderHandle;
    parameters.geometryShaderHandle = geometryShaderHandle;
    Impl::internal_init(*this, shaderProgramName, parameters);
}
ShaderProgram::ShaderProgram(std::string_view shaderProgramName, std::string_view vertexShaderFileOrContent, std::string_view fragmentShaderFileOrContent) 
    : Resource{ ResourceType::ShaderProgram, shaderProgramName }
{
    ShaderProgramParameters parameters;
    parameters.vertexShaderHandle   = Engine::Resources::loadShader(vertexShaderFileOrContent, ShaderType::Vertex);
    parameters.fragmentShaderHandle = Engine::Resources::loadShader(fragmentShaderFileOrContent, ShaderType::Fragment);
    Impl::internal_init(*this, shaderProgramName, parameters);
}
ShaderProgram::ShaderProgram(std::string_view shaderProgramName, const ShaderProgramParameters& parameters)
    : Resource{ ResourceType::ShaderProgram, shaderProgramName }
{
    Impl::internal_init(*this, shaderProgramName, parameters);
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
    Impl::Unload(*this, true);
}

#pragma endregion


#pragma region PublicShaderProgram

void Engine::priv::PublicShaderProgram::LoadCPU(ShaderProgram& shaderProgram){
    PublicShaderProgram::UnloadCPU(shaderProgram);
    if (!shaderProgram.m_LoadedCPU) {
        shaderProgram.m_LoadedCPU = true;
    }
}
void Engine::priv::PublicShaderProgram::LoadGPU(ShaderProgram& shaderProgram, bool dispatchEventUnloaded){
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
void Engine::priv::PublicShaderProgram::UnloadCPU(ShaderProgram& shaderProgram, bool dispatchEventLoaded) {
    if (shaderProgram.m_LoadedCPU) {
        shaderProgram.m_LoadedCPU = false;
    }
    shaderProgram.Resource::unload(dispatchEventLoaded);
}
void Engine::priv::PublicShaderProgram::UnloadGPU(ShaderProgram& shaderProgram) {
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

#pragma endregion