#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_FXAA_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_FXAA_H_INCLUDE_GUARD

class  Shader;
class  ShaderProgram;
class  Viewport;

#include <string>

namespace Engine::priv {
    class  GBuffer;
    class  Renderer;
    class  FXAA final {
        private:
            Shader*         m_Vertex_shader   = nullptr;
            Shader*         m_Fragment_shader = nullptr;
            ShaderProgram*  m_Shader_program  = nullptr;
            std::string     m_GLSL_frag_code  = "";
        public:
            float reduce_min                  = 0.0078125f; // (1 / 128);
            float reduce_mul                  = 0.125f;     // (1 / 8);
            float span_max                    = 8.0f;

            FXAA() = default;
            ~FXAA();

            const bool init_shaders();

            void pass(GBuffer&, const Viewport& viewport, const unsigned int& sceneTextureEnum, const Engine::priv::Renderer& renderer);

            static FXAA fxaa;
    };
};
namespace Engine::Renderer::fxaa {
    void setReduceMin(const float r);
    const float getReduceMin();
    void setReduceMul(const float r);
    const float getReduceMul();
    void setSpanMax(const float r);
    const float getSpanMax();
};

#endif