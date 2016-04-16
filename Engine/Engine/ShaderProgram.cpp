#include "ShaderProgram.h"
#include "Engine_Resources.h"
#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>

ShaderP::ShaderP(std::string vs, std::string ps, bool fromFile){
    m_VertexShader = vs;
    m_PixelShader = ps;
	m_FromFile = fromFile;
}
ShaderP::~ShaderP(){
	cleanupRenderingContext(Engine::Resources::Detail::ResourceManagement::m_RenderingAPI);
}
void ShaderP::initRenderingContext(unsigned int api){
	if(api == ENGINE_RENDERING_API_OPENGL){
		m_Shader = CompileOpenGL(m_VertexShader.c_str(),m_PixelShader.c_str(), m_FromFile);
	}
	else if(api == ENGINE_RENDERING_API_DIRECTX){

	}
}
void ShaderP::cleanupRenderingContext(unsigned int api){
	if(api == ENGINE_RENDERING_API_OPENGL){
		glDeleteShader(m_Shader);
	}
	else if(api == ENGINE_RENDERING_API_DIRECTX){

	}
}
GLuint ShaderP::CompileOpenGL(std::string vs, std::string ps, bool fromFile){
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
 
    std::string VertexShaderCode = "";
    std::string FragmentShaderCode = "";

    if(fromFile){
        boost::iostreams::stream<boost::iostreams::mapped_file_source> str(vs);
        for(std::string line; std::getline(str, line, '\n');){
            VertexShaderCode += "\n" + line;
        }
        boost::iostreams::stream<boost::iostreams::mapped_file_source> str1(ps);
        for(std::string line; std::getline(str1, line, '\n');){
            FragmentShaderCode += "\n" + line;
        }
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
 
    // Compile Fragment Shader
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);
 
    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
 
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
 
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
 
    return ProgramID;
}

void ShaderP::CompileDirectX(std::string vs, std::string ps, bool fromFile){
}