#pragma once
#ifndef ENGINE_FRAMEBUFFER_OBJECT_H
#define ENGINE_FRAMEBUFFER_OBJECT_H

#include "BindableResource.h"
#include "GLImageConstants.h"
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

class Texture;
class FramebufferObject;
typedef unsigned int uint;

class FramebufferObjectAttatchment{
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        FramebufferObjectAttatchment(FramebufferObject*,FramebufferAttatchment::Attatchment,ImageInternalFormat::Format);
        FramebufferObjectAttatchment(FramebufferObject*,FramebufferAttatchment::Attatchment,Texture*);
        ~FramebufferObjectAttatchment();

        uint width();
        uint height();
        FramebufferObject* fbo();
        uint attatchment();
        virtual void resize();
        virtual GLuint address();
};
class FramebufferTexture{
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        FramebufferTexture(FramebufferObject*,FramebufferAttatchment::Attatchment,Texture*);
        ~FramebufferTexture();

        void resize();
        GLuint address();
};
class RenderbufferObject: public FramebufferObjectAttatchment{
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        RenderbufferObject(FramebufferObject*,FramebufferAttatchment::Attatchment,ImageInternalFormat::Format);
        ~RenderbufferObject();

        void resize();
        GLuint address();
};

class FramebufferObject: public BindableResource{
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        FramebufferObject(std::string name,uint width,uint height);
        ~FramebufferObject();

        void resize(uint,uint);

        void attatchTexture(Texture*,FramebufferAttatchment::Attatchment);
        void attatchRenderBuffer(RenderbufferObject*);

        uint width();
        uint height();

        GLuint address();
};
#endif
