#pragma once
#ifndef ENGINE_FRAMEBUFFER_OBJECT_H
#define ENGINE_FRAMEBUFFER_OBJECT_H

#include <core/engine/BindableResource.h>
#include <core/engine/renderer/GLImageConstants.h>
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
        
    class FramebufferObjectAttatchment{
        friend class  Engine::epriv::FramebufferObject;
        private:
            GLuint                     m_InternalFormat;
            GLuint                     m_GL_Attatchment;
            const FramebufferObject&   m_FBO;
        public:
            FramebufferObjectAttatchment(const FramebufferObject&, const FramebufferAttatchment::Attatchment&, const ImageInternalFormat::Format&);
            FramebufferObjectAttatchment(const FramebufferObject&, const FramebufferAttatchment::Attatchment&, const Texture&);
            virtual ~FramebufferObjectAttatchment();

            const uint width() const;
            const uint height() const;
            GLuint internalFormat();
            uint attatchment();
            virtual void resize(FramebufferObject&, const uint& width, const uint& height);
            virtual const GLuint& address() const;
            virtual void bind();
            virtual void unbind();
    };
    class FramebufferTexture final: public FramebufferObjectAttatchment{
        friend class  Engine::epriv::FramebufferObject;
        private:
            Texture*       m_Texture;
            GLuint         m_PixelFormat;
            GLuint         m_PixelType;
        public:
            FramebufferTexture(const FramebufferObject&,const FramebufferAttatchment::Attatchment&,const Texture&);
            virtual ~FramebufferTexture();

            void resize(FramebufferObject&, const uint& width, const uint& height);
            const GLuint& address() const;
            Texture& texture();
            void bind();
            void unbind();
    };
    class RenderbufferObject final: public FramebufferObjectAttatchment{
        friend class  Engine::epriv::FramebufferObject;
        private:
            GLuint m_RBO;
            uint   m_Width;
            uint   m_Height;
        public:
            RenderbufferObject(FramebufferObject&,FramebufferAttatchment::Attatchment,ImageInternalFormat::Format);
            virtual ~RenderbufferObject();

            void resize(FramebufferObject&, const uint& width, const uint& height);
            const GLuint& address() const;
            void bind();
            void unbind();
    };
    class FramebufferObject final: public BindableResource{
        friend class  Engine::epriv::FramebufferTexture;
        friend class  Engine::epriv::RenderbufferObject;
        friend struct Engine::epriv::FramebufferObjectDefaultBindFunctor;
        friend struct Engine::epriv::FramebufferObjectDefaultUnbindFunctor;
        private:
            uint                                                    m_CurrentFBOIndex;
            uint                                                    m_FramebufferWidth;
            uint                                                    m_FramebufferHeight;
            float                                                   m_Divisor;
            std::vector<GLuint>                                     m_FBO;
            std::unordered_map<uint, FramebufferObjectAttatchment*> m_Attatchments;
        public:
            FramebufferObject(const std::string& name, const uint& width, const uint& height, const float& divisor = 1.0f, const uint& swapBufferCount = 1);
            FramebufferObject(const std::string& name, const uint& width, const uint& height, const ImageInternalFormat::Format&, const float& divisor = 1.0f, const uint& swapBufferCount = 1);
            virtual ~FramebufferObject();

            void resize(const uint& width, const uint& height);
            FramebufferTexture* attatchTexture(Texture*, const FramebufferAttatchment::Attatchment&);
            RenderbufferObject* attatchRenderBuffer(RenderbufferObject&);
            const uint width() const;
            const uint height() const;
            std::unordered_map<uint,FramebufferObjectAttatchment*>& attatchments();
            const GLuint& address() const;
            const bool check();
            const float divisor() const;
    };
};
};

#endif
