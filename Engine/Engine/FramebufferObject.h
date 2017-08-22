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
        virtual ~FramebufferObjectAttatchment();

        virtual uint width();
        virtual uint height();
        virtual GLuint internalFormat();
        virtual FramebufferObject* fbo();
        virtual uint attatchment();
        virtual void resize(uint,uint);
        virtual GLuint address();
        virtual void bind();
        virtual void unbind();
};
class FramebufferTexture final: public FramebufferObjectAttatchment{
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        FramebufferTexture(FramebufferObject*,FramebufferAttatchment::Attatchment,Texture*,float divisor);
        FramebufferTexture(FramebufferObject*,FramebufferAttatchment::Attatchment,Texture*);
        ~FramebufferTexture();

        void resize(uint,uint);
        GLuint address();
        Texture* texture();
        void bind();
        void unbind();
        float divisor();
};
class RenderbufferObject final: public FramebufferObjectAttatchment{
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        RenderbufferObject(FramebufferObject*,FramebufferAttatchment::Attatchment,ImageInternalFormat::Format);
        ~RenderbufferObject();

        void resize(uint,uint);
        GLuint address();
        void bind();
        void unbind();
};

class FramebufferObject final: public BindableResource{
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        FramebufferObject(std::string name,uint width,uint height);
        FramebufferObject(std::string name,uint width,uint height,ImageInternalFormat::Format);
        ~FramebufferObject();

        void resize(uint,uint);
        FramebufferTexture* attatchTexture(Texture*,FramebufferAttatchment::Attatchment);
        FramebufferTexture* attatchTexture(Texture*,FramebufferAttatchment::Attatchment,float divisor);
        RenderbufferObject* attatchRenderBuffer(RenderbufferObject*);
        uint width();
        uint height();
        std::unordered_map<uint,FramebufferObjectAttatchment*>& attatchments();
        GLuint address();
        void check();
};
#endif
