#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_FXAA_H
#define ENGINE_RENDERER_POSTPROCESS_FXAA_H

class  Shader;
class  ShaderProgram;
class  Viewport;

#include <serenity/resources/Handle.h>
#include <string>
#include <serenity/dependencies/glm.h>

namespace Engine::priv {
    class  GBuffer;
    class  RenderModule;
    class  FXAA final {
        private:
            Handle         m_Vertex_shader;
            Handle         m_Fragment_shader;
            Handle         m_Shader_program;
            std::string    m_GLSL_frag_code;

            void internal_init_fragment_code();
        public:
            float reduce_min                  = 0.0078125f; // (1 / 128);
            float reduce_mul                  = 0.125f;     // (1 / 8);
            float span_max                    = 8.0f;

            static bool init();

            void pass(GBuffer&, const Viewport& viewport, uint32_t sceneTexture, const Engine::priv::RenderModule& renderer);

            static FXAA STATIC_FXAA;
    };
};
namespace Engine::Renderer::fxaa {
    inline void setReduceMin(float reduceMin) noexcept { Engine::priv::FXAA::STATIC_FXAA.reduce_min = glm::max(0.0f, reduceMin); }
    inline float getReduceMin() noexcept { return Engine::priv::FXAA::STATIC_FXAA.reduce_min; }
    inline void setReduceMul(float reduceMul) noexcept { Engine::priv::FXAA::STATIC_FXAA.reduce_mul = glm::max(0.0f, reduceMul); }
    inline float getReduceMul() noexcept { return Engine::priv::FXAA::STATIC_FXAA.reduce_mul; }
    inline void setSpanMax(float spanMax) noexcept { Engine::priv::FXAA::STATIC_FXAA.span_max = glm::max(0.0f, spanMax); }
    inline float getSpanMax() noexcept { return Engine::priv::FXAA::STATIC_FXAA.span_max; }
};

#endif