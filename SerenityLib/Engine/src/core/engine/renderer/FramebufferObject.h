#pragma once
#ifndef ENGINE_FRAMEBUFFER_OBJECT_H
#define ENGINE_FRAMEBUFFER_OBJECT_H

class Texture;
namespace Engine::priv {
    class FramebufferObject;
    class FramebufferObjectAttatchment;
    class FramebufferTexture;
    class RenderbufferObject;
    struct FramebufferObjectDefaultBindFunctor;
    struct FramebufferObjectDefaultUnbindFunctor;
};

#include <core/engine/renderer/GLImageConstants.h>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <unordered_map>
#include <functional>

namespace Engine::priv {     
    class FramebufferObjectAttatchment{
        friend class  Engine::priv::FramebufferObject;
        private:
            GLuint                     m_InternalFormat;
            GLuint                     m_GL_Attatchment;
            const FramebufferObject&   m_FBO;
        public:
            FramebufferObjectAttatchment(const FramebufferObject&, FramebufferAttatchment::Attatchment, ImageInternalFormat::Format);
            FramebufferObjectAttatchment(const FramebufferObject&, FramebufferAttatchment::Attatchment, const Texture&);
            virtual ~FramebufferObjectAttatchment();

            unsigned int width() const;
            unsigned int height() const;
            GLuint internalFormat() const;
            unsigned int attatchment() const;

            virtual void resize(FramebufferObject&, unsigned int width, unsigned int height);
            virtual GLuint address() const;
            virtual void bind();
            virtual void unbind();
    };
    class FramebufferTexture final: public FramebufferObjectAttatchment{
        friend class  Engine::priv::FramebufferObject;
        private:
            Texture*       m_Texture      = nullptr;
            GLuint         m_PixelFormat;
            GLuint         m_PixelType;
        public:
            FramebufferTexture(const FramebufferObject&, FramebufferAttatchment::Attatchment, const Texture&);
            ~FramebufferTexture();

            void resize(FramebufferObject&, unsigned int width, unsigned int height) override;
            GLuint address() const override;
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
            ~RenderbufferObject();

            void resize(FramebufferObject&, unsigned int width, unsigned int height) override;
            GLuint address() const override;
            void bind() override;
            void unbind() override;
    };
    class FramebufferObject final {
        friend class  Engine::priv::FramebufferTexture;
        friend class  Engine::priv::RenderbufferObject;
        friend struct Engine::priv::FramebufferObjectDefaultBindFunctor;
        friend struct Engine::priv::FramebufferObjectDefaultUnbindFunctor;
        private:
            std::function<void(const FramebufferObject*)>                           m_CustomBindFunctor   = [](const FramebufferObject*) {};
            std::function<void(const FramebufferObject*)>                           m_CustomUnbindFunctor = [](const FramebufferObject*) {};

            mutable size_t                                                          m_CurrentFBOIndex;
            unsigned int                                                            m_FramebufferWidth  = 0U;
            unsigned int                                                            m_FramebufferHeight = 0U;
            float                                                                   m_Divisor           = 1.0f;
            std::vector<GLuint>                                                     m_FBO;
            mutable std::unordered_map<unsigned int, FramebufferObjectAttatchment*> m_Attatchments;
        public:
            FramebufferObject();
            FramebufferObject(unsigned int width, unsigned int height, float divisor = 1.0f, unsigned int swapBufferCount = 1);
            FramebufferObject(unsigned int width, unsigned int height, ImageInternalFormat::Format, float divisor = 1.0f, unsigned int swapBufferCount = 1);
            ~FramebufferObject();

            template<typename T> void setCustomBindFunctor(const T& functor) {
                m_CustomBindFunctor = std::bind<void>(std::move(functor), std::placeholders::_1);
            }
            template<typename T> void setCustomUnbindFunctor(const T& functor) {
                m_CustomUnbindFunctor = std::bind<void>(std::move(functor), std::placeholders::_1);
            }
            void bind() const;
            void unbind() const;

            void init(unsigned int width, unsigned int height, float divisor = 1.0f, unsigned int swapBufferCount = 1);
            void init(unsigned int width, unsigned int height, ImageInternalFormat::Format, float divisor = 1.0f, unsigned int swapBufferCount = 1);

            void cleanup();

            FramebufferObjectAttatchment* getAttatchement(unsigned int) const;

            void resize(unsigned int width, unsigned int height);
            FramebufferTexture* attatchTexture(Texture*, FramebufferAttatchment::Attatchment);
            RenderbufferObject* attatchRenderBuffer(RenderbufferObject&);
            unsigned int width() const;
            unsigned int height() const;
            std::unordered_map<unsigned int, FramebufferObjectAttatchment*>& attatchments() const;
            GLuint address() const;
            bool check();
            float divisor() const;
    };
};

#endif
