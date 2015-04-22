#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <GL/glew.h>
#include <GL/GL.h>
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>

class Texture{
	private:
		std::string m_Name;
		GLuint m_TextureAddress;
		void _loadFromFile(std::string file,GLuint);
		void _loadFromFiles(std::string file[],GLuint);
		GLuint m_Type;
		unsigned int m_Width, m_Height;
	public:

		Texture(std::string file,GLuint = GL_TEXTURE_2D,std::string name = "");
		Texture(std::string file[],GLuint = GL_TEXTURE_CUBE_MAP,std::string name = "");
		~Texture();

		const GLuint getTextureAddress() const { return m_TextureAddress; }
		const GLuint getTextureType() const { return m_Type; }
		const unsigned int getWidth() const { return m_Width; }
		const unsigned int getHeight() const { return m_Height; }
		const std::string getName() const { return m_Name; }

		void render(glm::vec2& pos, glm::vec4 color,float angle, glm::vec2 scl, float depth);

};
#endif