#include "ShaderProgram.h"
#include "Material.h"
#include "Camera.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <iostream>

using namespace Engine;

#pragma region individualShaderFiles

class Shader::impl final{
    public:
        SHADER_TYPE m_Type;
        bool m_FromFile;
        std::string m_Data;

        void _construct(std::string& name, std::string& data, SHADER_TYPE type, bool fromFile,Shader* super){
            m_Data = data;
            m_Type = type;
            m_FromFile = fromFile;
            super->setName(name);
        }
};
Shader::Shader(std::string& name, std::string& shaderFileOrData, SHADER_TYPE shaderType,bool fromFile):m_i(new impl){
    m_i->_construct(name,shaderFileOrData,shaderType,fromFile,this);
}
Shader::~Shader(){
}
SHADER_TYPE Shader::type(){ return m_i->m_Type; }
std::string Shader::data(){ return m_i->m_Data; }
bool Shader::fromFile(){ return m_i->m_FromFile; }

#pragma endregion

struct DefaultShaderBindFunctor{void operator()(EngineResource* r) const {
    Camera* c = Resources::getActiveCamera();
    Renderer::sendUniformMatrix4fSafe("VP",c->getViewProjection());
	Renderer::sendUniform1fSafe("fcoeff",2.0f / glm::log2(c->getFar() + 1.0f));

    glm::vec3 camPos = glm::vec3(c->getPosition());
    Renderer::sendUniform3fSafe("CameraPosition",camPos);

    if(Renderer::Detail::RendererInfo::GodRaysInfo::godRays) Renderer::sendUniform1iSafe("HasGodsRays",1);
    else                                                     Renderer::sendUniform1iSafe("HasGodsRays",0);
}};
struct DefaultShaderUnbindFunctor{void operator()(EngineResource* r) const {
}};

class ShaderP::impl final{
    public:
        static DefaultShaderBindFunctor DEFAULT_BIND_FUNCTOR;
        static DefaultShaderUnbindFunctor DEFAULT_UNBIND_FUNCTOR;

        SHADER_PIPELINE_STAGE m_Stage;
        GLuint m_ShaderProgram;
        std::vector<Material*> m_Materials;
        std::unordered_map<std::string,GLint> m_UniformLocations;
        Shader* m_VertexShader;
        Shader* m_FragmentShader;
        void _construct(std::string& name, Shader* vs, Shader* ps, SHADER_PIPELINE_STAGE stage,ShaderP* super){
            m_Stage = stage;
            m_VertexShader = vs;
            m_FragmentShader = ps;
            m_UniformLocations.clear();

            if(stage == SHADER_PIPELINE_STAGE_GEOMETRY){
                Renderer::Detail::RenderManagement::m_GeometryPassShaderPrograms.push_back(super);
            }
            else if(stage == SHADER_PIPELINE_STAGE_LIGHTING){
            }
            else if(stage == SHADER_PIPELINE_STAGE_POSTPROCESSING){
            }
            else{
            }
            super->setName(name);

            super->setCustomBindFunctor(ShaderP::impl::DEFAULT_BIND_FUNCTOR);
            super->setCustomUnbindFunctor(ShaderP::impl::DEFAULT_UNBIND_FUNCTOR);
        }
        void _construct(std::string& name, std::string& vs, std::string& ps, SHADER_PIPELINE_STAGE stage,ShaderP* super){
            Shader* v = Resources::getShader(vs); Shader* f = Resources::getShader(ps);
            if(v == nullptr){
                Resources::addShader(vs,vs,SHADER_TYPE_VERTEX,true);
                v = Resources::getShader(vs);
            }
            if(f == nullptr){
                Resources::addShader(ps,ps,SHADER_TYPE_FRAGMENT,true);
                f = Resources::getShader(ps);
            }
            _construct(name,v,f,stage,super);
        }

