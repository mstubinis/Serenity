
#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/resources/shader/ShaderHelper.h>
#include <serenity/resources/shader/Shader.h>

#include <serenity/scene/Camera.h>
#include <serenity/scene/Scene.h>
#include <serenity/system/Engine.h>
#include <serenity/renderer/opengl/UniformBufferObject.h>
#include <array>

using namespace Engine;
using namespace Engine::priv;

Handle ShaderProgram::Deferred = Handle{};
Handle ShaderProgram::Forward  = Handle{};
Handle ShaderProgram::Decal    = Handle{};

constexpr std::array<std::tuple<const char*, GLuint>, (size_t)ShaderType::_TOTAL> SHADER_TYPE_INFO {
    std::make_tuple( ".vert", GL_VERTEX_SHADER ),
    std::make_tuple( ".frag", GL_FRAGMENT_SHADER ),
    std::make_tuple( ".geom", GL_GEOMETRY_SHADER ),
    std::make_tuple( ".comp", GL_COMPUTE_SHADER ),         // (requires GL 4.3 or ARB_compute_shader)
    std::make_tuple( ".tess", GL_TESS_CONTROL_SHADER ),    // (requires GL 4.0 or ARB_tessellation_shader)
    std::make_tuple( ".tess", GL_TESS_EVALUATION_SHADER ), // (requires GL 4.0 or ARB_tessellation_shader)
};

