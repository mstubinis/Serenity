#include "Engine.h"
#include "ShaderProgram.h"
#include "Material.h"
#include "Camera.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Scene.h"

#include <boost/filesystem.hpp>
#include <regex>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/stream.hpp>
#include <iostream>
#include <SFML/OpenGL.hpp>

using namespace Engine;
using namespace std;

bool sfind(string str1,string str2){ if(str1.find(str2) != string::npos) return true; return false; }

UniformBufferObject* UniformBufferObject::UBO_CAMERA = nullptr;

namespace Engine{
    namespace epriv{
        struct DefaultShaderBindFunctor{void operator()(EngineResource* r) const {
            Scene* s = Resources::getCurrentScene(); if(s == nullptr) return;
            Camera* c = s->getActiveCamera();        if(c == nullptr) return;
            Renderer::sendUniformMatrix4fSafe("VP",c->getViewProjection());
            float fcoeff = (2.0f / glm::log2(c->getFar() + 1.0f)) * 0.5f;
            Renderer::sendUniform1fSafe("fcoeff",fcoeff);

            glm::vec3 camPos = c->getPosition();
            Renderer::sendUniform3fSafe("CameraPosition",camPos);

            if(Renderer::Settings::GodRays::enabled()) Renderer::sendUniform1iSafe("HasGodsRays",1);
            else                                       Renderer::sendUniform1iSafe("HasGodsRays",0);
        }};
        struct DefaultShaderUnbindFunctor{void operator()(EngineResource* r) const {
        }};
        struct srtKey{inline bool operator() ( Material* _1,  Material* _2){return (_1->name() < _2->name());}};
    };
};
GLint UniformBufferObject::MAX_UBO_BINDINGS;
uint UniformBufferObject::CUSTOM_UBO_AUTOMATIC_COUNT = 0;

class Shader::impl final{
    public:
        ShaderType::Type m_Type;
        bool m_FromFile;
        string m_Data;
        void _init(string& data, ShaderType::Type type, bool fromFile,Shader* super){
            m_Data = data;
            m_Type = type;
            m_FromFile = fromFile;
            if(fromFile){
                super->setName(data);
            }
            else{
                super->setName("NULL");
            }
        }
		void _destruct(){
		}

};
Shader::Shader(string shaderFileOrData, ShaderType::Type shaderType,bool fromFile):m_i(new impl){ m_i->_init(shaderFileOrData,shaderType,fromFile,this); }
Shader::~Shader(){ m_i->_destruct(); }
ShaderType::Type Shader::type(){ return m_i->m_Type; }
string Shader::data(){ return m_i->m_Data; }
bool Shader::fromFile(){ return m_i->m_FromFile; }

epriv::DefaultShaderBindFunctor DEFAULT_BIND_FUNCTOR;
epriv::DefaultShaderUnbindFunctor DEFAULT_UNBIND_FUNCTOR;
class ShaderP::impl final{
    public:
        ShaderRenderPass::Pass m_Stage;
        GLuint m_ShaderProgram;
        vector<Material*> m_Materials;
        unordered_map<string,GLint> m_UniformLocations;
		unordered_map<GLuint,bool> m_AttachedUBOs;
        Shader* m_VertexShader;
        Shader* m_FragmentShader;

