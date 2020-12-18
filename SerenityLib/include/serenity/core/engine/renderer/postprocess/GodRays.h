#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_GODRAYS_H
#define ENGINE_RENDERER_POSTPROCESS_GODRAYS_H

class  ShaderProgram;
class  Shader;
struct Entity;
class  Viewport;

#include <serenity/core/engine/resources/Handle.h>
#include <serenity/core/engine/dependencies/glm.h>
#include <string>

namespace Engine::priv {
    class  GBuffer;
    class  RenderModule;
    class  GodRays final {
        private:
            Handle          m_Vertex_Shader;
            Handle          m_Fragment_Shader;
            Handle          m_Shader_Program;
            std::string     m_GLSL_frag_code   = "";
        public:
            glm::vec4   clearColor             = glm::vec4(0.030f, 0.023f, 0.032f, 1.0f);
            float       exposure               = 0.03f;
            float       factor                 = 1.0f;
            float       decay                  = 0.97f;
            float       density                = 1.5f;
            float       weight                 = 0.567f;
            float       fovDegrees             = 75.0f;
            float       alphaFalloff           = 2.0f;
            int         samples                = 80;
            bool        godRays_active         = true;
            
            bool init_shaders();

            void pass(GBuffer&, const Viewport& viewport, const glm::vec2& lightScrnPos, float alpha, const RenderModule& renderer);

            static GodRays STATIC_GOD_RAYS;
    };
};
namespace Engine::Renderer::godRays {
    inline bool enabled() noexcept { return Engine::priv::GodRays::STATIC_GOD_RAYS.godRays_active; }
    inline void enable(bool enabled) noexcept { Engine::priv::GodRays::STATIC_GOD_RAYS.godRays_active = enabled; }
    inline void disable() noexcept { Engine::priv::GodRays::STATIC_GOD_RAYS.godRays_active = false; }
    inline float getExposure() noexcept { return Engine::priv::GodRays::STATIC_GOD_RAYS.exposure; }
    inline void setExposure(float exposure) noexcept { Engine::priv::GodRays::STATIC_GOD_RAYS.exposure = exposure; }
    inline float getFactor() noexcept { return Engine::priv::GodRays::STATIC_GOD_RAYS.factor; }
    inline void setFactor(float factor) noexcept { Engine::priv::GodRays::STATIC_GOD_RAYS.factor = factor; }
    inline float getDecay() noexcept { return Engine::priv::GodRays::STATIC_GOD_RAYS.decay; }
    inline void setDecay(float decay) noexcept { Engine::priv::GodRays::STATIC_GOD_RAYS.decay = decay; }
    inline float getDensity() noexcept { return Engine::priv::GodRays::STATIC_GOD_RAYS.density; }
    inline void setDensity(float density) noexcept { Engine::priv::GodRays::STATIC_GOD_RAYS.density = density; }
    inline float getWeight() noexcept { return Engine::priv::GodRays::STATIC_GOD_RAYS.weight; }
    inline void setWeight(float weight) noexcept { Engine::priv::GodRays::STATIC_GOD_RAYS.weight = weight; }
    inline int getSamples() noexcept { return Engine::priv::GodRays::STATIC_GOD_RAYS.samples; }
    inline void setSamples(int numSamples) noexcept { Engine::priv::GodRays::STATIC_GOD_RAYS.samples = glm::max(0, numSamples); }
    inline float getFOVDegrees() noexcept { return Engine::priv::GodRays::STATIC_GOD_RAYS.fovDegrees; }
    inline void setFOVDegrees(float fovInDegrees) noexcept { Engine::priv::GodRays::STATIC_GOD_RAYS.fovDegrees = fovInDegrees; }
    inline float getAlphaFalloff() noexcept { return Engine::priv::GodRays::STATIC_GOD_RAYS.alphaFalloff; }
    inline void setAlphaFalloff(float alphaFalloff) noexcept { Engine::priv::GodRays::STATIC_GOD_RAYS.alphaFalloff = alphaFalloff; }

    void setSun(Entity sun) noexcept;
    Entity getSun() noexcept;
};
#endif