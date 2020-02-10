#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_FXAA_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_FXAA_H_INCLUDE_GUARD

class  Shader;
class  ShaderProgram;

#include <string>

namespace Engine::priv {
    class  GBuffer;
    class  FXAA final {
        private:
            Shader*         m_Vertex_shader;
            Shader*         m_Fragment_shader;
            ShaderProgram*  m_Shader_program;
            std::string     m_GLSL_frag_code;
        public:
            float reduce_min;
            float reduce_mul;
            float span_max;

            FXAA();
            ~FXAA();

            const bool init_shaders();

            void pass(GBuffer&, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& sceneTextureEnum);

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