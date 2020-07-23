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
            GLuint                     m_InternalFormat = 0U;
            GLuint                     m_GL_Attatchment = 0U;
            const FramebufferObject&   m_FBO;
        public:
            FramebufferObjectAttatchment(const FramebufferObject&, FramebufferAttatchment, ImageInternalFormat);
            FramebufferObjectAttatchment(const FramebufferObject&, FramebufferAttatchment, const Texture&);
            virtual ~FramebufferObjectAttatchment() {}

            unsigned int width() const;
            unsigned int height() const;
            inline CONSTEXPR GLuint internalFormat() const noexcept { return m_InternalFormat; }
            inline CONSTEXPR unsigned int attatchment() const noexcept { return m_GL_Attatchment; }

            virtual void resize(FramebufferObject&, unsigned int width, unsigned int height) {}
            virtual GLuint address() const { return 0; }
            virtual void bind() {}
            virtual void unbind() {}
    };
    class FramebufferTexture final: public FramebufferObjectAttatchment{
        friend class  Engine::priv::FramebufferObject;
        private:
            Texture*       m_Texture      = nullptr;
            GLuint         m_PixelFormat  = 0U;
            GLuint         m_PixelType    = 0U;
        public:
            FramebufferTexture(const FramebufferObject&, FramebufferAttatchment, const Texture&);
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
            RenderbufferObject(FramebufferObject&, FramebufferAttatchment, ImageInternalFormat);
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
            FramebufferObject(unsigned int width, unsigned int height, ImageInternalFormat, float divisor = 1.0f, unsigned int swapBufferCount = 1);
            ~FramebufferObject();

            void setCustomBindFunctor(std::function<void(const FramebufferObject*)>&& functor) noexcept {
                m_CustomBindFunctor = std::move(functor);
            }
            void setCustomUnbindFunctor(std::function<void(const FramebufferObject*)>&& functor) noexcept {
                m_CustomUnbindFunctor = std::move(functor);
            }
            void bind() const;
            void unbind() const;

            void init(unsigned int width, unsigned int height, float divisor = 1.0f, unsigned int swapBufferCount = 1);
            void init(unsigned int width, unsigned int height, ImageInternalFormat, float divisor = 1.0f, unsigned int swapBufferCount = 1);

            void cleanup();

            inline CONSTEXPR FramebufferObjectAttatchment* getAttatchement(unsigned int index) const noexcept { return m_Attatchments.at(index); }

            void resize(unsigned int width, unsigned int height);
            FramebufferTexture* attatchTexture(Texture*, FramebufferAttatchment);
            RenderbufferObject* attatchRenderBuffer(RenderbufferObject&);
            inline CONSTEXPR unsigned int width() const noexcept { return m_FramebufferWidth; }
            inline CONSTEXPR unsigned int height() const noexcept { return m_FramebufferHeight; }
            inline CONSTEXPR std::unordered_map<unsigned int, FramebufferObjectAttatchment*>& attatchments() const noexcept { return m_Attatchments; }
            GLuint address() const;
            bool check();
            inline CONSTEXPR float divisor() const noexcept { return m_Divisor; }
    };
};

#endif
