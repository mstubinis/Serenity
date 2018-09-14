#pragma once
#ifndef ENGINE_FRAMEBUFFER_OBJECT_H
#define ENGINE_FRAMEBUFFER_OBJECT_H

#include "BindableResource.h"
#include "GLImageConstants.h"
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <unordered_map>

class Texture;
namespace Engine{
    namespace epriv{
        class FramebufferObject;
        class FramebufferObjectAttatchment;
        class FramebufferTexture;
        class RenderbufferObject;
        struct FramebufferObjectDefaultBindFunctor;
        struct FramebufferObjectDefaultUnbindFunctor;
        
		class FramebufferObjectAttatchment: private Engine::epriv::noncopyable{
            friend class ::Engine::epriv::FramebufferObject;
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
                virtual void resize(FramebufferObject*,uint width,uint height);
                virtual GLuint address();
                virtual void bind();
                virtual void unbind();
        };
        class FramebufferTexture final: public FramebufferObjectAttatchment{
            friend class ::Engine::epriv::FramebufferObject;
            private:
                class impl; std::unique_ptr<impl> m_i;
            public:
                FramebufferTexture(FramebufferObject*,FramebufferAttatchment::Attatchment,Texture*);
                virtual ~FramebufferTexture();

                void resize(FramebufferObject*,uint width,uint height);
                GLuint address();
                Texture* texture();
                void bind();
                void unbind();
        };
        class RenderbufferObject final: public FramebufferObjectAttatchment{
            friend class ::Engine::epriv::FramebufferObject;
            private:
                class impl; std::unique_ptr<impl> m_i;
            public:
                RenderbufferObject(FramebufferObject*,FramebufferAttatchment::Attatchment,ImageInternalFormat::Format);
                virtual ~RenderbufferObject();

                void resize(FramebufferObject*,uint width,uint height);
                GLuint address();
                void bind();
                void unbind();
        };
        class FramebufferObject final: public BindableResource{
            friend class ::Engine::epriv::FramebufferTexture;
            friend class ::Engine::epriv::RenderbufferObject;
            friend struct ::Engine::epriv::FramebufferObjectDefaultBindFunctor;
            friend struct ::Engine::epriv::FramebufferObjectDefaultUnbindFunctor;
            private:
                class impl; std::unique_ptr<impl> m_i;
            public:
                FramebufferObject(std::string name,uint width,uint height,float divisor = 1.0f,uint swapBufferCount = 1);
                FramebufferObject(std::string name,uint width,uint height,ImageInternalFormat::Format,float divisor = 1.0f, uint swapBufferCount = 1);
                virtual ~FramebufferObject();

                void resize(uint width,uint height);
                FramebufferTexture* attatchTexture(Texture*,FramebufferAttatchment::Attatchment);
                RenderbufferObject* attatchRenderBuffer(RenderbufferObject*);
                uint width();
                uint height();
                std::unordered_map<uint,FramebufferObjectAttatchment*>& attatchments();
                const GLuint& address() const;
                bool check();
				float divisor();
        };
    };
};

#endif
