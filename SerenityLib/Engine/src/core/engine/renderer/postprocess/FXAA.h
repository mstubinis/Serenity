#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_FXAA_H
#define ENGINE_RENDERER_POSTPROCESS_FXAA_H

class  Shader;
class  ShaderProgram;
class  Viewport;

namespace Engine::priv {
    class  GBuffer;
    class  Renderer;
    class  FXAA final {
        private:
            std::unique_ptr<Shader>         m_Vertex_shader;
            std::unique_ptr<Shader>         m_Fragment_shader;
            std::unique_ptr<ShaderProgram>  m_Shader_program;
            std::string                     m_GLSL_frag_code  = "";

            FXAA() = default;
            ~FXAA() = default;
        public:
            float reduce_min                  = 0.0078125f; // (1 / 128);
            float reduce_mul                  = 0.125f;     // (1 / 8);
            float span_max                    = 8.0f;

            bool init_shaders();

            void pass(GBuffer&, const Viewport& viewport, unsigned int sceneTexture, const Engine::priv::Renderer& renderer);

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