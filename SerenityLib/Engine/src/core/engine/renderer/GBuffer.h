#pragma once
#ifndef ENGINE_GBUFFER_H
#define ENGINE_GBUFFER_H

class Texture;
class Viewport;
class Camera;
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
    _TOTAL};};
};

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <core/engine/utils/Utils.h>
#include <core/engine/renderer/FramebufferObject.h>

namespace Engine::priv{
    class GBuffer final : public Engine::NonCopyable, public Engine::NonMoveable{
        private:
            mutable FramebufferObject                             m_FBO;
            mutable FramebufferObject                             m_SmallFBO;
            std::array<FramebufferTexture*, GBufferType::_TOTAL>  m_FramebufferTextures;
            uint32_t                                              m_Width                = 0U;
            uint32_t                                              m_Height               = 0U;

            void internal_Build_Texture_Buffer(FramebufferObject& fbo, GBufferType::Type gbuffer_type, uint32_t w, uint32_t h);
            void internal_Destruct();
            void internal_Start(std::vector<uint32_t>& types, std::string_view channels, bool first_fbo);
        public:
            GBuffer() = default;
            ~GBuffer();

            void init(uint32_t width, uint32_t height);
            bool resize(uint32_t width, uint32_t height);

            void bindFramebuffers(std::string_view channels = "RGBA", bool isMainFBO = true);
            void bindFramebuffers(uint32_t buffer, std::string_view channels = "RGBA", bool isMainFBO = true);
            void bindFramebuffers(uint32_t, uint32_t, std::string_view channels = "RGBA", bool isMainFBO = true);
            void bindFramebuffers(uint32_t, uint32_t, uint32_t, std::string_view channels = "RGBA", bool isMainFBO = true);
            void bindFramebuffers(uint32_t, uint32_t, uint32_t, uint32_t, std::string_view channels = "RGBA", bool isMainFBO = true);
            void bindFramebuffers(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, std::string_view channels = "RGBA", bool isMainFBO = true);

            void bindBackbuffer(const Viewport& viewport, GLuint final_fbo = 0, GLuint final_rbo = 0);

            inline CONSTEXPR uint32_t width() const noexcept { return m_Width; }
            inline CONSTEXPR uint32_t height() const noexcept { return m_Height; }

            inline CONSTEXPR const std::array<FramebufferTexture*, GBufferType::_TOTAL>& getBuffers() const noexcept { return m_FramebufferTextures; }
            FramebufferTexture& getBuffer(uint32_t index) const noexcept;
            Texture& getTexture(uint32_t index) const noexcept;

            inline CONSTEXPR const FramebufferObject& getMainFBO() const noexcept { return m_FBO; }
            inline CONSTEXPR const FramebufferObject& getSmallFBO() const noexcept { return m_SmallFBO; }
    };
};
#endif
