#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_DOF_H
#define ENGINE_RENDERER_POSTPROCESS_DOF_H

class  ShaderProgram;
class  Shader;
class  Viewport;

namespace Engine::priv {
    class  GBuffer;
    class  Renderer;
    class  DepthOfField final {
        private:
            Shader*        m_Vertex_Shader   = nullptr;
            Shader*        m_Fragment_Shader = nullptr;
            ShaderProgram* m_Shader_Program  = nullptr;

            std::string    m_GLSL_frag_code  = "";
        public:
            float bias                       = 0.6f;
            float focus                      = 2.0f;
            float blur_radius                = 3.0f;
            bool  dof                        = false;

            DepthOfField() = default;
            ~DepthOfField();

            const bool init_shaders();

            void pass(GBuffer&, const Viewport& viewport, const unsigned int sceneTexture, const Engine::priv::Renderer& renderer);

            static DepthOfField DOF;
    };
};
namespace Engine::Renderer::depthOfField {
    void enable(const bool b = true);
    void disable();
    const bool enabled();
    const float getFocus();
    void setFocus(const float);
    const float getBias();
    void setBias(const float);
    const float getBlurRadius();
    void setBlurRadius(const float);
};

#endif