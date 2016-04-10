#ifndef ENGINE_TEXTURE_H
#define ENGINE_TEXTURE_H

#include <string>
#include <vector>
#include <GL/glew.h>
#include <GL/GL.h>
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>


class Texture final{
    private:
		std::vector<unsigned char> m_Pixels;
        std::string m_Directory;
        std::string m_Name;
        GLuint m_TextureAddress;
        void _loadFromPixels(const unsigned char*,unsigned int, unsigned int,GLuint);
        void _loadFromFile(std::string file,GLuint);
        void _loadFromFilesCubemap(std::string file[],GLuint);
        GLuint m_Type;
        unsigned int m_Width, m_Height;
		void _init();
    public:
        Texture(const unsigned char*,unsigned int, unsigned int,std::string name = "",GLuint = GL_TEXTURE_2D);
        Texture(std::string file,std::string name = "",GLuint = GL_TEXTURE_2D);
        Texture(std::string file[],std::string name = "Cubemap ",GLuint = GL_TEXTURE_CUBE_MAP);
        ~Texture();

		const unsigned char* getPixels();

        const GLuint getTextureAddress() const { return m_TextureAddress; }
        const GLuint getTextureType() const { return m_Type; }
        const unsigned int width() const { return m_Width; }
        const unsigned int height() const { return m_Height; }
        const std::string getName() const { return m_Name; }

        void render(glm::vec2 pos, glm::vec4 color,float angle, glm::vec2 scl, float depth);

};
#endif