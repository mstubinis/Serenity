#pragma once
#ifndef ENGINE_FRAMEBUFFER_OBJECT_H
#define ENGINE_FRAMEBUFFER_OBJECT_H

class Texture;
namespace Engine::priv {
    class FramebufferObject;
    class FramebufferObjectAttatchment;
    class FramebufferTexture;
    class RenderbufferObject;
};

#include <serenity/renderer/opengl/GLImageConstants.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/renderer/Renderer.h>

#include <serenity/renderer/opengl/APIStateOpenGL.h>
#include <serenity/renderer/direct3d/APIStateDirectX.h>

namespace Engine::priv {     
    class FramebufferObjectAttatchment {
        friend class  Engine::priv::FramebufferObject;
        private:
            FramebufferObject*         m_FBO            = nullptr;
            GLuint                     m_InternalFormat = 0;
            GLuint                     m_GL_Attatchment = 0;

            FramebufferObjectAttatchment() = delete;
        public:
            FramebufferObjectAttatchment(const FramebufferObjectAttatchment&) = delete;
            FramebufferObjectAttatchment& operator=(const FramebufferObjectAttatchment&) = delete;
            FramebufferObjectAttatchment(FramebufferObjectAttatchment&&) noexcept;
            FramebufferObjectAttatchment& operator=(FramebufferObjectAttatchment&&) noexcept;

            FramebufferObjectAttatchment(FramebufferObject&, FramebufferAttatchment, ImageInternalFormat);
            FramebufferObjectAttatchment(FramebufferObject&, FramebufferAttatchment, const Texture&);
            virtual ~FramebufferObjectAttatchment() = default;

            [[nodiscard]] uint32_t width() const;
            [[nodiscard]] uint32_t height() const;
            [[nodiscard]] inline constexpr GLuint internalFormat() const noexcept { return m_InternalFormat; }
            [[nodiscard]] inline constexpr unsigned int attatchment() const noexcept { return m_GL_Attatchment; }

            virtual void resize(FramebufferObject&, uint32_t width, uint32_t height) {}
            [[nodiscard]] virtual GLuint address() const { return 0; }
            virtual void bind() {};
            virtual void unbind() {};
    };
    class FramebufferTexture final: public FramebufferObjectAttatchment {
        friend class  Engine::priv::FramebufferObject;
        private:
            Engine::view_ptr<Texture>  m_Texture;
            GLuint                     m_PixelFormat  = 0;
            GLuint                     m_PixelType    = 0;

            FramebufferTexture() = delete;
        public:
            FramebufferTexture(const FramebufferTexture&)                = delete;
            FramebufferTexture& operator=(const FramebufferTexture&)     = delete;
            FramebufferTexture(FramebufferTexture&&) noexcept            = default;
            FramebufferTexture& operator=(FramebufferTexture&&) noexcept = default;

            FramebufferTexture(FramebufferObject&, FramebufferAttatchment, Texture*);


            void resize(FramebufferObject&, uint32_t width, uint32_t height) override;
            [[nodiscard]] GLuint address() const override;
            [[nodiscard]] inline Texture& texture() const noexcept { return *m_Texture; }
    };
    class RenderbufferObject final: public FramebufferObjectAttatchment {
        friend class  Engine::priv::FramebufferObject;
        private:
            GLuint     m_RBO    = 0;
            uint32_t   m_Width  = 0;
            uint32_t   m_Height = 0;

            RenderbufferObject() = delete;
        public:
            RenderbufferObject(const RenderbufferObject&)                = delete;
            RenderbufferObject& operator=(const RenderbufferObject&)     = delete;
            RenderbufferObject(RenderbufferObject&&) noexcept            = default;
            RenderbufferObject& operator=(RenderbufferObject&&) noexcept = default;

            RenderbufferObject(FramebufferAttatchment, FramebufferObject&, ImageInternalFormat);
            ~RenderbufferObject();

            void resize(FramebufferObject&, uint32_t width, uint32_t height) override;
            [[nodiscard]] GLuint address() const override { return m_RBO; }
            void bind() override;
            void unbind() override;
    };
    class FramebufferObject final {
        friend class  Engine::priv::FramebufferTexture;
        friend class  Engine::priv::RenderbufferObject;
        private:
            std::vector<GLuint>       m_FBOs;
            mutable std::unordered_map<uint32_t, FramebufferObjectAttatchment*>    m_Attatchments;
            mutable size_t            m_CurrentFBOIndex   = 0U;
            uint32_t                  m_FramebufferWidth  = 0U;
            uint32_t                  m_FramebufferHeight = 0U;
            float                     m_Divisor           = 1.0f;

