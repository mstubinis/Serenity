#pragma once
#ifndef ENGINE_TEXTURE_H
#define ENGINE_TEXTURE_H

#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_CUBE_MAP 0x8513

#include "Engine_ResourceBasic.h"

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <memory>

typedef unsigned int GLuint;
typedef unsigned int uint;
typedef unsigned char uchar;
namespace sf{ class Image; }

class Texture: public EngineResource{
    private:
        class impl;
        std::unique_ptr<impl> m_i;
    public:
        Texture(std::string name,uint w, uint h,GLuint = GL_TEXTURE_2D);
        Texture(std::string file,std::string name = "",GLuint = GL_TEXTURE_2D);
        Texture(sf::Image&,std::string name = "",GLuint = GL_TEXTURE_2D);
        Texture(std::string file[],std::string name = "Cubemap",GLuint = GL_TEXTURE_CUBE_MAP);
        virtual ~Texture();

        uchar* pixels();
        GLuint& address();
        GLuint type();
        uint width();
        uint height();

        virtual void load();
        virtual void unload();

        void _constructAsFramebuffer(uint,uint,float,int,int,int,int);

        void render(glm::vec2& pos, glm::vec4& color,float angle, glm::vec2& scl, float depth);

};
#endif