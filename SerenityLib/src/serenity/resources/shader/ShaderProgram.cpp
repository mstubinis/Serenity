
#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/resources/shader/ShaderHelper.h>
#include <serenity/resources/shader/Shader.h>

#include <serenity/scene/Camera.h>
#include <serenity/scene/Scene.h>
#include <serenity/system/Engine.h>
#include <serenity/renderer/opengl/UniformBufferObject.h>

using namespace Engine;
using namespace Engine::priv;

Handle ShaderProgram::Deferred = Handle{};
Handle ShaderProgram::Forward  = Handle{};
Handle ShaderProgram::Decal    = Handle{};

namespace Engine::priv {
    struct DefaultShaderBindFunctor{void operator()(ShaderProgram* shaderProgram) const {
        Scene* scene = Resources::getCurrentScene();  
        if (!scene) {
            return;
        }
        Camera* camera = scene->getActiveCamera();    
        if (!camera) {
            return;
        }
        float fcoeff = (2.0f / glm::log2(camera->getFar() + 1.0f)) * 0.5f;
        Engine::Renderer::sendUniform1Safe("fcoeff", fcoeff);

        //yes this is needed
        if(priv::RenderModule::GLSL_VERSION < 140){
            Engine::Renderer::sendUniformMatrix4Safe("CameraViewProj", camera->getViewProjection());
        }
    }};
};

ShaderProgram::ShaderProgram(const std::string& in_name, Handle vsHandle, Handle fsHandle)
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
    setCustomBindFunctor(DefaultShaderBindFunctor());
    load();
}
ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept 
    : Resource(std::move(other))
    , m_VertexShader      { std::exchange(other.m_VertexShader, Handle{})}
    , m_FragmentShader    { std::exchange(other.m_FragmentShader, Handle{}) }
    , m_CustomBindFunctor {std::move(other.m_CustomBindFunctor)}
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

