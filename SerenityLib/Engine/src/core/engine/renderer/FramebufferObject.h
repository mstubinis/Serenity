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
    class FramebufferObjectAttatchment {
        friend class  Engine::priv::FramebufferObject;
        private:
            GLuint                     m_InternalFormat = 0U;
            GLuint                     m_GL_Attatchment = 0U;
            const FramebufferObject&   m_FBO;
        public:
            FramebufferObjectAttatchment(const FramebufferObject&, FramebufferAttatchment, ImageInternalFormat);
            FramebufferObjectAttatchment(const FramebufferObject&, FramebufferAttatchment, const Texture&);
            virtual ~FramebufferObjectAttatchment() {}

            uint32_t width() const;
            uint32_t height() const;
            inline CONSTEXPR GLuint internalFormat() const noexcept { return m_InternalFormat; }
            inline CONSTEXPR unsigned int attatchment() const noexcept { return m_GL_Attatchment; }

            virtual void resize(FramebufferObject&, uint32_t width, uint32_t height) {}
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

            void resize(FramebufferObject&, uint32_t width, uint32_t height) override;
            GLuint address() const override;
            Texture& texture() const;
            void bind() override {}
            void unbind() override {}
    };
    class RenderbufferObject final: public FramebufferObjectAttatchment{
        friend class  Engine::priv::FramebufferObject;
        private:
            GLuint     m_RBO    = 0U;
            uint32_t   m_Width  = 0U;
            uint32_t   m_Height = 0U;
        public:
            RenderbufferObject(FramebufferObject&, FramebufferAttatchment, ImageInternalFormat);
            ~RenderbufferObject();

            void resize(FramebufferObject&, uint32_t width, uint32_t height) override;
            GLuint address() const override { return m_RBO; }
            void bind() override;
            void unbind() override;
    };
    class FramebufferObject final {
        friend class  Engine::priv::FramebufferTexture;
        friend class  Engine::priv::RenderbufferObject;
        friend struct Engine::priv::FramebufferObjectDefaultBindFunctor;
        friend struct Engine::priv::FramebufferObjectDefaultUnbindFunctor;
        using BindFP         = std::function<void(const FramebufferObject*)>;
        using UnbindFP       = std::function<void(const FramebufferObject*)>;
        using AttatchmentMap = std::unordered_map<uint32_t, FramebufferObjectAttatchment*>;
        private:
            BindFP                    m_CustomBindFunctor   = [](const FramebufferObject*) {};
            UnbindFP                  m_CustomUnbindFunctor = [](const FramebufferObject*) {};

            mutable size_t            m_CurrentFBOIndex     = 0U;
            uint32_t                  m_FramebufferWidth    = 0U;
            uint32_t                  m_FramebufferHeight   = 0U;
            float                     m_Divisor             = 1.0f;
            std::vector<GLuint>       m_FBO;
            mutable AttatchmentMap    m_Attatchments;
        public:
            FramebufferObject() = default;
            FramebufferObject(uint32_t width, uint32_t height, float divisor = 1.0f, uint32_t swapBufferCount = 1);
            FramebufferObject(uint32_t width, uint32_t height, ImageInternalFormat, float divisor = 1.0f, uint32_t swapBufferCount = 1);
            ~FramebufferObject();

            void init(uint32_t width, uint32_t height, float divisor = 1.0f, uint32_t swapBufferCount = 1);
            void init(uint32_t width, uint32_t height, ImageInternalFormat, float divisor = 1.0f, uint32_t swapBufferCount = 1);
            void cleanup();

            inline void setCustomBindFunctor(const BindFP& functor) noexcept { m_CustomBindFunctor = functor; }
            inline void setCustomUnbindFunctor(const UnbindFP& functor) noexcept { m_CustomUnbindFunctor = functor; }
            inline void setCustomBindFunctor(BindFP&& functor) noexcept { m_CustomBindFunctor = std::move(functor); }
            inline void setCustomUnbindFunctor(UnbindFP&& functor) noexcept { m_CustomUnbindFunctor = std::move(functor); }
            inline void bind() const noexcept { m_CustomBindFunctor(this); }
            inline void unbind() const noexcept { m_CustomUnbindFunctor(this); }
            inline CONSTEXPR FramebufferObjectAttatchment* getAttatchement(uint32_t index) const noexcept { return m_Attatchments.at(index); }

            void resize(uint32_t width, uint32_t height);
            FramebufferTexture* attatchTexture(Texture*, FramebufferAttatchment);
            RenderbufferObject* attatchRenderBuffer(RenderbufferObject&);
            inline CONSTEXPR uint32_t width() const noexcept { return m_FramebufferWidth; }
            inline CONSTEXPR uint32_t height() const noexcept { return m_FramebufferHeight; }
            inline CONSTEXPR std::unordered_map<uint32_t, FramebufferObjectAttatchment*>& attatchments() const noexcept { return m_Attatchments; }
            inline GLuint address() const noexcept { return m_FBO[m_CurrentFBOIndex]; }
            bool checkStatus();
            inline CONSTEXPR float divisor() const noexcept { return m_Divisor; }
    };
};

#endif
