#include "ShaderProgram.h"

ShaderP::ShaderP(std::string vs, std::string ps, bool fromFile){
	m_VertexShader = vs;
	m_PixelShader = ps;
	m_Shader = CompileShader(m_VertexShader.c_str(),m_PixelShader.c_str(), fromFile);
}
ShaderP::~ShaderP(){
	glDeleteShader(m_Shader);
}