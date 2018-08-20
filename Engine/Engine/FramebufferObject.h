#pragma once
#ifndef ENGINE_FRAMEBUFFER_OBJECT_H
#define ENGINE_FRAMEBUFFER_OBJECT_H

#include "BindableResource.h"
#include "GLImageConstants.h"
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <unordered_map>

typedef unsigned int uint;

class Texture;
namespace Engine{
    namespace epriv{
        class FramebufferObject;
        class FramebufferObjectAttatchment;
        class FramebufferTexture;
        class RenderbufferObject;
        
        class FramebufferObjectAttatchment{
            private:
                class impl; std::unique_ptr<impl> m_i;
                FramebufferObjectAttatchment(const FramebufferObjectAttatchment&); // non construction-copyable
                FramebufferObjectAttatchment& operator=(const FramebufferObjectAttatchment&); // non copyable
				FramebufferObjectAttatchment(const FramebufferObjectAttatchment&&);           // non moveable 1
				FramebufferObjectAttatchment(FramebufferObjectAttatchment&&);                 // non moveable 2
				FramebufferObjectAttatchment& operator=(const FramebufferObjectAttatchment&&);// non move assignable 1
				FramebufferObjectAttatchment& operator=(FramebufferObjectAttatchment&&);      // non move assignable 2
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
                FramebufferTexture(const FramebufferTexture&); // non construction-copyable
                FramebufferTexture& operator=(const FramebufferTexture&); // non copyable
				FramebufferTexture(const FramebufferTexture&&);           // non moveable 1
				FramebufferTexture(FramebufferTexture&&);                 // non moveable 2
				FramebufferTexture& operator=(const FramebufferTexture&&);// non move assignable 1
				FramebufferTexture& operator=(FramebufferTexture&&);      // non move assignable 2
            public:
                FramebufferTexture(FramebufferObject*,FramebufferAttatchment::Attatchment,Texture*,float divisor = 1.0f);
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
                RenderbufferObject(const RenderbufferObject&); // non construction-copyable
                RenderbufferObject& operator=(const RenderbufferObject&); // non copyable
				RenderbufferObject(const RenderbufferObject&&);           // non moveable 1
				RenderbufferObject(RenderbufferObject&&);                 // non moveable 2
				RenderbufferObject& operator=(const RenderbufferObject&&);// non move assignable 1
				RenderbufferObject& operator=(RenderbufferObject&&);      // non move assignable 2
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
                FramebufferObject(const FramebufferObject&); // non construction-copyable
                FramebufferObject& operator=(const FramebufferObject&); // non copyable
				FramebufferObject(const FramebufferObject&&);           // non moveable 1
				FramebufferObject(FramebufferObject&&);                 // non moveable 2
				FramebufferObject& operator=(const FramebufferObject&&);// non move assignable 1
				FramebufferObject& operator=(FramebufferObject&&);      // non move assignable 2
            public:
                FramebufferObject(std::string name,uint width,uint height);
                FramebufferObject(std::string name,uint width,uint height,ImageInternalFormat::Format);
                ~FramebufferObject();

                void resize(uint,uint);
                FramebufferTexture* attatchTexture(Texture*,FramebufferAttatchment::Attatchment,float divisor = 1.0f);
                RenderbufferObject* attatchRenderBuffer(RenderbufferObject*);
                uint width();
                uint height();
                std::unordered_map<uint,FramebufferObjectAttatchment*>& attatchments();
                const GLuint& address() const;
                void check();
        };
    };
};

#endif
