#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_BLOOM_H
#define ENGINE_RENDERER_POSTPROCESS_BLOOM_H

class  ShaderProgram;
class  Shader;
class  Viewport;

#include <core/engine/resources/Handle.h>

namespace Engine::priv {
    class  GBuffer;
    class  RenderModule;
    class  Bloom final {
        private:
            std::string    m_GLSL_frag_code = "";
            Handle         m_Vertex_Shader;
            Handle         m_Fragment_Shader;
            Handle         m_Shader_Program;
        public:
            uint32_t  m_Num_Passes       = 3;
            bool      m_Bloom_Active     = true;
            float     m_Blur_Radius      = 1.24f;
            float     m_Blur_Strength    = 0.62f;
            float     m_Scale            = 0.27f;
            float     m_Threshold        = 0.55f;
            float     m_Exposure         = 1.6f;

            bool init_shaders();

            void pass(GBuffer&, const Viewport& viewport, uint32_t sceneTexture, const Engine::priv::RenderModule& renderer);

            static Bloom bloom;
    };
};
namespace Engine::Renderer::bloom {
    inline float getThreshold() noexcept { return Engine::priv::Bloom::bloom.m_Threshold; }
    inline void setThreshold(float threshold) noexcept { Engine::priv::Bloom::bloom.m_Threshold = threshold; }
    inline float getExposure() noexcept { return Engine::priv::Bloom::bloom.m_Exposure; }
    inline void setExposure(float e) noexcept { Engine::priv::Bloom::bloom.m_Exposure = e; }
    inline bool enabled() noexcept { return Engine::priv::Bloom::bloom.m_Bloom_Active; }
    inline uint32_t getNumPasses() noexcept { return Engine::priv::Bloom::bloom.m_Num_Passes; }
    inline void setNumPasses(uint32_t numPasses) noexcept { Engine::priv::Bloom::bloom.m_Num_Passes = numPasses; }
    inline void enable(bool enabled = true) noexcept { Engine::priv::Bloom::bloom.m_Bloom_Active = enabled; }
    inline void disable() noexcept { Engine::priv::Bloom::bloom.m_Bloom_Active = false; }
    inline float getBlurRadius() noexcept { return Engine::priv::Bloom::bloom.m_Blur_Radius; }
    inline float getBlurStrength() noexcept { return Engine::priv::Bloom::bloom.m_Blur_Strength; }
    inline void setBlurRadius(float blurRadius) noexcept { Engine::priv::Bloom::bloom.m_Blur_Radius = glm::max(0.0f, blurRadius); }
    inline void setBlurStrength(float blurStrength) noexcept { Engine::priv::Bloom::bloom.m_Blur_Strength = glm::max(0.0f, blurStrength); }
    inline float getScale() noexcept { return Engine::priv::Bloom::bloom.m_Scale; }
    inline void setScale(float scale) noexcept { Engine::priv::Bloom::bloom.m_Scale = glm::max(0.0f, scale); }
};
#endif