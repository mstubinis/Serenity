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
            virtual ~FramebufferObjectAttatchment() {}

            unsigned int width() const;
            unsigned int height() const;
            inline constexpr GLuint internalFormat() const noexcept { return m_InternalFormat; }
            inline constexpr unsigned int attatchment() const noexcept { return m_GL_Attatchment; }

            virtual void resize(FramebufferObject&, unsigned int width, unsigned int height) {}
            virtual GLuint address() const { return 0; }
            virtual void bind() {}
            virtual void unbind() {}
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
            void bind() override {}
            void unbind() override {}
    };
    class RenderbufferObject final: public FramebufferObjectAttatchment{
        friend class  Engine::priv::FramebufferObject;
        private:
            GLuint         m_RBO    = 0U;
            unsigned int   m_Width  = 0U;
            unsigned int   m_Height = 0U;
        public:
            RenderbufferObject(FramebufferObject&, FramebufferAttatchment::Attatchment, ImageInternalFormat::Format);
            ~RenderbufferObject();

            void resize(FramebufferObject&, unsigned int width, unsigned int height) override;
            GLuint address() const override { return m_RBO; }
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

            mutable size_t                                                          m_CurrentFBOIndex     = 0U;
            unsigned int                                                            m_FramebufferWidth    = 0U;
            unsigned int                                                            m_FramebufferHeight   = 0U;
            float                                                                   m_Divisor             = 1.0f;
            std::vector<GLuint>                                                     m_FBO;
            mutable std::unordered_map<unsigned int, FramebufferObjectAttatchment*> m_Attatchments;
        public:
            FramebufferObject() = default;
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

            inline FramebufferObjectAttatchment* getAttatchement(unsigned int index) const noexcept { return m_Attatchments.at(index); }

            void resize(unsigned int width, unsigned int height);
            FramebufferTexture* attatchTexture(Texture*, FramebufferAttatchment::Attatchment);
            RenderbufferObject* attatchRenderBuffer(RenderbufferObject&);
            inline constexpr unsigned int width() const noexcept { return m_FramebufferWidth; }
            inline constexpr unsigned int height() const noexcept { return m_FramebufferHeight; }
            inline std::unordered_map<unsigned int, FramebufferObjectAttatchment*>& attatchments() const noexcept { return m_Attatchments; }
            GLuint address() const;
            bool check();
            inline constexpr float divisor() const noexcept { return m_Divisor; }
    };
};

#endif