constexpr ShaderProgram::BindFunc DefaultShaderBindFunctor = [](ShaderProgram* shaderProgram) {
    Scene* scene   = Engine::Resources::getCurrentScene();
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

ShaderProgram::ShaderProgram(std::string_view inName, Handle vertexShaderHandle, Handle fragmentShaderHandle)
    : Resource{ ResourceType::ShaderProgram, inName }
{
    ShaderProgramParameters parameters;
    parameters.vertexShaderHandle = vertexShaderHandle;
    parameters.fragmentShaderHandle = fragmentShaderHandle;
    internal_init(inName, parameters);
}
ShaderProgram::ShaderProgram(std::string_view inName, Handle vertexShaderHandle, Handle fragmentShaderHandle, Handle geometryShaderHandle)
    : Resource{ ResourceType::ShaderProgram, inName }
{
    ShaderProgramParameters parameters;
    parameters.vertexShaderHandle = vertexShaderHandle;
    parameters.fragmentShaderHandle = fragmentShaderHandle;
    parameters.geometryShaderHandle = geometryShaderHandle;
    internal_init(inName, parameters);
}
ShaderProgram::ShaderProgram(std::string_view inName, const ShaderProgramParameters& parameters)
    : Resource{ ResourceType::ShaderProgram, inName }
{
    internal_init(inName, parameters);
}
void ShaderProgram::internal_init(std::string_view inName, const ShaderProgramParameters& parameters) {
    if (parameters.vertexShaderHandle)
        m_Shaders.emplace_back(parameters.vertexShaderHandle, ShaderType::Vertex);
    if (parameters.fragmentShaderHandle)
        m_Shaders.emplace_back(parameters.fragmentShaderHandle, ShaderType::Fragment);
    if (parameters.geometryShaderHandle)
        m_Shaders.emplace_back(parameters.geometryShaderHandle, ShaderType::Geometry);
    if (parameters.computeShaderHandle)
        m_Shaders.emplace_back(parameters.computeShaderHandle, ShaderType::Compute);
    if (parameters.tesselationControlShaderHandle)
        m_Shaders.emplace_back(parameters.tesselationControlShaderHandle, ShaderType::TessellationControl);
    if (parameters.tesselationEvaluationShaderHandle)
        m_Shaders.emplace_back(parameters.tesselationEvaluationShaderHandle, ShaderType::TessellationEvaluation);
    setName(inName);
    const std::string& name_ = name();
    for (int i = 0; i < m_Shaders.size(); ++i) {
        auto shader = m_Shaders[i].first.get<Shader>();
        if (shader && shader->name() == "NULL") {
            shader->setName(name_ + std::get<0>(SHADER_TYPE_INFO[(size_t)m_Shaders[i].second]));
        }
    }
    setCustomBindFunctor(DefaultShaderBindFunctor);
    load();
}
ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept 
    : Resource{ std::move(other) }
    , m_CustomBindFunctor { std::move(other.m_CustomBindFunctor) }
    , m_UniformLocations  { std::move(other.m_UniformLocations) }
    , m_AttachedUBOs      { std::move(other.m_AttachedUBOs) }
    , m_Shaders           { std::move(other.m_Shaders) }
    , m_ShaderProgram     { std::move(other.m_ShaderProgram) }
    , m_LoadedCPU         { std::move(other.m_LoadedCPU) }
    , m_LoadedGPU         { std::move(other.m_LoadedGPU) }
{}
ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept {
    if (this != &other) {
        Resource::operator=(std::move(other));
        m_CustomBindFunctor = std::move(other.m_CustomBindFunctor);
        m_UniformLocations  = std::move(other.m_UniformLocations);
        m_AttachedUBOs      = std::move(other.m_AttachedUBOs);
        m_Shaders           = std::move(other.m_Shaders);
        m_ShaderProgram     = std::move(other.m_ShaderProgram);
        m_LoadedCPU         = std::move(other.m_LoadedCPU);
        m_LoadedGPU         = std::move(other.m_LoadedGPU);
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
uint32_t PublicShaderProgram::CompileShader(Shader& shader, const char* sourceCode, uint32_t GLShaderType) {
    GLint res = GL_FALSE;
    int logLength;
    GLuint shaderID = glCreateShader(GLShaderType);
    glShaderSource(shaderID, 1, &sourceCode, NULL);
    glCompileShader(shaderID);
    // Check Vertex Shader
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &res);
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<char> infoErrors( logLength );
    if (infoErrors.size() > 0) {
        glGetShaderInfoLog(shaderID, logLength, NULL, &infoErrors[0]);
    }
    if (res == GL_FALSE) {
        if (shader.fromFile()) {
            ENGINE_PRODUCTION_LOG("Shader Log (" + shader.m_FileName + "): ")
        }else{
            ENGINE_PRODUCTION_LOG("Shader Log (" + shader.name() + "): ")
        }
        ENGINE_PRODUCTION_LOG(&infoErrors[0])
    }
    return shaderID;
}
bool PublicShaderProgram::LinkShadersToProgram(ShaderProgram& shaderProgram, const std::vector<uint32_t>& shaderIDs, std::vector<char>& errors, const char* fragmentSourceCode) {
    shaderProgram.m_ShaderProgram = glCreateProgram();
    for (const auto shaderID : shaderIDs) {
        glAttachShader(shaderProgram.m_ShaderProgram, shaderID);
    }
    GLint MAX_DRAW_BUFFERS;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &MAX_DRAW_BUFFERS);
    for (uint32_t i = 0; i < (uint32_t)MAX_DRAW_BUFFERS; ++i) {
        std::string outFragCol = "out vec4 FRAG_COL_" + std::to_string(i) + ";";
        if (ShaderHelper::sfind(fragmentSourceCode, outFragCol)) {
            glBindFragDataLocation(shaderProgram.m_ShaderProgram, i, std::string{ "FRAG_COL_" + std::to_string(i) }.c_str());
        }
    }
    glLinkProgram(shaderProgram.m_ShaderProgram);
    for (const auto shaderID : shaderIDs) {
        glDetachShader(shaderProgram.m_ShaderProgram, shaderID);
    }
    for (const auto shaderID : shaderIDs) {
        glDeleteShader(shaderID);
    }
    // Check the program
    GLint res = GL_FALSE;
    int logLength;
    glGetProgramiv(shaderProgram.m_ShaderProgram, GL_LINK_STATUS, &res);
    glGetProgramiv(shaderProgram.m_ShaderProgram, GL_INFO_LOG_LENGTH, &logLength);
    errors.resize(std::max(logLength, (int)1));
    glGetProgramInfoLog(shaderProgram.m_ShaderProgram, logLength, NULL, &errors[0]);
    return static_cast<bool>(res);
}
void PublicShaderProgram::PopulateUniformTable(ShaderProgram& shaderProgram) {
    GLint                   uniformCount;
    GLint                   uniformSize;
    GLenum                  uniformType;
    std::array<GLchar, 256> uniformName;
    GLsizei                 uniformLength;
    glGetProgramiv(shaderProgram.m_ShaderProgram, GL_ACTIVE_UNIFORMS, &uniformCount);
    for (GLint i = 0; i < uniformCount; ++i) {
        glGetActiveUniform(shaderProgram.m_ShaderProgram, (GLuint)i, (GLsizei)uniformName.size(), &uniformLength, &uniformSize, &uniformType, uniformName.data());
        if (uniformLength > 0) {
            GLint uniformLocationNum = glGetUniformLocation(shaderProgram.m_ShaderProgram, uniformName.data());
            shaderProgram.m_UniformLocations.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(std::string{ (char*)uniformName.data(), (size_t)uniformLength }), 
                std::forward_as_tuple(uniformLocationNum)
            );
        }
    }
}
void PublicShaderProgram::LoadGPU(ShaderProgram& shaderProgram){
    PublicShaderProgram::UnloadGPU(shaderProgram);
    if (!shaderProgram.m_LoadedGPU) {
        std::vector<char> programErrors;
        std::vector<uint32_t> shaderIDs;
        shaderIDs.reserve((size_t)ShaderType::_TOTAL);
        const char* FragmentCode = "";
        for (const auto& data : shaderProgram.m_Shaders) {
            Shader* shader = data.first.get<Shader>();
            if (shader) {
                shaderIDs.push_back(PublicShaderProgram::CompileShader(*shader, shader->m_Code.c_str(), std::get<1>(SHADER_TYPE_INFO[(size_t)data.second])));
                if (data.second == ShaderType::Fragment) {
                    FragmentCode = shader->m_Code.c_str();
                }
            }
        }
        bool linkStatus = PublicShaderProgram::LinkShadersToProgram(shaderProgram, shaderIDs, programErrors, FragmentCode);
        if (linkStatus == GL_FALSE) {
            //TODO: Assert or return failure code?
            ENGINE_PRODUCTION_LOG("ShaderProgram Log : ")
            ENGINE_PRODUCTION_LOG(&programErrors[0])
        }else{
            PopulateUniformTable(shaderProgram);
            //plug valid UBOs into this shader program
            auto ubos = Engine::Resources::GetAllResourcesOfType<UniformBufferObject>();
            for (const auto& ubo : ubos) {
                ubo->attachToShaderProgram(shaderProgram);
            }
        }
        shaderProgram.m_LoadedGPU = true;
    }
    shaderProgram.Resource::load();
}
void PublicShaderProgram::UnloadCPU(ShaderProgram& shaderProgram) {
    if (shaderProgram.m_LoadedCPU) {
        shaderProgram.m_LoadedCPU = false;
    }
    shaderProgram.Resource::unload();
}
void PublicShaderProgram::UnloadGPU(ShaderProgram& shaderProgram) {
    if (shaderProgram.m_LoadedGPU) {
        shaderProgram.m_UniformLocations.clear();
        shaderProgram.m_AttachedUBOs.clear();
        glDeleteProgram(shaderProgram.m_ShaderProgram);
        shaderProgram.m_LoadedGPU = false;
    }
}
void ShaderProgram::load() {
    if(!isLoaded()){
        PublicShaderProgram::LoadCPU(*this);
        PublicShaderProgram::LoadGPU(*this);

        Resource::load();
    }
}
void ShaderProgram::unload() {
    if(isLoaded() /*&& useCount() == 0*/){
        PublicShaderProgram::UnloadGPU(*this);
        PublicShaderProgram::UnloadCPU(*this);

        Resource::unload();
    }
}