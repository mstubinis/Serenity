#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/shaders/ShaderHelper.h>
#include <core/engine/shaders/Shader.h>

#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/system/Engine.h>
#include <core/engine/renderer/opengl/UniformBufferObject.h>

#include <iostream>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

ShaderProgram* ShaderProgram::Deferred = nullptr;
ShaderProgram* ShaderProgram::Forward  = nullptr;
ShaderProgram* ShaderProgram::Decal    = nullptr;

namespace Engine{
    namespace priv{
        struct DefaultShaderBindFunctor{void operator()(EngineResource* r) const {
            Scene* scene = Resources::getCurrentScene();  if(!scene) return;
            Camera* camera = scene->getActiveCamera();    if(!camera) return;
            Camera& c = *camera;

            const float fcoeff = (2.0f / glm::log2(c.getFar() + 1.0f)) * 0.5f;
            Engine::Renderer::sendUniform1Safe("fcoeff",fcoeff);

            //yes this is needed
            if(priv::Renderer::GLSL_VERSION < 140){
                Engine::Renderer::sendUniformMatrix4Safe("CameraViewProj",c.getViewProjection());
            }
        }};
        struct DefaultShaderUnbindFunctor{void operator()(EngineResource* r) const {
        }};
    };
};

ShaderProgram::ShaderProgram(const string& _name, Shader& vs, Shader& fs):m_VertexShader(vs), m_FragmentShader(fs), BindableResource(ResourceType::ShaderProgram, _name){
    m_LoadedGPU = m_LoadedCPU = false;

    setCustomBindFunctor(DefaultShaderBindFunctor());
    setCustomUnbindFunctor(DefaultShaderUnbindFunctor());
    setName(_name);

    const string& name_ = name();
    if (vs.name() == "NULL") vs.setName(name_ + ".vert");
    if (fs.name() == "NULL") fs.setName(name_ + ".frag");
    load();
}
ShaderProgram::~ShaderProgram(){ 
    unload(); 
}

