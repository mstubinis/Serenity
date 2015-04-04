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