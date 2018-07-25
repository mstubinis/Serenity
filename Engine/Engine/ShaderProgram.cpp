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


class Shader::impl final{
    public:
        ShaderType::Type m_Type;
        bool m_FromFile;
        string m_Data;
        void _construct(string& data, ShaderType::Type type, bool fromFile,Shader* super){
            m_Data = data;
            m_Type = type;
            m_FromFile = fromFile;
			if(fromFile){
				super->setName(data);
			}
			else{
				super->setName("ShaderFromMemory");
			}
        }
};
Shader::Shader(string shaderFileOrData, ShaderType::Type shaderType,bool fromFile):m_i(new impl){
    m_i->_construct(shaderFileOrData,shaderType,fromFile,this);
}
Shader::~Shader(){
}
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
		void _convertCode(string& _data1,Shader* shader1,string& _data2,Shader* shader2){
			_convertCode(_data1,shader1); _convertCode(_data2,shader2);
		}
        void _convertCode(string& _data,Shader* shader){
            istringstream str(_data); string line; 
            
            //get the first line with actual content
            while(true){
                getline(str,line);
                if(line != "" && line != "\n"){
                    break;
                }
            }
            string versionNumberString = regex_replace(line,regex("([^0-9])"),"");
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
        }
        void _construct(string& name, Shader* vs, Shader* fs, ShaderRenderPass::Pass stage,ShaderP* super){
            m_Stage = stage;
            m_VertexShader = vs;
            m_FragmentShader = fs;
            m_UniformLocations.clear();

            epriv::Core::m_Engine->m_RenderManager->_addShaderToStage(super,stage);
            super->setName(name);

            super->setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
            super->setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
            _compileOGL(m_VertexShader,m_FragmentShader,name);
        }
        void _destruct(){
            glDeleteShader(m_ShaderProgram);
            glDeleteProgram(m_ShaderProgram);
            m_UniformLocations.clear();
        }
        void _compileOGL(Shader* vs,Shader*  ps,string& _shaderProgramName){
            m_UniformLocations.clear();
            GLuint vid=glCreateShader(GL_VERTEX_SHADER);GLuint fid=glCreateShader(GL_FRAGMENT_SHADER);
            string VertexCode,FragmentCode = "";
            if(vs->fromFile()){
                boost::iostreams::stream<boost::iostreams::mapped_file_source> str(vs->data());
                for(string line;getline(str,line,'\n');){VertexCode+="\n"+line;}
            }
            else{VertexCode=vs->data();}
            if(ps->fromFile()){
                boost::iostreams::stream<boost::iostreams::mapped_file_source> str1(ps->data());
                for(string line;getline(str1,line,'\n');){FragmentCode+="\n"+line; }
            }
            else{FragmentCode=ps->data();}

            _convertCode(VertexCode,vs,FragmentCode,ps);

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
                if(ps->fromFile()){cout<<"FragmentShader Log ("+ps->data()+"): "<<endl;}
                else{cout<<"FragmentShader Log ("+ps->name()+"): "<<endl;}
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
ShaderP::ShaderP(string n, Shader* vs, Shader* fs, ShaderRenderPass::Pass s):m_i(new impl){ m_i->_construct(n,vs,fs,s,this); }
ShaderP::~ShaderP(){
    m_i->_destruct();
}
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
