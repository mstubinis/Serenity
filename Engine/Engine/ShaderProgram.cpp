#include "ShaderProgram.h"
#include "Material.h"
#include "Camera.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include <boost/filesystem.hpp>
#include <regex>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/stream.hpp>
#include <iostream>
#include <SFML/OpenGL.hpp>

using namespace Engine;
using namespace std;

class Shader::impl final{
    public:
        ShaderType::Type m_Type;
        bool m_FromFile;
        string m_Data;
        void _construct(string& name, string& data, ShaderType::Type type, bool fromFile,Shader* super){
            m_Data = data;
            m_Type = type;
            m_FromFile = fromFile;
            super->setName(name);
            Resources::Detail::ResourceManagement::_addToContainer(Resources::Detail::ResourceManagement::m_Shaders,name,boost::shared_ptr<Shader>(super));
        }
};
Shader::Shader(string name, string shaderFileOrData, ShaderType::Type shaderType,bool fromFile):m_i(new impl){
    m_i->_construct(name,shaderFileOrData,shaderType,fromFile,this);
}
Shader::~Shader(){
}
ShaderType::Type Shader::type(){ return m_i->m_Type; }
string Shader::data(){ return m_i->m_Data; }
bool Shader::fromFile(){ return m_i->m_FromFile; }

struct DefaultShaderBindFunctor{void operator()(EngineResource* r) const {
    Camera* c = Resources::getActiveCamera();
    if(c == nullptr) return;
    Renderer::sendUniformMatrix4fSafe("VP",c->getViewProjection());
    Renderer::sendUniform1fSafe("fcoeff",2.0f / glm::log2(c->getFar() + 1.0f));

    glm::vec3 camPos = c->getPosition();
    Renderer::sendUniform3fSafe("CameraPosition",camPos);

    if(Renderer::Detail::RendererInfo::GodRaysInfo::godRays) Renderer::sendUniform1iSafe("HasGodsRays",1);
    else                                                     Renderer::sendUniform1iSafe("HasGodsRays",0);
}};
struct DefaultShaderUnbindFunctor{void operator()(EngineResource* r) const {
}};
struct srtKey{inline bool operator() ( Material* _1,  Material* _2){return (_1->name() < _2->name());}};

class ShaderP::impl final{
    public:
        static DefaultShaderBindFunctor DEFAULT_BIND_FUNCTOR;
        static DefaultShaderUnbindFunctor DEFAULT_UNBIND_FUNCTOR;

        ShaderRenderPass::Pass m_Stage;
        GLuint m_ShaderProgram;
        vector<Material*> m_Materials;
        unordered_map<string,GLint> m_UniformLocations;
        Shader* m_VertexShader;
        Shader* m_FragmentShader;

