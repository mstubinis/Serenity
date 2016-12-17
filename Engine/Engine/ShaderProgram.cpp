#include "ShaderProgram.h"
#include "Engine_Resources.h"
#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>

#include <iostream>

using namespace Engine;

class ShaderP::impl final{
    public:
        GLuint m_Shader;
        bool m_FromFile;
        std::string m_VertexShader;
        std::string m_PixelShader;
		std::string m_ProgramName;

        void _construct(std::string name, std::string vs, std::string ps, bool fromFile){
            m_VertexShader = vs;
            m_PixelShader = ps;
            m_FromFile = fromFile;
			m_ProgramName = name;
        }
        void _destruct(){
            _cleanupRenderingContext(Resources::Detail::ResourceManagement::m_RenderingAPI);
        }
        void _initRenderingContext(uint api){
            if(api == ENGINE_RENDERING_API_OPENGL){
                m_Shader = _compileOGL(m_VertexShader.c_str(),m_PixelShader.c_str(), m_FromFile);
            }
            else if(api == ENGINE_RENDERING_API_DIRECTX){

            }
        }
        void _cleanupRenderingContext(uint api){
            if(api == ENGINE_RENDERING_API_OPENGL){
                glDeleteShader(m_Shader);
            }
            else if(api == ENGINE_RENDERING_API_DIRECTX){

            }
        }
        GLuint _compileOGL(std::string vs,std::string ps,bool fromFile){
            // Create the shaders
            GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
            GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
 
            std::string VertexShaderCode = ""; std::string FragmentShaderCode = "";
            if(fromFile){
                boost::iostreams::stream<boost::iostreams::mapped_file_source> str(vs);
                for(std::string line; std::getline(str, line, '\n');){ VertexShaderCode += "\n" + line; }
                boost::iostreams::stream<boost::iostreams::mapped_file_source> str1(ps);
                for(std::string line; std::getline(str1, line, '\n');){ FragmentShaderCode += "\n" + line; }
            }
            else{
                VertexShaderCode = vs;
                FragmentShaderCode = ps;
            }
 
            GLint Result = GL_FALSE;
            int InfoLogLength;
 
            // Compile Vertex Shader
            char const * VertexSourcePointer = VertexShaderCode.c_str();
            glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
            glCompileShader(VertexShaderID);
 
            // Check Vertex Shader
            glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
            glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
            std::vector<char> VertexShaderErrorMessage(InfoLogLength);
            glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);

			if(Result == GL_FALSE) {
				if(fromFile){ std::cout << "VertexShader Log (" + vs + "): " << std::endl; }
				else{         std::cout << "VertexShader Log : " << std::endl; }
				std::cout << &VertexShaderErrorMessage[0] << std::endl;
			}
 
            // Compile Fragment Shader
            char const * FragmentSourcePointer = FragmentShaderCode.c_str();
            glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
            glCompileShader(FragmentShaderID);
 
            // Check Fragment Shader
            glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
            glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
            std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
            glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
 
			if(Result == GL_FALSE) {
				if(fromFile){ std::cout << "FragmentShader Log (" + ps + "): " << std::endl; }
				else{         std::cout << "FragmentShader Log : " << std::endl; }
				std::cout << &FragmentShaderErrorMessage[0] << std::endl;
			}

            // Link the program
            GLuint ProgramID = glCreateProgram();
            glAttachShader(ProgramID, VertexShaderID);
            glAttachShader(ProgramID, FragmentShaderID);
            glLinkProgram(ProgramID);
 
            // Check the program
            glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
            glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
            std::vector<char> ProgramErrorMessage( std::max(InfoLogLength, int(1)) );
            glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
 
			if(Result == GL_FALSE) {
				std::cout << "ShaderProgram Log ("+m_ProgramName+"): " << std::endl;
				std::cout << &ProgramErrorMessage[0] << std::endl;
			}
            glDeleteShader(VertexShaderID);
            glDeleteShader(FragmentShaderID);
 
            return ProgramID;
        }
        void _compileDX(std::string vs, std::string ps, bool fromFile){
        }
};

ShaderP::ShaderP(std::string name,std::string vs, std::string ps, bool fromFile):m_i(new impl()){
    m_i->_construct(name,vs,ps,fromFile);
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
GLuint ShaderP::program(){ return m_i->m_Shader; }
std::string ShaderP::vertexShader(){ return m_i->m_VertexShader; }
std::string ShaderP::pixelShader(){ return m_i->m_PixelShader; }