        void _insertStringAtLine(string& source, const string& newLineContent,uint lineToInsertAt){
            istringstream str(source); string line; 
            vector<string> lines;
            uint count = 0;
            while(std::getline(str,line)){
                lines.push_back(line + "\n");
                if(count == lineToInsertAt){
                    lines.push_back(newLineContent + "\n");
                }
                ++count;
            }
            source = "";
            for(auto line:lines){
                source = source + line;
            }
        }
        void _convertCode(string& _data1,Shader* shader1,string& _data2,Shader* shader2,ShaderP* super){ _convertCode(_data1,shader1,super); _convertCode(_data2,shader2,super); }
        void _convertCode(string& _d,Shader* shader,ShaderP* super){
            istringstream str(_d); string line; 
            
            //get the first line with actual content
            while(true){
                getline(str,line);
                if(line != "" && line != "\n"){
                    break;
                }
            }
			//see if we actually have a version line
			if(!sfind(line,"#version")){
				string core = "";
				if(epriv::RenderManager::GLSL_VERSION >= 330)
					core = " core";
				line = "#version " + boost::lexical_cast<string>(epriv::RenderManager::GLSL_VERSION) + core + "\n";
				_d = line +  _d;
			}


            string versionNumberString = regex_replace(line,regex("([^0-9])"),"");
            uint versionNumber = boost::lexical_cast<uint>(versionNumberString);
            if(versionNumber >= 130){
                if(shader->type() == ShaderType::Vertex){
                    boost::replace_all(_d, "varying", "out");
                }
                else if(shader->type() == ShaderType::Fragment){
                    boost::replace_all(_d, "varying", "in");

                    boost::replace_all(_d, "gl_FragColor", "FRAGMENT_OUTPUT_COLOR");
                    _insertStringAtLine(_d,"out vec4 FRAGMENT_OUTPUT_COLOR;",1);
                }
            }
            if(versionNumber >= 140){
                if(shader->type() == ShaderType::Vertex){
                }
                else if(shader->type() == ShaderType::Fragment){
                    boost::replace_all(_d, "textureCube(", "texture(");
                    boost::replace_all(_d, "textureCubeLod(", "textureLod(");
                    boost::replace_all(_d, "texture2DLod(", "textureLod(");
                    boost::replace_all(_d, "texture2D(", "texture(");
                }
            }	
            if(versionNumber >= 150){
                if(shader->type() == ShaderType::Vertex){
                }
                else if(shader->type() == ShaderType::Fragment){
                }
            }
            if(versionNumber >= 330){
                if(shader->type() == ShaderType::Vertex){
                }
                else if(shader->type() == ShaderType::Fragment){
                }
            }


			//see if we need a UBO for the camera
			if(sfind(_d,"CameraView") || sfind(_d,"CameraProj") || sfind(_d,"CameraViewProj") || sfind(_d,"CameraInvView") || sfind(_d,"CameraInvProj") || 
			sfind(_d,"CameraInvViewProj") || sfind(_d,"CameraPosition") || sfind(_d,"CameraNear") || sfind(_d,"CameraFar") || sfind(_d,"CameraInfo1") || sfind(_d,"CameraInfo2")){
				string uboCameraString;
				if(versionNumber >= 140){ //UBO
					 if(!sfind(_d,"layout (std140) uniform Camera {//generated")){
						 uboCameraString = "\n"
						 "layout (std140) uniform Camera {//generated\n"
				 		 "    mat4 CameraView;\n"
						 "    mat4 CameraProj;\n"
						 "    mat4 CameraViewProj;\n"
						 "    mat4 CameraInvView;\n"
						 "    mat4 CameraInvProj;\n"
						 "    mat4 CameraInvViewProj;\n"
						 "    vec4 CameraInfo1;\n"
						 "    vec4 CameraInfo2;\n"
						 "};\n"
						 "vec3 CameraPosition = CameraInfo1.xyz;\n"
					     "vec3 CameraViewVector = CameraInfo2.xyz;\n"
						 "float CameraNear = CameraInfo1.w;\n"
						 "float CameraFar = CameraInfo2.w;\n"
						 "\n";
						 _insertStringAtLine(_d,uboCameraString,1);
						 UniformBufferObject::UBO_CAMERA->attachToShader(super);
					 }
				}
				else{ //no UBO's, just add a uniform struct
					if(!sfind(_d,"uniform mat4 CameraView;//generated")){
						 uboCameraString = "\n"
				 		 "uniform mat4 CameraView;//generated;\n"
						 "uniform mat4 CameraProj;\n"
						 "uniform mat4 CameraViewProj;\n"
						 "uniform mat4 CameraInvView;\n"
						 "uniform mat4 CameraInvProj;\n"
						 "uniform mat4 CameraInvViewProj;\n"
						 "uniform vec4 CameraInfo1;\n"
						 "uniform vec4 CameraInfo2;\n"
						 "vec3 CameraPosition = CameraInfo1.xyz;\n"
					     "vec3 CameraViewVector = CameraInfo2.xyz;\n"
						 "float CameraNear = CameraInfo1.w;\n"
						 "float CameraFar = CameraInfo2.w;\n"
						 "\n";
						 _insertStringAtLine(_d,uboCameraString,1);
					}
				}	
			}
        }
        void _init(string& name, Shader* vs, Shader* fs, ShaderRenderPass::Pass stage,ShaderP* super){
            m_Stage = stage;
            m_VertexShader = vs;
            m_FragmentShader = fs;

            super->setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
            super->setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
			super->setName(name);
            epriv::Core::m_Engine->m_RenderManager->_addShaderToStage(super,stage);
            
			string& _name = super->name();
			if(vs->name() == "NULL") vs->setName(_name + ".vert");
			if(fs->name() == "NULL") fs->setName(_name + ".frag");
            _compileOGL(m_VertexShader,m_FragmentShader,_name,super);
        }
        void _destruct(){
            glDeleteShader(m_ShaderProgram);
            glDeleteProgram(m_ShaderProgram);
            m_UniformLocations.clear();
			m_AttachedUBOs.clear();
        }
        void _compileOGL(Shader* vs,Shader* fs,string& _shaderProgramName,ShaderP* super){
            m_UniformLocations.clear();
			m_AttachedUBOs.clear();
            GLuint vid=glCreateShader(GL_VERTEX_SHADER);GLuint fid=glCreateShader(GL_FRAGMENT_SHADER);
            string VertexCode,FragmentCode = "";
            if(vs->fromFile()){
                boost::iostreams::stream<boost::iostreams::mapped_file_source> str(vs->data());
                for(string line;getline(str,line,'\n');){VertexCode+="\n"+line;}
            }
            else{VertexCode=vs->data();}
            if(fs->fromFile()){
                boost::iostreams::stream<boost::iostreams::mapped_file_source> str1(fs->data());
                for(string line;getline(str1,line,'\n');){FragmentCode+="\n"+line; }
            }
            else{FragmentCode=fs->data();}

            _convertCode(VertexCode,vs,FragmentCode,fs,super);

            GLint res=GL_FALSE; int ll;

            // Compile Vertex Shader
            char const* vss=VertexCode.c_str();glShaderSource(vid,1,&vss,NULL);glCompileShader(vid);

            // Check Vertex Shader
            glGetShaderiv(vid,GL_COMPILE_STATUS,&res);glGetShaderiv(vid,GL_INFO_LOG_LENGTH,&ll);vector<char>ve(ll);glGetShaderInfoLog(vid,ll,NULL,&ve[0]);

            if(res==GL_FALSE){
                if(vs->fromFile()){cout<<"VertexShader Log ("+vs->data()+"): "<<endl;}
                else{cout<<"VertexShader Log ("+vs->name()+"): "<<endl;}
                cout<<&ve[0]<<endl;
            }

            // Compile Fragment Shader
            char const* fss=FragmentCode.c_str();glShaderSource(fid,1,&fss,NULL);glCompileShader(fid);

            // Check Fragment Shader
            glGetShaderiv(fid,GL_COMPILE_STATUS,&res);glGetShaderiv(fid,GL_INFO_LOG_LENGTH,&ll);vector<char>fe(ll);glGetShaderInfoLog(fid,ll,NULL,&fe[0]);

            if(res==GL_FALSE){
                if(fs->fromFile()){cout<<"FragmentShader Log ("+fs->data()+"): "<<endl;}
                else{cout<<"FragmentShader Log ("+fs->name()+"): "<<endl;}
                cout<<&fe[0]<<endl;
            }

            // Link the program id
            m_ShaderProgram=glCreateProgram();
            glAttachShader(m_ShaderProgram,vid);glAttachShader(m_ShaderProgram,fid);
            glLinkProgram(m_ShaderProgram);
            glDetachShader(m_ShaderProgram,vid);glDetachShader(m_ShaderProgram,fid);
            glDeleteShader(vid);glDeleteShader(fid);

            // Check the program
            glGetProgramiv(m_ShaderProgram,GL_LINK_STATUS,&res);glGetProgramiv(m_ShaderProgram,GL_INFO_LOG_LENGTH,&ll);vector<char>pe(std::max(ll,int(1)));
            glGetProgramInfoLog(m_ShaderProgram,ll,NULL,&pe[0]);

            if(res == GL_FALSE){cout<<"ShaderProgram Log : "<<endl;cout<<&pe[0]<<endl;}
         
            //populate uniform table
            if(res==GL_TRUE){
                GLint _i,_count,_size;
                GLenum _type; // type of the variable (float, vec3 or mat4, etc)
                const GLsizei _bufSize = 256; // maximum name length
                GLchar _name[_bufSize]; // variable name in GLSL
                GLsizei _length; // name length
                glGetProgramiv(m_ShaderProgram,GL_ACTIVE_UNIFORMS,&_count);
                for(_i=0;_i<_count;++_i){
                    glGetActiveUniform(m_ShaderProgram,(GLuint)_i,_bufSize,&_length,&_size,&_type,_name);
                    if(_length>0){
                        string _name1((char*)_name,_length);
                        GLint _loc = glGetUniformLocation(m_ShaderProgram,_name);
                        this->m_UniformLocations.emplace(_name1,_loc);
                    }
                }
            }
        }
};
ShaderP::ShaderP(string n, Shader* vs, Shader* fs, ShaderRenderPass::Pass s):m_i(new impl){ m_i->_init(n,vs,fs,s,this); }
ShaderP::~ShaderP(){ m_i->_destruct(); }
GLuint ShaderP::program(){ return m_i->m_ShaderProgram; }
ShaderRenderPass::Pass ShaderP::stage(){ return m_i->m_Stage; }
vector<Material*>& ShaderP::getMaterials(){ return m_i->m_Materials; }