const GLuint& ShaderProgram::program() const {
    return m_ShaderProgram; 
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
        string& VertexCode = shaderP.m_VertexShader.m_Code;
        string& FragmentCode = shaderP.m_FragmentShader.m_Code;
        GLuint vid = glCreateShader(GL_VERTEX_SHADER);
        GLuint fid = glCreateShader(GL_FRAGMENT_SHADER);
        GLint res = GL_FALSE;
        int ll;
        // Compile Vertex Shader
        char const* vss = VertexCode.c_str();
        glShaderSource(vid, 1, &vss, NULL);
        glCompileShader(vid);
        // Check Vertex Shader
        glGetShaderiv(vid, GL_COMPILE_STATUS, &res);
        glGetShaderiv(vid, GL_INFO_LOG_LENGTH, &ll);
        vector<char>ve(ll);
        if (ve.size() > 0) {
            glGetShaderInfoLog(vid, ll, NULL, &ve[0]);
        }
        if (res == GL_FALSE) {
            if (shaderP.m_VertexShader.fromFile()) {
                cout << "VertexShader Log (" + shaderP.m_VertexShader.m_FileName + "): " << endl;
            }else{
                cout << "VertexShader Log (" + shaderP.m_VertexShader.name() + "): " << endl;
            }
            cout << &ve[0] << endl;
        }
        // Compile Fragment Shader
        char const* fss = FragmentCode.c_str();
        glShaderSource(fid, 1, &fss, NULL);
        glCompileShader(fid);
        // Check Fragment Shader
        glGetShaderiv(fid, GL_COMPILE_STATUS, &res);
        glGetShaderiv(fid, GL_INFO_LOG_LENGTH, &ll);
        vector<char>fe(ll);
        if (fe.size() > 0) {
            glGetShaderInfoLog(fid, ll, NULL, &fe[0]);
        }
        if (res == GL_FALSE) {
            if (shaderP.m_FragmentShader.fromFile()) {
                cout << "FragmentShader Log (" + shaderP.m_FragmentShader.m_FileName + "): " << endl;
            }else{
                cout << "FragmentShader Log (" + shaderP.m_FragmentShader.name() + "): " << endl;
            }
            cout << &fe[0] << endl;
        }
        // Link the program id
        shaderP.m_ShaderProgram = glCreateProgram();
        glAttachShader(shaderP.m_ShaderProgram, vid);
        glAttachShader(shaderP.m_ShaderProgram, fid);

        for (unsigned int i = 0; i < 100; ++i) {
            string outFragCol = "out vec4 FRAG_COL_" + to_string(i) + ";";
            if (ShaderHelper::sfind(FragmentCode, outFragCol)) {
                glBindFragDataLocation(shaderP.m_ShaderProgram, i, string("FRAG_COL_" + to_string(i)).c_str());
            }
        }

        glLinkProgram(shaderP.m_ShaderProgram);
        glDetachShader(shaderP.m_ShaderProgram, vid);
        glDetachShader(shaderP.m_ShaderProgram, fid);
        glDeleteShader(vid);
        glDeleteShader(fid);
        // Check the program
        glGetProgramiv(shaderP.m_ShaderProgram, GL_LINK_STATUS, &res);
        glGetProgramiv(shaderP.m_ShaderProgram, GL_INFO_LOG_LENGTH, &ll);
        vector<char>pe(std::max(ll, static_cast<int>(1)));
        glGetProgramInfoLog(shaderP.m_ShaderProgram, ll, NULL, &pe[0]);
        if (res == GL_FALSE) {
            cout << "ShaderProgram Log : " << endl; cout << &pe[0] << endl;
        }
        //populate uniform table
        if (res == GL_TRUE) {
            GLint _i, _count, _size;
            GLenum _type;
            const GLsizei _bufSize = 256; // maximum name length
            GLchar _name[_bufSize]; // variable name in GLSL
            GLsizei _length; // name length
            glGetProgramiv(shaderP.m_ShaderProgram, GL_ACTIVE_UNIFORMS, &_count);
            for (_i = 0; _i < _count; ++_i) {
                glGetActiveUniform(shaderP.m_ShaderProgram, static_cast<GLuint>(_i), _bufSize, &_length, &_size, &_type, _name);
                if (_length > 0) {
                    string _name1(static_cast<char*>(_name), _length);
                    GLint _loc = glGetUniformLocation(shaderP.m_ShaderProgram, _name);
                    shaderP.m_UniformLocations.emplace(_name1, _loc);
                }
            }
        }
        //link UBO's
        if (ShaderHelper::sfind(VertexCode, "layout (std140) uniform Camera //generated") || ShaderHelper::sfind(FragmentCode, "layout (std140) uniform Camera //generated")) {
            UniformBufferObject::UBO_CAMERA->attachToShader(shaderP);
        }
        shaderP.m_LoadedGPU = true;
    }
    shaderP.EngineResource::load();
}
void InternalShaderProgramPublicInterface::UnloadCPU(ShaderProgram& shaderP){
    if (shaderP.m_LoadedCPU) {
        shaderP.m_LoadedCPU = false;
    }
    shaderP.EngineResource::unload();
}
void InternalShaderProgramPublicInterface::UnloadGPU(ShaderProgram& shaderP){
    if (shaderP.m_LoadedGPU) {
        shaderP.m_UniformLocations.clear();
        shaderP.m_AttachedUBOs.clear();
        glDeleteProgram(shaderP.m_ShaderProgram);
        shaderP.m_LoadedGPU = false;
    }
}
void ShaderProgram::load(){
    if(!isLoaded()){
        auto& _this = *this;
        InternalShaderProgramPublicInterface::LoadCPU(_this);
        InternalShaderProgramPublicInterface::LoadGPU(_this);
        cout << "(Shader Program) ";
        EngineResource::load();
    }
}
void ShaderProgram::unload(){
    if(isLoaded() /*&& useCount() == 0*/){
        auto& _this = *this;
        InternalShaderProgramPublicInterface::UnloadGPU(_this);
        InternalShaderProgramPublicInterface::UnloadCPU(_this);
        cout << "(Shader Program) ";
        EngineResource::unload();
    }
}
void ShaderProgram::bind() {
    Core::m_Engine->m_RenderManager._bindShaderProgram(this); 
}
void ShaderProgram::unbind(){ 
    Core::m_Engine->m_RenderManager._unbindShaderProgram(); 
}
const unordered_map<string,GLint>& ShaderProgram::uniforms() const { 
    return m_UniformLocations; 
}
