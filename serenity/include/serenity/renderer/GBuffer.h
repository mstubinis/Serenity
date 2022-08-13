#pragma once
#ifndef ENGINE_GBUFFER_H
#define ENGINE_GBUFFER_H

class Texture;
class Viewport;
class Camera;

#include <serenity/system/TypeDefs.h>

namespace Engine::priv {
    class FramebufferTexture;
    class FramebufferObject;
    struct GBufferType { enum Type : uint8_t {
        Diffuse,
        Normal,
        Misc,
        Lighting,
        Bloom,
        GodRays,
        Depth,
        _TOTAL,
    BackBuffer};};
};

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <serenity/utils/Utils.h>
#include <serenity/renderer/FramebufferObject.h>
#include <array>
#include <string_view>

namespace Engine::priv {
    class GBuffer final {
        public:
            class Impl;
        friend class GBuffer::Impl;
        private:
            std::vector<FramebufferObject>                        m_FBOs;
            std::array<FramebufferTexture*, GBufferType::_TOTAL>  m_FramebufferTextures  = { nullptr };
            uint32_t                                              m_Width                = 0U;
            uint32_t                                              m_Height               = 0U;

            GBuffer() = delete;
        public:
            GBuffer(uint32_t width, uint32_t height);
            GBuffer(const GBuffer&)                = delete;
            GBuffer& operator=(const GBuffer&)     = delete;
            GBuffer(GBuffer&&) noexcept            = delete;
            GBuffer& operator=(GBuffer&&) noexcept = delete;
            ~GBuffer();

            Engine::priv::FramebufferObject& createFBO(float divisor, uint32_t swapBufferCount = 1);
            Texture& createRenderTarget(size_t index, FramebufferObject&, FramebufferAttatchment, ImageInternalFormat, ImagePixelFormat, ImagePixelType, std::string textureName = {});

            //void init(uint32_t width, uint32_t height);
            bool resize(uint32_t width, uint32_t height);

            void bindFramebuffers(std::string_view channels, size_t fboIndex = 0, float x = 0.0f, float y = 0.0f, float width = 0.0f, float height = 0.0f);
            void bindFramebuffers(std::vector<uint32_t>&& buffers, std::string_view channels, size_t fboIndex = 0, float x = 0.0f, float y = 0.0f, float width = 0.0f, float height = 0.0f);

            void bindBackbuffer(const Viewport&, GLuint final_fbo = 0, GLuint final_rbo = 0);
            void bindBackbuffer(GLuint final_fbo = 0, GLuint final_rbo = 0);

            [[nodiscard]] inline uint32_t width() const noexcept { return m_Width; }
            [[nodiscard]] inline uint32_t height() const noexcept { return m_Height; }

            [[nodiscard]] inline const std::array<FramebufferTexture*, GBufferType::_TOTAL>& getBuffers() const noexcept { return m_FramebufferTextures; }
            [[nodiscard]] inline FramebufferTexture& getBuffer(const uint32_t index) const noexcept { return *m_FramebufferTextures[index]; }
            [[nodiscard]] inline Texture& getTexture(const uint32_t index) const noexcept { return m_FramebufferTextures[index]->texture(); }
            [[nodiscard]] inline FramebufferObject& getMainFBO() noexcept { return m_FBOs[0]; }
            [[nodiscard]] inline FramebufferObject& getSmallFBO() noexcept { return m_FBOs[1]; }
    };
};
#endif
