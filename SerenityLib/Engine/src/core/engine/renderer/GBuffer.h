#pragma once
#ifndef ENGINE_GBUFFER_H
#define ENGINE_GBUFFER_H

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <core/engine/utils/Utils.h>

class Texture;
class Viewport;
class Camera;
namespace Engine{
namespace epriv{
    class FramebufferTexture;
    class FramebufferObject;
    struct GBufferType{enum Type{
        Diffuse, Normal, Misc, Lighting, Bloom, GodRays, Depth,
    _TOTAL};};
    class GBuffer final{
        private:
            FramebufferObject*                m_FBO;
            FramebufferObject*                m_SmallFBO;
            std::vector<FramebufferTexture*>  m_Buffers;
            uint                              m_Width;
            uint                              m_Height;

            void internalBuildTextureBuffer(FramebufferObject* fbo, const GBufferType::Type gbuffer_type, const uint& w, const uint& h);
            void internalDestruct();
            void internalStart(const unsigned int* types, const unsigned int& size, const std::string& channels, const bool first_fbo);

        public:
            GBuffer(const uint& width, const uint& height);
            ~GBuffer();

            bool resize(const uint& width, const uint& height);

            void bindFramebuffers(const unsigned int, const std::string& channels = "RGBA", const bool isMainFBO = true);
            void bindFramebuffers(const unsigned int, const unsigned int, const std::string& channels = "RGBA", const bool isMainFBO = true);
            void bindFramebuffers(const unsigned int, const unsigned int, const unsigned int, const std::string& channels = "RGBA", const bool isMainFBO = true);
            void bindFramebuffers(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const std::string& channels = "RGBA", const bool isMainFBO = true);
            void bindFramebuffers(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const std::string& channels = "RGBA", const bool isMainFBO = true);

            void bindBackbuffer(Viewport&, const GLuint final_fbo = 0, const GLuint final_rbo = 0);

            const std::vector<FramebufferTexture*>& getBuffers() const;
            FramebufferTexture& getBuffer(const uint);
            Texture& getTexture(const uint);

            FramebufferObject* getMainFBO();
            FramebufferObject* getSmallFBO();
    };
};
};
#endif