		void _insertStringAtLine(std::string& source, const std::string& newLineContent,uint lineToInsertAt){
			std::istringstream str(source); std::string line; 
			std::vector<std::string> lines;
			uint count = 0;
			while(std::getline(str,line)){
				lines.push_back(line + "\n");
				if(count == lineToInsertAt){
					lines.push_back(newLineContent + "\n");
				}
				count++;
			}
			source = "";
			for(auto line:lines){
				source = source + line;
			}
		}
        void _convertCode(std::string& _data,Shader* shader){
			std::istringstream str(_data); std::string line; 
			
			//get the first line with actual content
			while(true){
			    std::getline(str,line);
				if(line != "" && line != "\n"){
				    break;
				}
			}
			std::string versionNumberString = std::regex_replace(line,std::regex("([^0-9])"),"");
			uint versionNumber = boost::lexical_cast<uint>(versionNumberString);
			if (line == "#version 110"){
			}
			else if (line == "#version 120"){
			}
			else if(line == "#version 130"){
			}
			else if(line == "#version 140"){
			}
			else if(line == "#version 150"){
			}
			else if(line == "#version 330 core"){
			}
			else if(line == "#version 400 core"){
			}
			else if(line == "#version 410 core"){
			}
			else if(line == "#version 420 core"){
			}
			else if(line == "#version 430 core"){
			}
			else if(line == "#version 440 core"){
			}
			else if(line == "#version 450 core"){
			}
			if(versionNumber >= 130){
				if(shader->type() == ShaderType::Vertex){
					boost::replace_all(_data, "varying", "out");
				}
				else if(shader->type() == ShaderType::Fragment){
					boost::replace_all(_data, "varying", "in");

					boost::replace_all(_data, "gl_FragColor", "FRAGMENT_OUTPUT_COLOR");
					_insertStringAtLine(_data,"out vec4 FRAGMENT_OUTPUT_COLOR;",1);
				}
			}
			if(versionNumber >= 140){
				if(shader->type() == ShaderType::Vertex){
				}
				else if(shader->type() == ShaderType::Fragment){
					boost::replace_all(_data, "textureCube(", "texture(");
					boost::replace_all(_data, "textureCubeLod(", "textureLod(");
					boost::replace_all(_data, "texture2DLod(", "textureLod(");
					boost::replace_all(_data, "texture2D(", "texture(");
				}
			}
		}
        void _construct(string& name, Shader* vs, Shader* fs, ShaderRenderPass::Pass stage,ShaderP* super){
            m_Stage = stage;
            m_VertexShader = vs;
            m_FragmentShader = fs;
            m_UniformLocations.clear();

            if(stage == ShaderRenderPass::Geometry){
                Renderer::Detail::RenderManagement::m_GeometryPassShaderPrograms.push_back(super);
            }
            else if(stage == ShaderRenderPass::Forward){
                Renderer::Detail::RenderManagement::m_ForwardPassShaderPrograms.push_back(super);
            }
            else if(stage == ShaderRenderPass::Lighting){
            }
            else if(stage == ShaderRenderPass::Postprocess){
            }
            else{
            }
            super->setName(name);

            super->setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
            super->setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
            m_ShaderProgram = _compileOGL(m_VertexShader,m_FragmentShader,name);
        }
        void _construct(string& name, string& vs, string& fs, ShaderRenderPass::Pass stage,ShaderP* super){
            Shader* _vs = Resources::getShader(vs); Shader* _fs = Resources::getShader(fs);
            if(_vs == nullptr){
                new Shader(vs,vs,ShaderType::Vertex,true);
                _vs = Resources::getShader(vs);
            }
            if(_fs == nullptr){
                new Shader(fs,fs,ShaderType::Fragment,true);
                _fs = Resources::getShader(fs);
            }
            _construct(name,_vs,_fs,stage,super);
        }
        void _construct(string& name, Shader* vs, string& fs, ShaderRenderPass::Pass stage,ShaderP* super){
            Shader* _fs = Resources::getShader(fs);
            if(_fs == nullptr){
                new Shader(fs,fs,ShaderType::Fragment,true);
                _fs = Resources::getShader(fs);
            }
            _construct(name,vs,_fs,stage,super);
        }
        void _construct(string& name, string& vs, Shader* fs, ShaderRenderPass::Pass stage,ShaderP* super){
            Shader* _vs = Resources::getShader(vs);
            if(_vs == nullptr){
                new Shader(vs,vs,ShaderType::Vertex,true);
                _vs = Resources::getShader(vs);
            }
            _construct(name,_vs,fs,stage,super);
        }
        void _destruct(){
            glDeleteShader(m_ShaderProgram);
            glDeleteProgram(m_ShaderProgram);
            m_UniformLocations.clear();
        }
        GLuint _compileOGL(Shader* vs,Shader*  ps,string& _shaderProgramName){
            m_UniformLocations.clear();
            GLuint vid = glCreateShader(GL_VERTEX_SHADER); GLuint fid = glCreateShader(GL_FRAGMENT_SHADER);
            string VertexShaderCode = ""; string FragmentShaderCode = "";
            if(vs->fromFile()){
                boost::iostreams::stream<boost::iostreams::mapped_file_source> str(vs->data());
                for(string line; getline(str, line, '\n');){ VertexShaderCode += "\n" + line; }
            }
            else{ VertexShaderCode = vs->data(); }
            if(ps->fromFile()){
                boost::iostreams::stream<boost::iostreams::mapped_file_source> str1(ps->data());
                for(string line; getline(str1, line, '\n');){ FragmentShaderCode += "\n" + line; }
            }
            else{ FragmentShaderCode = ps->data(); }

			_convertCode(VertexShaderCode,vs);
			_convertCode(FragmentShaderCode,ps);

            GLint res = GL_FALSE;
            int logLength;

            // Compile Vertex Shader
            char const * vSource = VertexShaderCode.c_str();
            glShaderSource(vid, 1, &vSource , NULL);
            glCompileShader(vid);

            // Check Vertex Shader
            glGetShaderiv(vid, GL_COMPILE_STATUS, &res);
            glGetShaderiv(vid, GL_INFO_LOG_LENGTH, &logLength);
            vector<char> vError(logLength);
            glGetShaderInfoLog(vid, logLength, NULL, &vError[0]);

            if(res == GL_FALSE) {
                if(vs->fromFile()){ cout << "VertexShader Log (" + vs->data() + "): " << endl; }
                else{               cout << "VertexShader Log (" + vs->name() + "): " << endl; }
                cout << &vError[0] << endl;
            }

            // Compile Fragment Shader
            char const* fSource = FragmentShaderCode.c_str();
            glShaderSource(fid, 1, &fSource , NULL);
            glCompileShader(fid);

            // Check Fragment Shader
            glGetShaderiv(fid, GL_COMPILE_STATUS, &res);
            glGetShaderiv(fid, GL_INFO_LOG_LENGTH, &logLength);
            vector<char> fError(logLength);
            glGetShaderInfoLog(fid, logLength, NULL, &fError[0]);

            if(res == GL_FALSE) {
                if(ps->fromFile()){ cout << "FragmentShader Log (" + ps->data() + "): " << endl; }
                else{               cout << "FragmentShader Log (" + ps->name() + "): " << endl; }
                cout << &fError[0] << endl;
            }

            // Link the program id
            GLuint pid = glCreateProgram();
            glAttachShader(pid, vid); glAttachShader(pid, fid);
            glLinkProgram(pid);

            // Check the program
            glGetProgramiv(pid, GL_LINK_STATUS, &res);
            glGetProgramiv(pid, GL_INFO_LOG_LENGTH, &logLength);
            vector<char> pError( std::max(logLength, int(1)) );
            glGetProgramInfoLog(pid, logLength, NULL, &pError[0]);

            if(res == GL_FALSE) {
                cout << "ShaderProgram Log : " << endl; cout << &pError[0] << endl;
            }
            glDetachShader(pid,vid); glDetachShader(pid,fid);
            glDeleteShader(vid); glDeleteShader(fid);

            //populate uniform table
            if(res == GL_TRUE) {
                GLint _i; GLint _count; GLint _size;
                GLenum _type; // type of the variable (float, vec3 or mat4, etc)

                const GLsizei _bufSize = 256; // maximum name length
                GLchar _name[_bufSize]; // variable name in GLSL
                GLsizei _length; // name length
                glGetProgramiv(pid,GL_ACTIVE_UNIFORMS,&_count);
                for(_i = 0; _i < _count; ++_i){
                    glGetActiveUniform(pid, (GLuint)_i, _bufSize, &_length, &_size, &_type, _name);
                    if(_length > 0){
                        string _name1((char*)_name, _length);
                        GLint _uniformLoc = glGetUniformLocation(pid,_name);
                        this->m_UniformLocations.emplace(_name1,_uniformLoc);
                    }
                }
            }
            return pid;
        }
};
DefaultShaderBindFunctor ShaderP::impl::DEFAULT_BIND_FUNCTOR;
DefaultShaderUnbindFunctor ShaderP::impl::DEFAULT_UNBIND_FUNCTOR;

