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
    struct GBufferType{enum Type{
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
            unsigned int                        m_Width;
            unsigned int                        m_Height;

            void internalBuildTextureBuffer(FramebufferObject& fbo, const GBufferType::Type gbuffer_type, const unsigned int& w, const unsigned int& h);
            void internalDestruct();
            void internalStart(const unsigned int* types, const unsigned int& size, const std::string& channels, const bool first_fbo);

        public:
            GBuffer();
            ~GBuffer();

            void init(const unsigned int& width, const unsigned int& height);
            const bool resize(const unsigned int& width, const unsigned int& height);

            void bindFramebuffers(const unsigned int, const std::string& channels = "RGBA", const bool isMainFBO = true);
            void bindFramebuffers(const unsigned int, const unsigned int, const std::string& channels = "RGBA", const bool isMainFBO = true);
            void bindFramebuffers(const unsigned int, const unsigned int, const unsigned int, const std::string& channels = "RGBA", const bool isMainFBO = true);
            void bindFramebuffers(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const std::string& channels = "RGBA", const bool isMainFBO = true);
            void bindFramebuffers(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const std::string& channels = "RGBA", const bool isMainFBO = true);

            void bindBackbuffer(const Viewport&, const GLuint final_fbo = 0, const GLuint final_rbo = 0);

            const std::vector<FramebufferTexture*>& getBuffers() const;
            FramebufferTexture& getBuffer(const unsigned int) const;
            Texture& getTexture(const unsigned int) const;

            const FramebufferObject& getMainFBO() const;
            const FramebufferObject& getSmallFBO() const;
    };
};
#endif
