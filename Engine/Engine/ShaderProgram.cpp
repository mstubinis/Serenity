#include "ShaderProgram.h"

ShaderP::ShaderP(std::string vs, std::string ps){
	m_VertexShader = vs;
	if(vs == ""){
		m_VertexShader = "Shaders\\vert.glsl";
	}
	m_PixelShader = ps;
	if(ps == ""){
		m_PixelShader = "Shaders\\frag.glsl";
	}
	m_Shader = CompileShader(m_VertexShader.c_str(),m_PixelShader.c_str());
}
ShaderP::~ShaderP(){
	glDeleteShader(m_Shader);
}
std::string ShaderP::Vertex_Shader()const { return m_VertexShader; }
std::string ShaderP::Pixel_Shader()const { return m_PixelShader; }
GLuint ShaderP::Get_Shader_Program()const { return m_Shader; }