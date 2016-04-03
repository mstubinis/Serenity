#ifndef ENGINE_SHADERPROGRAM_H
#define ENGINE_SHADERPROGRAM_H

#include <vector>
#include <string>
#include <GL/glew.h>
#include <GL/GL.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>


static GLuint CompileShader(const char* vShader, const char* fShader, bool fromFile = true){
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
 
    std::string VertexShaderCode = "";
    std::string FragmentShaderCode = "";

    if(fromFile){
        boost::iostreams::stream<boost::iostreams::mapped_file_source> str(vShader);
        for(std::string line; std::getline(str, line, '\n');){
            VertexShaderCode += "\n" + line;
        }
        boost::iostreams::stream<boost::iostreams::mapped_file_source> str1(fShader);
        for(std::string line; std::getline(str1, line, '\n');){
            FragmentShaderCode += "\n" + line;
        }
    }
    else{
        VertexShaderCode = vShader;
        FragmentShaderCode = fShader;
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

class ShaderP final{
    private:
        GLuint m_Shader;

        std::string m_VertexShader;
        std::string m_PixelShader;
    public:
        ShaderP(std::string vs,std::string ps, bool fromFile = true);
        ~ShaderP();

        const GLuint getShaderProgram() const{ return m_Shader; }

        const std::string getVertexShader() const{ return m_VertexShader; }
        const std::string getPixelShader() const{ return m_PixelShader; }
};

#endif