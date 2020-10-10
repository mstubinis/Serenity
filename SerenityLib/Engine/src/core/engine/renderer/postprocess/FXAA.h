#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_FXAA_H
#define ENGINE_RENDERER_POSTPROCESS_FXAA_H

class  Shader;
class  ShaderProgram;
class  Viewport;

#include <core/engine/resources/Handle.h>

namespace Engine::priv {
    class  GBuffer;
    class  RenderModule;
    class  FXAA final {
        private:
            Handle         m_Vertex_shader;
            Handle         m_Fragment_shader;
            Handle         m_Shader_program;
            std::string    m_GLSL_frag_code  = "";

            FXAA() = default;
            ~FXAA() = default;
        public:
            float reduce_min                  = 0.0078125f; // (1 / 128);
            float reduce_mul                  = 0.125f;     // (1 / 8);
            float span_max                    = 8.0f;

            bool init_shaders();

            void pass(GBuffer&, const Viewport& viewport, unsigned int sceneTexture, const Engine::priv::RenderModule& renderer);

            static FXAA STATIC_FXAA;
    };
};
namespace Engine::Renderer::fxaa {
    void setReduceMin(float reduceMin);
    float getReduceMin();
    void setReduceMul(float reduceMul);
    float getReduceMul();
    void setSpanMax(float spanMax);
    float getSpanMax();
};

#endif