#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/shaders/ShaderHelper.h>
#include <core/engine/shaders/Shader.h>

#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/renderer/opengl/UniformBufferObject.h>

#include <iostream>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

ShaderProgram* ShaderProgram::Deferred = nullptr;
ShaderProgram* ShaderProgram::Forward  = nullptr;

namespace Engine{
    namespace epriv{
        struct DefaultShaderBindFunctor{void operator()(EngineResource* r) const {
            Scene* scene = Resources::getCurrentScene();  if(!scene) return;
            Camera* camera = scene->getActiveCamera();    if(!camera) return;
            Camera& c = *camera;

            float fcoeff = (2.0f / glm::log2(c.getFar() + 1.0f)) * 0.5f;
            Renderer::sendUniform1Safe("fcoeff",fcoeff);

            //yes this is needed
            if(RenderManager::GLSL_VERSION < 140){
                Renderer::sendUniformMatrix4Safe("CameraViewProj",c.getViewProjection());
            }
        }};
        struct DefaultShaderUnbindFunctor{void operator()(EngineResource* r) const {
        }};
    };
};

ShaderProgram::ShaderProgram(string _name, Shader& vs, Shader& fs):m_VertexShader(vs), m_FragmentShader(fs){
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

void ShaderProgram::_load_CPU() {
    _unload_CPU();
    if (!m_LoadedCPU) {
        m_LoadedCPU = true;
    }
}
void ShaderProgram::_unload_CPU() {
    if (m_LoadedCPU) {
        m_LoadedCPU = false;
    }
}
void ShaderProgram::_load_GPU() {
    _unload_GPU();
    if (!m_LoadedGPU) {
        string& VertexCode = m_VertexShader.m_Code; 
        string& FragmentCode = m_FragmentShader.m_Code;
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
            if (m_VertexShader.fromFile()) { 
                cout << "VertexShader Log (" + m_VertexShader.m_FileName + "): " << endl; 
            }else{ 
                cout << "VertexShader Log (" + m_VertexShader.name() + "): " << endl; 
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
            if (m_FragmentShader.fromFile()) { 
                cout << "FragmentShader Log (" + m_FragmentShader.m_FileName + "): " << endl; 
            }else{ 
                cout << "FragmentShader Log (" + m_FragmentShader.name() + "): " << endl; 
            }
            cout << &fe[0] << endl;
        }
        // Link the program id
        m_ShaderProgram = glCreateProgram();
        glAttachShader(m_ShaderProgram, vid);
        glAttachShader(m_ShaderProgram, fid);

        for (uint i = 0; i < 100; ++i) {
            string outFragCol = "out vec4 FRAG_COL_" + to_string(i) + ";";
            if (ShaderHelper::sfind(FragmentCode, outFragCol)) {
                glBindFragDataLocation(m_ShaderProgram, i, string("FRAG_COL_" + to_string(i)).c_str());
            }
        }

        glLinkProgram(m_ShaderProgram);
        glDetachShader(m_ShaderProgram, vid);
        glDetachShader(m_ShaderProgram, fid);
        glDeleteShader(vid);
        glDeleteShader(fid);
        // Check the program
        glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &res);
        glGetProgramiv(m_ShaderProgram, GL_INFO_LOG_LENGTH, &ll);
        vector<char>pe(std::max(ll, static_cast<int>(1)));
        glGetProgramInfoLog(m_ShaderProgram, ll, NULL, &pe[0]);
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
            glGetProgramiv(m_ShaderProgram, GL_ACTIVE_UNIFORMS, &_count);
            for (_i = 0; _i < _count; ++_i) {
                glGetActiveUniform(m_ShaderProgram, static_cast<GLuint>(_i), _bufSize, &_length, &_size, &_type, _name);
                if (_length > 0) {
                    string _name1(static_cast<char*>(_name), _length);
                    GLint _loc = glGetUniformLocation(m_ShaderProgram, _name);
                    m_UniformLocations.emplace(_name1, _loc);
                }
            }
        }
        //link UBO's
        if (ShaderHelper::sfind(VertexCode, "layout (std140) uniform Camera //generated") || ShaderHelper::sfind(FragmentCode, "layout (std140) uniform Camera //generated")) {
            UniformBufferObject::UBO_CAMERA->attachToShader(*this);
        }
        m_LoadedGPU = true;
    }
}
void ShaderProgram::_unload_GPU() {
    if (m_LoadedGPU) {
        m_UniformLocations.clear();
        m_AttachedUBOs.clear();
        glDeleteProgram(m_ShaderProgram);
        m_LoadedGPU = false;
    }
}
const GLuint& ShaderProgram::program() const {
    return m_ShaderProgram; 
}

void epriv::InternalShaderProgramPublicInterface::LoadCPU(ShaderProgram& shaderP){
    shaderP._load_CPU();
}
void epriv::InternalShaderProgramPublicInterface::LoadGPU(ShaderProgram& shaderP){
    shaderP._load_GPU();
    shaderP.EngineResource::load();
}
void epriv::InternalShaderProgramPublicInterface::UnloadCPU(ShaderProgram& shaderP){
    shaderP._unload_CPU();
    shaderP.EngineResource::unload();
}
void epriv::InternalShaderProgramPublicInterface::UnloadGPU(ShaderProgram& shaderP){
    shaderP._unload_GPU();        
}
void ShaderProgram::load(){
    if(!isLoaded()){
        _load_CPU();
        _load_GPU();
        cout << "(Shader Program) ";
        EngineResource::load();
    }
}
void ShaderProgram::unload(){
    if(isLoaded() /*&& useCount() == 0*/){
        _unload_GPU();
        _unload_CPU();
        cout << "(Shader Program) ";
        EngineResource::unload();
    }
}
void ShaderProgram::bind(){ 
    epriv::Core::m_Engine->m_RenderManager._bindShaderProgram(this); 
}
void ShaderProgram::unbind(){ 
    epriv::Core::m_Engine->m_RenderManager._unbindShaderProgram(); 
}
const unordered_map<string,GLint>& ShaderProgram::uniforms() const { 
    return m_UniformLocations; 
}
