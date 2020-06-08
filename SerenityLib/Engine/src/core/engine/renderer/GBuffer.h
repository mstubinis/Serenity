#pragma once
#ifndef ENGINE_GBUFFER_H
#define ENGINE_GBUFFER_H

class Texture;
class Viewport;
class Camera;
namespace Engine::priv {
    class FramebufferTexture;
    class FramebufferObject;
};

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <core/engine/utils/Utils.h>
#include <core/engine/renderer/FramebufferObject.h>

namespace Engine::priv{
    struct GBufferType { enum Type : unsigned char {
        Diffuse, 
        Normal, 
        Misc, 
        Lighting, 
        Bloom, 
        GodRays, 
        Depth,
    _TOTAL};};
    class GBuffer final : public Engine::NonCopyable, public Engine::NonMoveable{
        private:
            mutable FramebufferObject           m_FBO;
            mutable FramebufferObject           m_SmallFBO;
            std::vector<FramebufferTexture*>    m_FramebufferTextures;
            unsigned int                        m_Width                = 0U;
            unsigned int                        m_Height               = 0U;

            void internalBuildTextureBuffer(FramebufferObject& fbo, GBufferType::Type gbuffer_type, unsigned int w, unsigned int h);
            void internalDestruct();
            void internalStart(const unsigned int* types, unsigned int size, std::string_view channels, bool first_fbo);

        public:
            GBuffer() = default;
            ~GBuffer();

            void init(const unsigned int width, const unsigned int height);
            bool resize(const unsigned int width, const unsigned int height);

            void bindFramebuffers(unsigned int, std::string_view channels = "RGBA", bool isMainFBO = true);
            void bindFramebuffers(unsigned int, unsigned int, std::string_view channels = "RGBA", bool isMainFBO = true);
            void bindFramebuffers(unsigned int, unsigned int, unsigned int, std::string_view channels = "RGBA", bool isMainFBO = true);
            void bindFramebuffers(unsigned int, unsigned int, unsigned int, unsigned int, std::string_view channels = "RGBA", bool isMainFBO = true);
            void bindFramebuffers(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, std::string_view channels = "RGBA", bool isMainFBO = true);

            void bindBackbuffer(const Viewport& viewport, GLuint final_fbo = 0, GLuint final_rbo = 0);

            unsigned int width() const;
            unsigned int height() const;

            const std::vector<FramebufferTexture*>& getBuffers() const;
            FramebufferTexture& getBuffer(unsigned int) const;
            Texture& getTexture(unsigned int) const;

            const FramebufferObject& getMainFBO() const;
            const FramebufferObject& getSmallFBO() const;
    };
};
#endif
