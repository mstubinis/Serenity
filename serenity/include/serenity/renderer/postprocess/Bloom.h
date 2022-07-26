#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_BLOOM_H
#define ENGINE_RENDERER_POSTPROCESS_BLOOM_H

class  ShaderProgram;
class  Shader;
class  Viewport;

#include <serenity/resources/Handle.h>
#include <string>
#include <serenity/system/TypeDefs.h>

namespace Engine::priv {
    class  GBuffer;
    class  RenderModule;
    class  Bloom final {
        private:
            Handle         m_Vertex_Shader;
            Handle         m_Fragment_Shader;
            Handle         m_Shader_Program;
        public:
            int       m_Num_Passes       = 3;
            float     m_Bloom_Strength   = 0.0f;
            float     m_Blur_Radius      = 1.24f;
            float     m_Blur_Strength    = 0.62f;
            float     m_Scale            = 0.27f;
            float     m_Threshold        = 0.55f;
            float     m_Exposure         = 1.6f;
            bool      m_Bloom_Active     = true;
            bool      m_Init             = false;

            void init();
            void pass(const GBuffer&, const Viewport&, uint32_t sceneTexture, const Engine::priv::RenderModule&);

            static Bloom STATIC_BLOOM;
    };
};
namespace Engine::Renderer::bloom {
    inline float getStrength() noexcept { return Engine::priv::Bloom::STATIC_BLOOM.m_Bloom_Strength; }
    inline void setStrength(float strength) noexcept { Engine::priv::Bloom::STATIC_BLOOM.m_Bloom_Strength = std::max(0.0f, strength); }
    inline float getThreshold() noexcept { return Engine::priv::Bloom::STATIC_BLOOM.m_Threshold; }
    inline void setThreshold(float threshold) noexcept { Engine::priv::Bloom::STATIC_BLOOM.m_Threshold = threshold; }
    inline float getExposure() noexcept { return Engine::priv::Bloom::STATIC_BLOOM.m_Exposure; }
    inline void setExposure(float exposure) noexcept { Engine::priv::Bloom::STATIC_BLOOM.m_Exposure = exposure; }
    inline bool enabled() noexcept { return Engine::priv::Bloom::STATIC_BLOOM.m_Bloom_Active; }
    inline int getNumPasses() noexcept { return Engine::priv::Bloom::STATIC_BLOOM.m_Num_Passes; }
    inline void setNumPasses(int numPasses) noexcept { Engine::priv::Bloom::STATIC_BLOOM.m_Num_Passes = std::max(0, numPasses); }
    inline void enable(bool enabled = true) noexcept { Engine::priv::Bloom::STATIC_BLOOM.m_Bloom_Active = enabled; }
    inline void disable() noexcept { Engine::priv::Bloom::STATIC_BLOOM.m_Bloom_Active = false; }
    inline float getBlurRadius() noexcept { return Engine::priv::Bloom::STATIC_BLOOM.m_Blur_Radius; }
    inline float getBlurStrength() noexcept { return Engine::priv::Bloom::STATIC_BLOOM.m_Blur_Strength; }
    inline void setBlurRadius(float blurRadius) noexcept { Engine::priv::Bloom::STATIC_BLOOM.m_Blur_Radius = std::max(0.0f, blurRadius); }
    inline void setBlurStrength(float blurStrength) noexcept { Engine::priv::Bloom::STATIC_BLOOM.m_Blur_Strength = std::max(0.0f, blurStrength); }
    inline float getScale() noexcept { return Engine::priv::Bloom::STATIC_BLOOM.m_Scale; }
    inline void setScale(float scale) noexcept { Engine::priv::Bloom::STATIC_BLOOM.m_Scale = std::max(0.0f, scale); }
};
#endif