ShaderP::ShaderP(string& n, string& vs, string& fs, ShaderRenderPass::Pass s):m_i(new impl){ m_i->_construct(n,vs,fs,s,this); }
ShaderP::ShaderP(string& n, Shader* vs, Shader* fs, ShaderRenderPass::Pass s):m_i(new impl){ m_i->_construct(n,vs,fs,s,this); }
ShaderP::ShaderP(string& n, Shader* vs, string& fs, ShaderRenderPass::Pass s):m_i(new impl){ m_i->_construct(n,vs,fs,s,this); }
ShaderP::ShaderP(string& n, string& vs, Shader* fs, ShaderRenderPass::Pass s):m_i(new impl){ m_i->_construct(n,vs,fs,s,this); }
ShaderP::~ShaderP(){
    m_i->_destruct();
}
GLuint ShaderP::program(){ return m_i->m_ShaderProgram; }
ShaderRenderPass::Pass ShaderP::stage(){ return m_i->m_Stage; }
vector<Material*>& ShaderP::getMaterials(){ return m_i->m_Materials; }

void ShaderP::bind(){
    if(Engine::Renderer::Detail::RendererInfo::GeneralInfo::current_shader_program != this){
        glUseProgram(m_i->m_ShaderProgram);
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
void ShaderP::addMaterial(const string& materialName){
    if(materialName == "" || !Resources::Detail::ResourceManagement::m_Materials.count(materialName)){
        cout << "Material : '" << materialName << "' does not exist (ShaderP::addMaterial()) Returning..." << endl;
        return;
    }
    Material* mat = Resources::getMaterial(materialName);
    m_i->m_Materials.push_back(mat);
    sort(m_i->m_Materials.begin(),m_i->m_Materials.end(),srtKey());
}
const unordered_map<string,GLint>& ShaderP::uniforms() const { return this->m_i->m_UniformLocations; }
