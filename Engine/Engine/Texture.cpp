#include "Texture.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "ShaderProgram.h"
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

using namespace Engine;

Texture::Texture(const unsigned char* pixels,unsigned int w, unsigned int h,std::string name,GLuint type){
	m_TextureAddress = 0;
	m_Width = 0;
	m_Height = 0;
	m_Type = type;
	_loadFromPixels(pixels,w,h,type);
	m_Name = name;
	Resources::Detail::ResourceManagement::m_Textures[m_Name] = this;
}
Texture::Texture(std::string file,std::string name,GLuint type){
	m_Directory = file;
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
Texture::Texture(std::string files[],std::string name,GLuint type){
	m_Directory = "";
	m_TextureAddress = 0;
	m_Width = 0;
	m_Height = 0;
	m_Type = type;
	_loadFromFiles(files,type);
	m_Name = name;
	if(name == "Cubemap "){
		unsigned int total = 0;
		for(auto texture:Resources::Detail::ResourceManagement::m_Textures){
			std::string lower = texture.second->getName();
			boost::to_lower(lower);
			if(boost::algorithm::contains(lower,"cubemap")){
				total++;
			}
		}
		m_Name = "Cubemap " + boost::lexical_cast<std::string>(total);
	}
	Resources::Detail::ResourceManagement::m_Textures[m_Name] = this;
}
Texture::~Texture(){
	m_PixelsPointer.clear();
	glDeleteTextures(1,&m_TextureAddress);
	m_TextureAddress = 0;
	m_Width = 0;
	m_Height = 0;
}
void Texture::_loadFromPixels(const unsigned char* pixels, unsigned int w, unsigned int h,GLuint type){
	if(type == GL_TEXTURE_2D){
		glGenTextures(1, &m_TextureAddress);

		glBindTexture(GL_TEXTURE_2D, m_TextureAddress);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,GL_UNSIGNED_BYTE, pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);

		m_Width = w;
		m_Height = h;
	}
}
void Texture::_loadFromFile(std::string file,GLuint type){
	std::string extention;
	for(unsigned int i = file.length() - 4; i < file.length(); i++) extention += tolower(file.at(i));
	if(type == GL_TEXTURE_2D){
		glGenTextures(1, &m_TextureAddress);
		glBindTexture(GL_TEXTURE_2D, m_TextureAddress);

		sf::Image image;
		image.loadFromFile(file.c_str());

		if(extention == ".png")
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.getSize().x, image.getSize().y, 0, GL_RGBA,GL_UNSIGNED_BYTE, image.getPixelsPtr());
		else if(extention == ".jpg")
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.getSize().x, image.getSize().y, 0, GL_RGBA,GL_UNSIGNED_BYTE, image.getPixelsPtr());
		else
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
			std::string extention;
			for(unsigned int s = file[i].length() - 4; s < file[i].length(); s++) extention += tolower(file[i].at(s));

			sf::Image image;
			image.loadFromFile(file[i].c_str());

			GLenum skyboxSide;
			if(i==0)           skyboxSide = GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
			else if(i == 1)    skyboxSide = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
			else if(i == 2)    skyboxSide = GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
			else if(i == 3)    skyboxSide = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
			else if(i == 4)    skyboxSide = GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
			else               skyboxSide = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;

			if(extention == ".png")      glTexImage2D(skyboxSide, 0, GL_RGBA,image.getSize().x, image.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE,image.getPixelsPtr());
			else if(extention == ".jpg") glTexImage2D(skyboxSide, 0, GL_RGBA,image.getSize().x, image.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE,image.getPixelsPtr());
			else                         glTexImage2D(skyboxSide, 0, GL_RGBA,image.getSize().x, image.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE,image.getPixelsPtr());
		}
		glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
}
void Texture::generatePixelPointer(){
	sf::Image image;
	image.loadFromFile(m_Directory.c_str());
	std::vector<sf::Uint8> pixels;
	for(unsigned int i = 0; i < image.getSize().x; i++){
		for(unsigned int s = 0; s < image.getSize().y; s++){
			sf::Color pixel = image.getPixel(s,i);
			m_PixelsPointer.push_back(pixel.r);
			m_PixelsPointer.push_back(pixel.g);
			m_PixelsPointer.push_back(pixel.b);
			m_PixelsPointer.push_back(pixel.a);
		}
	}
}
void Texture::render(glm::vec2& pos, glm::vec4 color,float angle, glm::vec2 scl, float depth){
	Engine::Renderer::Detail::RenderManagement::getTextureRenderQueue().push_back(TextureRenderInfo(m_Name,pos,color,scl,angle,depth));
}