void PublicShaderProgram::LoadCPU(ShaderProgram& shaderP){
    PublicShaderProgram::UnloadCPU(shaderP);
    if (!shaderP.m_LoadedCPU) {
        shaderP.m_LoadedCPU = true;
    }
}
void PublicShaderProgram::LoadGPU(ShaderProgram& shaderP){
    PublicShaderProgram::UnloadGPU(shaderP);
    if (!shaderP.m_LoadedGPU) {
        auto& vs = *shaderP.m_VertexShader.get<Shader>();
        auto& fs = *shaderP.m_FragmentShader.get<Shader>();
        const std::string& VertexCode   = vs.m_Code;
        const std::string& FragmentCode = fs.m_Code;
        GLCall(GLuint vid = glCreateShader(GL_VERTEX_SHADER));
        GLCall(GLuint fid = glCreateShader(GL_FRAGMENT_SHADER));
        GLint res  = GL_FALSE;
        int ll;
        // Compile Vertex Shader
        const char* vss = VertexCode.c_str();
        GLCall(glShaderSource(vid, 1, &vss, NULL));
        GLCall(glCompileShader(vid));
        // Check Vertex Shader
        GLCall(glGetShaderiv(vid, GL_COMPILE_STATUS, &res));
        GLCall(glGetShaderiv(vid, GL_INFO_LOG_LENGTH, &ll));
        std::vector<char> ve(ll);
        if (ve.size() > 0) {
            GLCall(glGetShaderInfoLog(vid, ll, NULL, &ve[0]));
        }
        if (res == GL_FALSE) {
            if (vs.fromFile()) {
                ENGINE_PRODUCTION_LOG("VertexShader Log (" + vs.m_FileName + "): ")
            }else{
                ENGINE_PRODUCTION_LOG("VertexShader Log (" + vs.name() + "): ")
            }
            ENGINE_PRODUCTION_LOG(&ve[0])
        }
        // Compile Fragment Shader
        const char* fss = FragmentCode.c_str();
        GLCall(glShaderSource(fid, 1, &fss, NULL));
        GLCall(glCompileShader(fid));
        // Check Fragment Shader
        GLCall(glGetShaderiv(fid, GL_COMPILE_STATUS, &res));
        GLCall(glGetShaderiv(fid, GL_INFO_LOG_LENGTH, &ll));
        std::vector<char> fe(ll);
        if (fe.size() > 0) {
            GLCall(glGetShaderInfoLog(fid, ll, NULL, &fe[0]));
        }
        if (res == GL_FALSE) {
            if (fs.fromFile()) {
                ENGINE_PRODUCTION_LOG("FragmentShader Log (" + fs.m_FileName + "): ")
            }else{
                ENGINE_PRODUCTION_LOG("FragmentShader Log (" + fs.name() + "): ")
            }
            ENGINE_PRODUCTION_LOG(&fe[0])
        }
        // Link the program id
        GLCall(shaderP.m_ShaderProgram = glCreateProgram());
        GLCall(glAttachShader(shaderP.m_ShaderProgram, vid));
        GLCall(glAttachShader(shaderP.m_ShaderProgram, fid));

        for (unsigned int i = 0; i < 100; ++i) {
            std::string outFragCol = "out vec4 FRAG_COL_" + std::to_string(i) + ";";
            if (ShaderHelper::sfind(FragmentCode, outFragCol)) {
                GLCall(glBindFragDataLocation(shaderP.m_ShaderProgram, i, std::string("FRAG_COL_" + std::to_string(i)).c_str()));
            }
        }

        GLCall(glLinkProgram(shaderP.m_ShaderProgram));
        GLCall(glDetachShader(shaderP.m_ShaderProgram, vid));
        GLCall(glDetachShader(shaderP.m_ShaderProgram, fid));
        GLCall(glDeleteShader(vid));
        GLCall(glDeleteShader(fid));
        // Check the program
        GLCall(glGetProgramiv(shaderP.m_ShaderProgram, GL_LINK_STATUS, &res));
        GLCall(glGetProgramiv(shaderP.m_ShaderProgram, GL_INFO_LOG_LENGTH, &ll));
        std::vector<char> pe(std::max(ll, (int)1));
        GLCall(glGetProgramInfoLog(shaderP.m_ShaderProgram, ll, NULL, &pe[0]));
        if (res == GL_FALSE) {
            ENGINE_PRODUCTION_LOG("ShaderProgram Log : ")
            ENGINE_PRODUCTION_LOG(&pe[0])
        }
        //populate uniform table
        if (res == GL_TRUE) {
            GLint           i_;
            GLint           count_;
            GLint           size_;
            GLenum          type_;
            const GLsizei   bufSize_ = 256;    // maximum name length
            GLchar          name_[bufSize_];   // variable name in GLSL
            GLsizei         length_;           // name length
            GLCall(glGetProgramiv(shaderP.m_ShaderProgram, GL_ACTIVE_UNIFORMS, &count_));
            for (i_ = 0; i_ < count_; ++i_) {
                GLCall(glGetActiveUniform(shaderP.m_ShaderProgram, (GLuint)i_, bufSize_, &length_, &size_, &type_, name_));
                if (length_ > 0) {
                    std::string name1_((char*)name_, length_);
                    GLCall(GLint loc_ = glGetUniformLocation(shaderP.m_ShaderProgram, name_));
                    shaderP.m_UniformLocations.emplace(
                        std::piecewise_construct,
                        std::forward_as_tuple(std::move(name1_)), 
                        std::forward_as_tuple(loc_)
                    );
                }
            }
        }
        //link UBO's
        if (ShaderHelper::sfind(VertexCode, "layout (std140) uniform Camera //generated") || ShaderHelper::sfind(FragmentCode, "layout (std140) uniform Camera //generated")) {
            UniformBufferObject::UBO_CAMERA->attachToShader(shaderP);
        }
        shaderP.m_LoadedGPU = true;
    }
    shaderP.Resource::load();
}
void PublicShaderProgram::UnloadCPU(ShaderProgram& shaderP) {
    if (shaderP.m_LoadedCPU) {
        shaderP.m_LoadedCPU = false;
    }
    shaderP.Resource::unload();
}
void PublicShaderProgram::UnloadGPU(ShaderProgram& shaderP) {
    if (shaderP.m_LoadedGPU) {
        shaderP.m_UniformLocations.clear();
        shaderP.m_AttachedUBOs.clear();
        GLCall(glDeleteProgram(shaderP.m_ShaderProgram));
        shaderP.m_LoadedGPU = false;
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

