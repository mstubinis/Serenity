#pragma once
#ifndef ENGINE_GBUFFER_H
#define ENGINE_GBUFFER_H

class Texture;
class Viewport;
class Camera;
namespace Engine::priv {
    class FramebufferTexture;
    class FramebufferObject;

    struct GBufferType { enum Type : unsigned char {
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
            unsigned int                                          m_Width                = 0U;
            unsigned int                                          m_Height               = 0U;

            void internalBuildTextureBuffer(FramebufferObject& fbo, GBufferType::Type gbuffer_type, unsigned int w, unsigned int h);
            void internalDestruct();
            void internalStart(std::vector<unsigned int>& types, unsigned int size, std::string_view channels, bool first_fbo);
        public:
            GBuffer() = default;
            ~GBuffer();

            void init(unsigned int width, unsigned int height);
            bool resize(unsigned int width, unsigned int height);

            void bindFramebuffers(std::string_view channels = "RGBA", bool isMainFBO = true);
            void bindFramebuffers(unsigned int buffer, std::string_view channels = "RGBA", bool isMainFBO = true);
            void bindFramebuffers(unsigned int, unsigned int, std::string_view channels = "RGBA", bool isMainFBO = true);
            void bindFramebuffers(unsigned int, unsigned int, unsigned int, std::string_view channels = "RGBA", bool isMainFBO = true);
            void bindFramebuffers(unsigned int, unsigned int, unsigned int, unsigned int, std::string_view channels = "RGBA", bool isMainFBO = true);
            void bindFramebuffers(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, std::string_view channels = "RGBA", bool isMainFBO = true);

            void bindBackbuffer(const Viewport& viewport, GLuint final_fbo = 0, GLuint final_rbo = 0);

            inline constexpr unsigned int width() const noexcept { return m_Width; }
            inline constexpr unsigned int height() const noexcept { return m_Height; }

            inline constexpr const std::array<FramebufferTexture*, GBufferType::_TOTAL>& getBuffers() const noexcept { return m_FramebufferTextures; }
            FramebufferTexture& getBuffer(unsigned int) const;
            Texture& getTexture(unsigned int) const;

            inline constexpr const FramebufferObject& getMainFBO() const noexcept { return m_FBO; }
            inline constexpr const FramebufferObject& getSmallFBO() const noexcept { return m_SmallFBO; }
    };
};
#endif