        void _destruct(){
            _cleanupRenderingContext();
        }
        void _initRenderingContext(std::string& name){
            m_ShaderProgram = _compileOGL(m_VertexShader,m_FragmentShader,name);
        }
        void _cleanupRenderingContext(){
            glDeleteShader(m_ShaderProgram);
            glDeleteProgram(m_ShaderProgram);
            m_UniformLocations.clear();
        }
        GLuint _compileOGL(Shader* vs,Shader*  ps,std::string& _shaderProgramName){
            m_UniformLocations.clear();
            // Create the shaders id's
            GLuint vid = glCreateShader(GL_VERTEX_SHADER);
            GLuint fid = glCreateShader(GL_FRAGMENT_SHADER);
 
            std::string VertexShaderCode = ""; std::string FragmentShaderCode = "";

            if(vs->fromFile()){
                boost::iostreams::stream<boost::iostreams::mapped_file_source> str(vs->data());
                for(std::string line; std::getline(str, line, '\n');){ VertexShaderCode += "\n" + line; }
            }
            else{
                VertexShaderCode = vs->data();
            }
            if(ps->fromFile()){
                boost::iostreams::stream<boost::iostreams::mapped_file_source> str1(ps->data());
                for(std::string line; std::getline(str1, line, '\n');){ FragmentShaderCode += "\n" + line; }
            }
            else{
                FragmentShaderCode = ps->data();
            }
 
            GLint res = GL_FALSE;
            int logLength;
 
            // Compile Vertex Shader
            char const * vSource = VertexShaderCode.c_str();
            glShaderSource(vid, 1, &vSource , NULL);
            glCompileShader(vid);
 
            // Check Vertex Shader
            glGetShaderiv(vid, GL_COMPILE_STATUS, &res);
            glGetShaderiv(vid, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> vError(logLength);
            glGetShaderInfoLog(vid, logLength, NULL, &vError[0]);

            if(res == GL_FALSE) {
                if(vs->fromFile()){ std::cout << "VertexShader Log (" + vs->data() + "): " << std::endl; }
                else{         std::cout << "VertexShader Log : " << std::endl; }
                std::cout << &vError[0] << std::endl;
            }
 
            // Compile Fragment Shader
            char const* fSource = FragmentShaderCode.c_str();
            glShaderSource(fid, 1, &fSource , NULL);
            glCompileShader(fid);
 
            // Check Fragment Shader
            glGetShaderiv(fid, GL_COMPILE_STATUS, &res);
            glGetShaderiv(fid, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> fError(logLength);
            glGetShaderInfoLog(fid, logLength, NULL, &fError[0]);
 
            if(res == GL_FALSE) {
                if(ps->fromFile()){ std::cout << "FragmentShader Log (" + ps->data() + "): " << std::endl; }
                else{         std::cout << "FragmentShader Log : " << std::endl; }
                std::cout << &fError[0] << std::endl;
            }

            // Link the program id
            GLuint pid = glCreateProgram();
            glAttachShader(pid, vid);
            glAttachShader(pid, fid);
            glLinkProgram(pid);
 
            // Check the program
            glGetProgramiv(pid, GL_LINK_STATUS, &res);
            glGetProgramiv(pid, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> pError( std::max(logLength, int(1)) );
            glGetProgramInfoLog(pid, logLength, NULL, &pError[0]);
 
            if(res == GL_FALSE) {
                std::cout << "ShaderProgram Log : " << std::endl;
                std::cout << &pError[0] << std::endl;
            }
            glDetachShader(pid,vid);
            glDetachShader(pid,fid);

            glDeleteShader(vid);
            glDeleteShader(fid);

            //populate uniform table
            if(res == GL_TRUE) {
                GLint _i;GLint _count;
                GLint _size; // size of the variable
                GLenum _type; // type of the variable (float, vec3 or mat4, etc)

                const GLsizei _bufSize = 256; // maximum name length
                GLchar _name[_bufSize]; // variable name in GLSL
                GLsizei _length; // name length
                glGetProgramiv(pid,GL_ACTIVE_UNIFORMS,&_count);
                for(_i = 0; _i < _count; ++_i){
                    glGetActiveUniform(pid, (GLuint)_i, _bufSize, &_length, &_size, &_type, _name);
                    if(_length > 0){
                        std::string _name1((char*)_name, _length);
                        GLint _uniformLoc = glGetUniformLocation(pid,_name);
                        this->m_UniformLocations.emplace(_name1,_uniformLoc);
                    }
                }
            }
            return pid;
        }
        void _compileDX(std::string vs, std::string ps, bool fromFile){
        
        }
};
DefaultShaderBindFunctor ShaderP::impl::DEFAULT_BIND_FUNCTOR;
DefaultShaderUnbindFunctor ShaderP::impl::DEFAULT_UNBIND_FUNCTOR;

ShaderP::ShaderP(std::string& n, std::string& vs, std::string& fs, SHADER_PIPELINE_STAGE s):m_i(new impl){
    m_i->_construct(n,vs,fs,s,this);
}
ShaderP::ShaderP(std::string& n, Shader* vs, Shader* fs, SHADER_PIPELINE_STAGE s):m_i(new impl){
    m_i->_construct(n,vs,fs,s,this);
}
ShaderP::~ShaderP(){
    m_i->_destruct();
}
void ShaderP::initRenderingContext(){
    m_i->_initRenderingContext(this->name());
}
void ShaderP::cleanupRenderingContext(){
    m_i->_cleanupRenderingContext();
}
GLuint ShaderP::program(){ return m_i->m_ShaderProgram; }
Shader* ShaderP::vertexShader(){ return m_i->m_VertexShader; }
Shader* ShaderP::fragmentShader(){ return m_i->m_FragmentShader; }
SHADER_PIPELINE_STAGE ShaderP::stage(){ return m_i->m_Stage; }
std::vector<Material*>& ShaderP::getMaterials(){ return m_i->m_Materials; }


struct less_than_key{
    inline bool operator() ( Material* struct1,  Material* struct2){return (struct1->name() < struct2->name());}
};
void ShaderP::bind(){
    if(Engine::Renderer::Detail::RendererInfo::GeneralInfo::current_shader_program != this){
        GLuint p = this->program();
        glUseProgram(p);
        Engine::Renderer::Detail::RendererInfo::GeneralInfo::current_shader_program = this;
    }
   	BindableResource::bind();
}
void ShaderP::unbind(){
	BindableResource::unbind();
    if(Engine::Renderer::Detail::RendererInfo::GeneralInfo::current_shader_program != nullptr){
        glUseProgram(0);
        Engine::Renderer::Detail::RendererInfo::GeneralInfo::current_shader_program = nullptr;
    }
}
void ShaderP::addMaterial(std::string materialName){
    if(materialName == "" || !Resources::Detail::ResourceManagement::m_Materials.count(materialName)){
        std::cout << "Material : '" << materialName << "' does not exist (ShaderP::addMaterial()) Returning..." << std::endl;
        return;
    }
    Material* mat = Resources::getMaterial(materialName);
    m_i->m_Materials.push_back(mat);
	std::sort(m_i->m_Materials.begin(),m_i->m_Materials.end(),less_than_key());
}
const std::unordered_map<std::string,GLint>& ShaderP::uniforms() const { return this->m_i->m_UniformLocations; }