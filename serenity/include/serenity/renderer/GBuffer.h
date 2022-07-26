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

namespace Engine::priv{
    class GBuffer final {
        private:
            mutable FramebufferObject                             m_FBO;
            mutable FramebufferObject                             m_SmallFBO;
            std::array<FramebufferTexture*, GBufferType::_TOTAL>  m_FramebufferTextures  = { nullptr };
            uint32_t                                              m_Width                = 0U;
            uint32_t                                              m_Height               = 0U;

            uint32_t internal_get_attatchment(uint32_t buffer) noexcept;
            void internal_Build_Texture_Buffer(FramebufferObject& fbo, GBufferType::Type gbuffer_type, uint32_t w, uint32_t h);
            void internal_Destruct();
            void internal_Start(std::vector<uint32_t>&& types, std::string_view channels, bool first_fbo, float x, float y, float width, float height);
        public:
            GBuffer() = default;
            GBuffer(const GBuffer&)                = delete;
            GBuffer& operator=(const GBuffer&)     = delete;
            GBuffer(GBuffer&&) noexcept            = delete;
            GBuffer& operator=(GBuffer&&) noexcept = delete;
            ~GBuffer();

            void init(uint32_t width, uint32_t height);
            bool resize(uint32_t width, uint32_t height);

            void bindFramebuffers(std::string_view channels, bool isMainFBO = true, float x = 0.0f, float y = 0.0f, float width = 0.0f, float height = 0.0f);
            void bindFramebuffers(std::vector<uint32_t>&&buffers, std::string_view channels, bool isMainFBO = true, float x = 0.0f, float y = 0.0f, float width = 0.0f, float height = 0.0f);

            void bindBackbuffer(const Viewport& viewport, GLuint final_fbo = 0, GLuint final_rbo = 0);
            void bindBackbuffer(GLuint final_fbo = 0, GLuint final_rbo = 0);

            [[nodiscard]] inline uint32_t width() const noexcept { return m_Width; }
            [[nodiscard]] inline uint32_t height() const noexcept { return m_Height; }

            [[nodiscard]] inline const std::array<FramebufferTexture*, GBufferType::_TOTAL>& getBuffers() const noexcept { return m_FramebufferTextures; }
            [[nodiscard]] FramebufferTexture& getBuffer(const uint32_t index) const noexcept;
            [[nodiscard]] Texture& getTexture(const uint32_t index) const noexcept;

            [[nodiscard]] inline FramebufferObject& getMainFBO() noexcept { return m_FBO; }
            [[nodiscard]] inline FramebufferObject& getSmallFBO() noexcept { return m_SmallFBO; }
    };
};
#endif
