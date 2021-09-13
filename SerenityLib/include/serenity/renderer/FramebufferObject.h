#pragma once
#ifndef ENGINE_FRAMEBUFFER_OBJECT_H
#define ENGINE_FRAMEBUFFER_OBJECT_H

class Texture;
namespace Engine::priv {
    class FramebufferObject;
    class FramebufferObjectAttatchment;
    struct FramebufferObjectPublicInterface;
    class FramebufferTexture;
    class RenderbufferObject;
};

#include <serenity/renderer/GLImageConstants.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/renderer/Renderer.h>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <functional>
#include <unordered_map>

namespace Engine::priv {     
    class FramebufferObjectAttatchment {
        friend class  Engine::priv::FramebufferObject;
        private:
            const FramebufferObject&   m_FBO;
            GLuint                     m_InternalFormat = 0;
            GLuint                     m_GL_Attatchment = 0;
        public:
            FramebufferObjectAttatchment(const FramebufferObject&, FramebufferAttatchment, ImageInternalFormat);
            FramebufferObjectAttatchment(const FramebufferObject&, FramebufferAttatchment, const Texture&);
            virtual ~FramebufferObjectAttatchment() {}

            uint32_t width() const;
            uint32_t height() const;
            inline constexpr GLuint internalFormat() const noexcept { return m_InternalFormat; }
            inline constexpr unsigned int attatchment() const noexcept { return m_GL_Attatchment; }

            virtual void resize(FramebufferObject&, uint32_t width, uint32_t height) {}
            virtual GLuint address() const { return 0; }
            virtual void bind() {}
            virtual void unbind() {}
    };
    class FramebufferTexture final: public FramebufferObjectAttatchment {
        friend class  Engine::priv::FramebufferObject;
        private:
            std::unique_ptr<Texture>  m_Texture;
            GLuint                    m_PixelFormat  = 0;
            GLuint                    m_PixelType    = 0;
        public:
            FramebufferTexture(const FramebufferObject&, FramebufferAttatchment, Texture*);


            void resize(FramebufferObject&, uint32_t width, uint32_t height) override;
            GLuint address() const override;
            inline Texture& texture() const noexcept { return *m_Texture; }
    };
    class RenderbufferObject final: public FramebufferObjectAttatchment {
        friend class  Engine::priv::FramebufferObject;
        private:
            GLuint     m_RBO    = 0;
            uint32_t   m_Width  = 0;
            uint32_t   m_Height = 0;
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
        friend struct Engine::priv::FramebufferObjectPublicInterface;
        using BindFP         = void(*)(const FramebufferObject*);
        using UnbindFP       = void(*)(const FramebufferObject*);
        using AttatchmentMap = std::unordered_map<uint32_t, std::unique_ptr<FramebufferObjectAttatchment>>;
        private:
            BindFP                    m_CustomBindFunctor   = [](const FramebufferObject*) {};
            UnbindFP                  m_CustomUnbindFunctor = [](const FramebufferObject*) {};

            std::vector<GLuint>       m_FBOs;
            mutable AttatchmentMap    m_Attatchments;
            mutable size_t            m_CurrentFBOIndex   = 0U;
            uint32_t                  m_FramebufferWidth  = 0U;
            uint32_t                  m_FramebufferHeight = 0U;
            float                     m_Divisor           = 1.0f;
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
            inline FramebufferObjectAttatchment* getAttatchement(const uint32_t index) const noexcept { return m_Attatchments.at(index).get(); }

            void resize(const uint32_t width, const uint32_t height);

            template<class ... ARGS>
            FramebufferTexture* attatchTexture(FramebufferAttatchment attatchment, ARGS&&... args) {
                if (m_Attatchments.contains((uint32_t)attatchment)) {
                    return nullptr;
                }
                m_Attatchments.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple((uint32_t)attatchment),
                    std::forward_as_tuple(std::make_unique<FramebufferTexture>(*this, attatchment, NEW Texture(std::forward<ARGS>(args)...)))
                );
                FramebufferTexture& fbTexture = *static_cast<FramebufferTexture*>(m_Attatchments.at((uint32_t)attatchment).get());
                for (const auto fbo : m_FBOs) {
                    Engine::Renderer::bindFBO(fbo);
                    glFramebufferTexture2D(GL_FRAMEBUFFER, fbTexture.attatchment(), fbTexture.m_Texture->getTextureType().toGLType(), fbTexture.m_Texture->address(), 0);
                }
                Engine::Renderer::unbindFBO();
                return &fbTexture;
            }
            template<class ... ARGS>
            RenderbufferObject* attatchRenderBuffer(ARGS&&... args) {
                RenderbufferObject* rbo = NEW RenderbufferObject(std::forward<ARGS>(args)...);
                if (m_Attatchments.contains(rbo->attatchment())) {
                    delete rbo;
                    return nullptr;
                }
                for (const auto fbo : m_FBOs) {
                    Engine::Renderer::bindFBO(fbo);
                    Engine::Renderer::bindRBO(*rbo);
                    glRenderbufferStorage(GL_RENDERBUFFER, rbo->internalFormat(), width(), height());
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, rbo->internalFormat(), GL_RENDERBUFFER, rbo->address());
                    Engine::Renderer::unbindRBO();
                }
                m_Attatchments.emplace(std::piecewise_construct, std::forward_as_tuple(rbo->attatchment()), std::forward_as_tuple(rbo));
                Engine::Renderer::unbindRBO();
                Engine::Renderer::unbindFBO();
                return rbo;
            }


            inline constexpr uint32_t width() const noexcept { return m_FramebufferWidth; }
            inline constexpr uint32_t height() const noexcept { return m_FramebufferHeight; }
            inline constexpr AttatchmentMap& attatchments() const noexcept { return m_Attatchments; }
            inline GLuint address() const noexcept { return m_FBOs[m_CurrentFBOIndex]; }

            //in debug mode, this binds the fbo and checks its status. in release mode, this only binds the fbo
            bool checkStatus();

            inline constexpr float divisor() const noexcept { return m_Divisor; }
    };
};

#endif
