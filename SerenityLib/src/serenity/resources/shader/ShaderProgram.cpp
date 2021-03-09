
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

constexpr ShaderProgram::BindFunc DefaultShaderBindFunctor = [](ShaderProgram* shaderProgram) {
    Scene* scene   = Engine::Resources::getCurrentScene();
    if (!scene) {
        return;
    }
    Camera* camera = scene->getActiveCamera();
    if (!camera) {
        return;
    }
    //yes this is needed
    if (priv::RenderModule::GLSL_VERSION < 140) {
        Engine::Renderer::sendUniformMatrix4Safe("CameraViewProj", camera->getViewProjection());
    }
};

ShaderProgram::ShaderProgram(std::string_view in_name, Handle vsHandle, Handle fsHandle)
    : Resource{ ResourceType::ShaderProgram, in_name }
    , m_VertexShader{ vsHandle }
    , m_FragmentShader{ fsHandle }
{
    setName(in_name);
    const std::string& name_ = name();
    auto& vs = *vsHandle.get<Shader>();
    auto& fs = *fsHandle.get<Shader>();
    if (vs.name() == "NULL") {
        vs.setName(name_ + ".vert");
    }
    if (fs.name() == "NULL") {
        fs.setName(name_ + ".frag");
    }
    setCustomBindFunctor(DefaultShaderBindFunctor);
    load();
}
ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept 
    : Resource{ std::move(other) }
    , m_VertexShader      { std::exchange(other.m_VertexShader, Handle{}) }
    , m_FragmentShader    { std::exchange(other.m_FragmentShader, Handle{}) }
    , m_CustomBindFunctor { std::move(other.m_CustomBindFunctor) }
    , m_ShaderProgram     { std::move(other.m_ShaderProgram) }
    , m_UniformLocations  { std::move(other.m_UniformLocations) }
    , m_AttachedUBOs      { std::move(other.m_AttachedUBOs) }
    , m_LoadedCPU         { std::move(other.m_LoadedCPU) }
    , m_LoadedGPU         { std::move(other.m_LoadedGPU) }
{}
ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept {
    Resource::operator=(std::move(other));
    m_VertexShader       = std::exchange(other.m_VertexShader, Handle{});
    m_FragmentShader     = std::exchange(other.m_FragmentShader, Handle{});
    m_CustomBindFunctor  = std::move(other.m_CustomBindFunctor);
    m_ShaderProgram      = std::move(other.m_ShaderProgram);
    m_UniformLocations   = std::move(other.m_UniformLocations);
    m_AttachedUBOs       = std::move(other.m_AttachedUBOs);
    m_LoadedCPU          = std::move(other.m_LoadedCPU);
    m_LoadedGPU          = std::move(other.m_LoadedGPU);
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
    GLCall(GLuint shaderID = glCreateShader(GLShaderType));
    GLCall(glShaderSource(shaderID, 1, &sourceCode, NULL));
    GLCall(glCompileShader(shaderID));
    // Check Vertex Shader
    GLCall(glGetShaderiv(shaderID, GL_COMPILE_STATUS, &res));
    GLCall(glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength));
    std::vector<char> infoErrors( logLength );
    if (infoErrors.size() > 0) {
        GLCall(glGetShaderInfoLog(shaderID, logLength, NULL, &infoErrors[0]));
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
bool PublicShaderProgram::LinkShadersToProgram(ShaderProgram& shaderProgram, std::vector<uint32_t>&& shaderIDs, std::vector<char>& errors, const char* fragmentSourceCode) {
    GLCall(shaderProgram.m_ShaderProgram = glCreateProgram());
    for (const auto shaderID : shaderIDs) {
        GLCall(glAttachShader(shaderProgram.m_ShaderProgram, shaderID));
    }
    for (uint32_t i = 0; i < 100; ++i) {
        std::string outFragCol = "out vec4 FRAG_COL_" + std::to_string(i) + ";";
        if (ShaderHelper::sfind(fragmentSourceCode, outFragCol)) {
            GLCall(glBindFragDataLocation(shaderProgram.m_ShaderProgram, i, std::string{ "FRAG_COL_" + std::to_string(i) }.c_str()));
        }
    }
    GLCall(glLinkProgram(shaderProgram.m_ShaderProgram));
    for (const auto shaderID : shaderIDs) {
        GLCall(glDetachShader(shaderProgram.m_ShaderProgram, shaderID));
    }
    for (const auto shaderID : shaderIDs) {
        GLCall(glDeleteShader(shaderID));
    }
    // Check the program
    GLint res = GL_FALSE;
    int logLength;
    GLCall(glGetProgramiv(shaderProgram.m_ShaderProgram, GL_LINK_STATUS, &res));
    GLCall(glGetProgramiv(shaderProgram.m_ShaderProgram, GL_INFO_LOG_LENGTH, &logLength));
    errors.resize(std::max(logLength, (int)1));
    GLCall(glGetProgramInfoLog(shaderProgram.m_ShaderProgram, logLength, NULL, &errors[0]));
    return static_cast<bool>(res);
}
void PublicShaderProgram::PopulateUniformTable(ShaderProgram& shaderProgram) {
    GLint                   uniformCount;
    GLint                   uniformSize;
    GLenum                  uniformType;
    std::array<GLchar, 256> uniformName;
    GLsizei                 uniformLength;
    GLCall(glGetProgramiv(shaderProgram.m_ShaderProgram, GL_ACTIVE_UNIFORMS, &uniformCount));
    for (GLint i = 0; i < uniformCount; ++i) {
        GLCall(glGetActiveUniform(shaderProgram.m_ShaderProgram, (GLuint)i, (GLsizei)uniformName.size(), &uniformLength, &uniformSize, &uniformType, uniformName.data()));
        if (uniformLength > 0) {
            GLCall(GLint loc_ = glGetUniformLocation(shaderProgram.m_ShaderProgram, uniformName.data()));
            shaderProgram.m_UniformLocations.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(std::string{ (char*)uniformName.data(), (size_t)uniformLength }), 
                std::forward_as_tuple(loc_)
            );
        }
    }
}
void PublicShaderProgram::LoadGPU(ShaderProgram& shaderProgram){
    PublicShaderProgram::UnloadGPU(shaderProgram);
    if (!shaderProgram.m_LoadedGPU) {
        auto& vertexShader       = *shaderProgram.m_VertexShader.get<Shader>();
        auto& fragmentShader     = *shaderProgram.m_FragmentShader.get<Shader>();
        const auto& VertexCode   = vertexShader.m_Code;
        const auto& FragmentCode = fragmentShader.m_Code;

        std::vector<char> programErrors;
        auto vid        = PublicShaderProgram::CompileShader(vertexShader, VertexCode.c_str(),   GL_VERTEX_SHADER);
        auto fid        = PublicShaderProgram::CompileShader(fragmentShader, FragmentCode.c_str(), GL_FRAGMENT_SHADER);
        auto linkStatus = PublicShaderProgram::LinkShadersToProgram(shaderProgram, { vid, fid }, programErrors, FragmentCode.c_str());
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
        GLCall(glDeleteProgram(shaderProgram.m_ShaderProgram));
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