            FramebufferObject() = delete;
        public:
            //FramebufferObject() = default;
            FramebufferObject(const FramebufferObject&) = delete;
            FramebufferObject& operator=(const FramebufferObject&) = delete;
            FramebufferObject(FramebufferObject&&) noexcept;
            FramebufferObject& operator=(FramebufferObject&&) noexcept;


            FramebufferObject(uint32_t width, uint32_t height, float divisor = 1.0f, uint32_t swapBufferCount = 1);
            FramebufferObject(uint32_t width, uint32_t height, ImageInternalFormat, float divisor = 1.0f, uint32_t swapBufferCount = 1);
            ~FramebufferObject();

            void init(uint32_t width, uint32_t height, float divisor = 1.0f, uint32_t swapBufferCount = 1);
            void init(uint32_t width, uint32_t height, ImageInternalFormat, float divisor = 1.0f, uint32_t swapBufferCount = 1);
            void cleanup();

            void bind(float x = 0.0f, float y = 0.0f, float width = 0.0f, float height = 0.0f);
            void unbind();
            [[nodiscard]] inline Engine::view_ptr<FramebufferObjectAttatchment> getAttatchement(const uint32_t index) const noexcept { return m_Attatchments.at(index); }

            void resize(const uint32_t width, const uint32_t height);

            template<class ... ARGS>
            FramebufferTexture* attatchTexture(FramebufferAttatchment attatchment, ARGS&&... args) {
                if (m_Attatchments.contains(attatchment)) {
                    return nullptr;
                }
                auto textureRenderTargetHandle = Engine::Resources::addResource<Texture>(std::forward<ARGS>(args)...);
                auto attachmentItr = m_Attatchments.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(attatchment),
                    std::forward_as_tuple(NEW FramebufferTexture(*this, attatchment, textureRenderTargetHandle.get<Texture>()))
                ).first;
                FramebufferTexture& fbTexture = *static_cast<FramebufferTexture*>(attachmentItr->second);
                for (const auto fbo : m_FBOs) {
                    Engine::opengl::bindFBO(fbo);
                    glFramebufferTexture2D(GL_FRAMEBUFFER, fbTexture.attatchment(), fbTexture.m_Texture->getTextureType().toGLType(), fbTexture.m_Texture->address(), 0);
                }
                Engine::opengl::unbindFBO();
                return &fbTexture;
            }
            template<class ... ARGS>
            RenderbufferObject* attatchRenderBuffer(FramebufferAttatchment attachment, ARGS&&... args) {
                if (m_Attatchments.contains(attachment)) {
                    return nullptr;
                }
                auto rboItr = m_Attatchments.emplace(
                    std::piecewise_construct, 
                    std::forward_as_tuple(attachment), 
                    std::forward_as_tuple(NEW RenderbufferObject(attachment, std::forward<ARGS>(args)...))
                ).first;
                for (const uint32_t fboHandle : m_FBOs) {
                    Engine::opengl::bindFBO(fboHandle);
                    Engine::opengl::bindRBO(rboItr->second->address());
                    glRenderbufferStorage(GL_RENDERBUFFER, rboItr->second->internalFormat(), width(), height());
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, rboItr->second->internalFormat(), GL_RENDERBUFFER, rboItr->second->address());
                    Engine::opengl::unbindRBO();
                }
                Engine::opengl::unbindRBO();
                Engine::opengl::unbindFBO();
                return static_cast<RenderbufferObject*>(rboItr->second);
            }


            [[nodiscard]] inline constexpr uint32_t width() const noexcept { return m_FramebufferWidth; }
            [[nodiscard]] inline constexpr uint32_t height() const noexcept { return m_FramebufferHeight; }
            [[nodiscard]] inline constexpr auto& attatchments() const noexcept { return m_Attatchments; }
            [[nodiscard]] inline GLuint address() const noexcept { return m_FBOs[m_CurrentFBOIndex]; }

            //in debug mode, this binds the fbo and checks its status. in release mode, this only binds the fbo
            bool checkStatus();

            [[nodiscard]] inline constexpr float divisor() const noexcept { return m_Divisor; }
    };
};

#endif
