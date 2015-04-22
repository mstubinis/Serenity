#include "Texture.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Camera.h"
#include "ShaderProgram.h"
#include "Mesh.h"
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

using namespace Engine;

Texture::Texture(std::string file,GLuint type,std::string name){
	m_TextureAddress = 0;
	m_Width = 0;
	m_Height = 0;
	m_Type = type;
	if(file != "")
		_loadFromFile(file,type);

	m_Name = name;
	if(name == ""){
		m_Name = file.substr(0,file.size()-4);
	}
	if(file != "")
		Resources::Detail::ResourceManagement::m_Textures[m_Name] = this;
}
Texture::Texture(std::string files[],GLuint type,std::string name){
	m_TextureAddress = 0;
	m_Width = 0;
	m_Height = 0;
	m_Type = type;
	_loadFromFiles(files,type);
	m_Name = name;

	Resources::Detail::ResourceManagement::m_Textures[m_Name] = this;
}
Texture::~Texture(){
	glDeleteTextures(1,&m_TextureAddress);
}
void Texture::_loadFromFile(std::string file,GLuint type){
	if(type == GL_TEXTURE_2D){
		glGenTextures(1, &m_TextureAddress);
		sf::Image image;

		glBindTexture(GL_TEXTURE_2D, m_TextureAddress);
		image.loadFromFile(file.c_str());
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.getSize().x, image.getSize().y, 0, GL_RGBA,GL_UNSIGNED_BYTE, image.getPixelsPtr());

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);

		m_Width = image.getSize().x;
		m_Height = image.getSize().y;
	}
}
void Texture::_loadFromFiles(std::string file[],GLuint type){
	if(type == GL_TEXTURE_CUBE_MAP){
		glGenTextures(1, &m_TextureAddress);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureAddress);
		for(unsigned int i = 0; i < 6; i++){
			sf::Image image;
		
			image.loadFromFile(file[i].c_str());
			GLenum skyboxSide;
			if(i==0)           skyboxSide = GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
			else if(i == 1)    skyboxSide = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
			else if(i == 2)    skyboxSide = GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
			else if(i == 3)    skyboxSide = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
			else if(i == 4)    skyboxSide = GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
			else               skyboxSide = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
			glTexImage2D(skyboxSide, 0, GL_RGBA,image.getSize().x, image.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE,image.getPixelsPtr());
		}
		glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
}
void Texture::render(glm::vec2& pos, glm::vec4 color,float angle, glm::vec2 scl, float depth){
	Engine::Renderer::Detail::RenderManagement::getTextureRenderQueue().push_back(TextureRenderInfo(m_Name,pos,color,scl,angle,depth));
}