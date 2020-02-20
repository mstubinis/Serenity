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
    namespace priv{
        class FramebufferObject;
        class FramebufferObjectAttatchment;
        class FramebufferTexture;
        class RenderbufferObject;
        struct FramebufferObjectDefaultBindFunctor;
        struct FramebufferObjectDefaultUnbindFunctor;
        
        class FramebufferObjectAttatchment{
            friend class  Engine::priv::FramebufferObject;
            private:
                GLuint                     m_InternalFormat;
                GLuint                     m_GL_Attatchment;
                const FramebufferObject&   m_FBO;
            public:
                FramebufferObjectAttatchment(const FramebufferObject&, const FramebufferAttatchment::Attatchment&, const ImageInternalFormat::Format&);
                FramebufferObjectAttatchment(const FramebufferObject&, const FramebufferAttatchment::Attatchment&, const Texture&);
                virtual ~FramebufferObjectAttatchment();

                const unsigned int width() const;
                const unsigned int height() const;
                const GLuint internalFormat() const;
                const unsigned int attatchment() const;

                virtual void resize(FramebufferObject&, const unsigned int& width, const unsigned int& height);
                virtual const GLuint address() const;
                virtual void bind();
                virtual void unbind();
        };
        class FramebufferTexture final: public FramebufferObjectAttatchment{
            friend class  Engine::priv::FramebufferObject;
            private:
                Texture*       m_Texture;
                GLuint         m_PixelFormat;
                GLuint         m_PixelType;
            public:
                FramebufferTexture(const FramebufferObject&, const FramebufferAttatchment::Attatchment&, const Texture&);
                virtual ~FramebufferTexture();

                void resize(FramebufferObject&, const unsigned int& width, const unsigned int& height) override;
                const GLuint address() const override;
                Texture& texture() const;
                void bind() override;
                void unbind() override;
        };
        class RenderbufferObject final: public FramebufferObjectAttatchment{
            friend class  Engine::priv::FramebufferObject;
            private:
                GLuint         m_RBO;
                unsigned int   m_Width;
                unsigned int   m_Height;
            public:
                RenderbufferObject(FramebufferObject&, FramebufferAttatchment::Attatchment, ImageInternalFormat::Format);
                virtual ~RenderbufferObject();

                void resize(FramebufferObject&, const unsigned int& width, const unsigned int& height) override;
                const GLuint address() const override;
                void bind() override;
                void unbind() override;
        };
        class FramebufferObject final : public BindableResource {
            friend class  Engine::priv::FramebufferTexture;
            friend class  Engine::priv::RenderbufferObject;
            friend struct Engine::priv::FramebufferObjectDefaultBindFunctor;
            friend struct Engine::priv::FramebufferObjectDefaultUnbindFunctor;
            private:
                size_t                                                             m_CurrentFBOIndex;
                unsigned int                                                       m_FramebufferWidth;
                unsigned int                                                       m_FramebufferHeight;
                float                                                              m_Divisor;
                std::vector<GLuint>                                                m_FBO;
                std::unordered_map<unsigned int, FramebufferObjectAttatchment*>    m_Attatchments;
            public:
                FramebufferObject();
                FramebufferObject(const unsigned int& width, const unsigned int& height, const float& divisor = 1.0f, const unsigned int& swapBufferCount = 1);
                FramebufferObject(const unsigned int& width, const unsigned int& height, const ImageInternalFormat::Format&, const float& divisor = 1.0f, const unsigned int& swapBufferCount = 1);
                virtual ~FramebufferObject();

                void init(const unsigned int& width, const unsigned int& height, const float& divisor = 1.0f, const unsigned int& swapBufferCount = 1);
                void init(const unsigned int& width, const unsigned int& height, const ImageInternalFormat::Format&, const float& divisor = 1.0f, const unsigned int& swapBufferCount = 1);

                void resize(const unsigned int& width, const unsigned int& height);
                FramebufferTexture* attatchTexture(Texture*, const FramebufferAttatchment::Attatchment);
                RenderbufferObject* attatchRenderBuffer(RenderbufferObject&);
                const unsigned int width() const;
                const unsigned int height() const;
                std::unordered_map<unsigned int, FramebufferObjectAttatchment*>& attatchments();
                const GLuint address() const;
                const bool check();
                const float divisor() const;
        };
    };
};

#endif