void ShaderP::bind(){
    epriv::Core::m_Engine->m_RenderManager->_bindShaderProgram(this);
    BindableResource::bind();
}
void ShaderP::unbind(){
    BindableResource::unbind();
}
void ShaderP::addMaterial(Handle& materialHandle){
    ShaderP::addMaterial(Resources::getMaterial(materialHandle));
}
void ShaderP::addMaterial(Material* material){
    m_i->m_Materials.push_back(material);
    sort(m_i->m_Materials.begin(),m_i->m_Materials.end(),epriv::srtKey());
}
const unordered_map<string,GLint>& ShaderP::uniforms() const { return this->m_i->m_UniformLocations; }


class UniformBufferObject::impl final{
    public:
        const char* nameInShader;
        uint sizeOfStruct;
		int globalBindingPointNumber;
        GLuint uboObject;
        void _init(const char* _nameInShader,uint& _sizeofStruct,int _globalBindingPointNumber){
            nameInShader = _nameInShader;
			if(epriv::RenderManager::GLSL_VERSION < 140) return;
			if(_globalBindingPointNumber == -1){
				//automatic assignment
				globalBindingPointNumber = UniformBufferObject::MAX_UBO_BINDINGS - UniformBufferObject::CUSTOM_UBO_AUTOMATIC_COUNT;
				++UniformBufferObject::CUSTOM_UBO_AUTOMATIC_COUNT;
			}
			else{
			    globalBindingPointNumber = _globalBindingPointNumber;
			}
            sizeOfStruct = _sizeofStruct;

            glGenBuffers(1, &uboObject);
            glBindBuffer(GL_UNIFORM_BUFFER, uboObject);

            glBufferData(GL_UNIFORM_BUFFER, sizeOfStruct, NULL, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_UNIFORM_BUFFER, globalBindingPointNumber, uboObject);

			glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }
        void _destruct(){
			if(epriv::RenderManager::GLSL_VERSION < 140) return;
			glDeleteBuffers(1,&uboObject);
        }
        void _update(void* _data){
			if(epriv::RenderManager::GLSL_VERSION < 140) return;
            glBindBuffer(GL_UNIFORM_BUFFER, uboObject);
			glBufferSubData(GL_UNIFORM_BUFFER,0, sizeOfStruct, _data);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }
        void _attachToShader(UniformBufferObject* super,ShaderP* _shaderProgram){
			if(epriv::RenderManager::GLSL_VERSION < 140 || _shaderProgram->m_i->m_AttachedUBOs.count(_shaderProgram->program())) return;
            uint block_index = glGetUniformBlockIndex(_shaderProgram->program(),nameInShader);
            glBindBufferBase(GL_UNIFORM_BUFFER, globalBindingPointNumber, uboObject);
            glUniformBlockBinding(_shaderProgram->program(), block_index, globalBindingPointNumber);
			_shaderProgram->m_i->m_AttachedUBOs.emplace(_shaderProgram->program(),true);
        }
};
UniformBufferObject::UniformBufferObject(const char* _nameInShader,uint _sizeofStruct,int _globalBindingPointNumber):m_i(new impl){ m_i->_init(_nameInShader,_sizeofStruct,_globalBindingPointNumber); }
UniformBufferObject::~UniformBufferObject(){ m_i->_destruct(); }
void UniformBufferObject::updateData(void* _data){ m_i->_update(_data); }
void UniformBufferObject::attachToShader(ShaderP* _shaderProgram){ m_i->_attachToShader(this,_shaderProgram); }