#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/shaders/ShaderHelper.h>
#include <core/engine/shaders/Shader.h>

#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/system/Engine.h>
#include <core/engine/renderer/opengl/UniformBufferObject.h>

using namespace Engine;
using namespace Engine::priv;

ShaderProgram* ShaderProgram::Deferred = nullptr;
ShaderProgram* ShaderProgram::Forward  = nullptr;
ShaderProgram* ShaderProgram::Decal    = nullptr;

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
        if(priv::Renderer::GLSL_VERSION < 140){
            Engine::Renderer::sendUniformMatrix4Safe("CameraViewProj", camera->getViewProjection());
        }
    }};
};

ShaderProgram::ShaderProgram(const std::string& in_name, Shader& vs, Shader& fs)
    : Resource{ ResourceType::ShaderProgram, in_name }
    , m_VertexShader{ vs }
    , m_FragmentShader{ fs }
{
    setName(in_name);
    const std::string& name_ = name();
    if (vs.name() == "NULL") {
        vs.setName(name_ + ".vert");
    }
    if (fs.name() == "NULL") {
        fs.setName(name_ + ".frag");
    }
    setCustomBindFunctor(DefaultShaderBindFunctor());
    load();
}
ShaderProgram::~ShaderProgram(){ 
    unload(); 
}

void InternalShaderProgramPublicInterface::LoadCPU(ShaderProgram& shaderP){
    InternalShaderProgramPublicInterface::UnloadCPU(shaderP);
    if (!shaderP.m_LoadedCPU) {
        shaderP.m_LoadedCPU = true;
    }
}
void InternalShaderProgramPublicInterface::LoadGPU(ShaderProgram& shaderP){
    InternalShaderProgramPublicInterface::UnloadGPU(shaderP);
    if (!shaderP.m_LoadedGPU) {
        const std::string& VertexCode   = shaderP.m_VertexShader.m_Code;
        const std::string& FragmentCode = shaderP.m_FragmentShader.m_Code;
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
            if (shaderP.m_VertexShader.fromFile()) {
                ENGINE_PRODUCTION_LOG("VertexShader Log (" + shaderP.m_VertexShader.m_FileName + "): ")
            }else{
                ENGINE_PRODUCTION_LOG("VertexShader Log (" + shaderP.m_VertexShader.name() + "): ")
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
            if (shaderP.m_FragmentShader.fromFile()) {
                ENGINE_PRODUCTION_LOG("FragmentShader Log (" + shaderP.m_FragmentShader.m_FileName + "): ")
            }else{
                ENGINE_PRODUCTION_LOG("FragmentShader Log (" + shaderP.m_FragmentShader.name() + "): ")
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
                        std::forward_as_tuple(name1_), 
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
void InternalShaderProgramPublicInterface::UnloadCPU(ShaderProgram& shaderP) {
    if (shaderP.m_LoadedCPU) {
        shaderP.m_LoadedCPU = false;
    }
    shaderP.Resource::unload();
}
void InternalShaderProgramPublicInterface::UnloadGPU(ShaderProgram& shaderP) {
    if (shaderP.m_LoadedGPU) {
        shaderP.m_UniformLocations.clear();
        shaderP.m_AttachedUBOs.clear();
        GLCall(glDeleteProgram(shaderP.m_ShaderProgram));
        shaderP.m_LoadedGPU = false;
    }
}
void ShaderProgram::load() {
    if(!isLoaded()){
        InternalShaderProgramPublicInterface::LoadCPU(*this);
        InternalShaderProgramPublicInterface::LoadGPU(*this);

        Resource::load();
    }
}
void ShaderProgram::unload() {
    if(isLoaded() /*&& useCount() == 0*/){
        InternalShaderProgramPublicInterface::UnloadGPU(*this);
        InternalShaderProgramPublicInterface::UnloadCPU(*this);

        Resource::unload();
    }
}

