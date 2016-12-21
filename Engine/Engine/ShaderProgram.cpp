#include "ShaderProgram.h"
#include "Material.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>

#include <iostream>

using namespace Engine;

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
Shader::Shader(std::string& name, std::string& shaderFileOrData, SHADER_TYPE shaderType,bool fromFile):m_i(new impl()){
	m_i->_construct(name,shaderFileOrData,shaderType,fromFile,this);
}
Shader::~Shader(){
}
SHADER_TYPE Shader::type(){ return m_i->m_Type; }
std::string Shader::data(){ return m_i->m_Data; }
bool Shader::fromFile(){ return m_i->m_FromFile; }

class ShaderP::impl final{
    public:
		SHADER_PIPELINE_STAGE m_Stage;
        GLuint m_ShaderProgram;
		std::vector<skey> m_Materials;
		Shader* m_VertexShader;
		Shader* m_FragmentShader;
        void _construct(std::string& name, Shader* vs, Shader* ps, SHADER_PIPELINE_STAGE stage,ShaderP* super){
			m_Stage = stage;
            m_VertexShader = vs;
            m_FragmentShader = ps;
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
            _cleanupRenderingContext(Resources::Detail::ResourceManagement::m_RenderingAPI);
        }
        void _initRenderingContext(uint api){
            if(api == ENGINE_RENDERING_API_OPENGL){
                m_ShaderProgram = _compileOGL(m_VertexShader,m_FragmentShader);
            }
            else if(api == ENGINE_RENDERING_API_DIRECTX){

            }
        }
        void _cleanupRenderingContext(uint api){
            if(api == ENGINE_RENDERING_API_OPENGL){
                glDeleteShader(m_ShaderProgram);
            }
            else if(api == ENGINE_RENDERING_API_DIRECTX){

            }
        }
        GLuint _compileOGL(Shader* vs,Shader*  ps){
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
            glDeleteShader(vid);
            glDeleteShader(fid);
 
            return pid;
        }
        void _compileDX(std::string vs, std::string ps, bool fromFile){
        }
};

ShaderP::ShaderP(std::string& n, std::string& vs, std::string& fs, SHADER_PIPELINE_STAGE s):m_i(new impl()){
    m_i->_construct(n,vs,fs,s,this);
}
ShaderP::ShaderP(std::string& n, Shader* vs, Shader* fs, SHADER_PIPELINE_STAGE s):m_i(new impl()){
    m_i->_construct(n,vs,fs,s,this);
}
ShaderP::~ShaderP(){
    m_i->_destruct();
}
void ShaderP::initRenderingContext(uint api){
    m_i->_initRenderingContext(api);
}
void ShaderP::cleanupRenderingContext(uint api){
    m_i->_cleanupRenderingContext(api);
}
GLuint ShaderP::program(){ return m_i->m_ShaderProgram; }
Shader* ShaderP::vertexShader(){ return m_i->m_VertexShader; }
Shader* ShaderP::fragmentShader(){ return m_i->m_FragmentShader; }
SHADER_PIPELINE_STAGE ShaderP::stage(){ return m_i->m_Stage; }
std::vector<skey>& ShaderP::getMaterials(){ return m_i->m_Materials; }

void ShaderP::addMaterial(std::string m){
	if(m == "" || !Resources::Detail::ResourceManagement::m_Materials.count(m)){
		std::cout << "Material : '" << m << "' does not exist (ShaderP::addMaterial()) Returning..." << std::endl;
		return;
	}
	Material* mat = Resources::getMaterial(m);
	skey k(mat);
	m_i->m_Materials.push_back(k);
	std::sort(m_i->m_Materials.begin(),m_i->m_Materials.end(),sksortlessthan());
}