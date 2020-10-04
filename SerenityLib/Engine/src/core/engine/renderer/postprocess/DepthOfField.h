#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_DOF_H
#define ENGINE_RENDERER_POSTPROCESS_DOF_H

class  ShaderProgram;
class  Shader;
class  Viewport;

#include <core/engine/resources/Handle.h>

namespace Engine::priv {
    class  GBuffer;
    class  Renderer;
    class  DepthOfField final {
        private:
            Handle       m_Vertex_Shader;
            Handle       m_Fragment_Shader;
            Handle       m_Shader_Program;

            std::string  m_GLSL_frag_code  = "";
        public:
            float bias                       = 0.6f;
            float focus                      = 2.0f;
            float blur_radius                = 3.0f;
            bool  dof                        = false;

            DepthOfField() = default;
            ~DepthOfField() = default;

            bool init_shaders();

            void pass(GBuffer&, const Viewport& viewport, unsigned int sceneTexture, const Engine::priv::Renderer& renderer);

            static DepthOfField STATIC_DOF;
    };
};
namespace Engine::Renderer::depthOfField {
    void enable(bool enabled = true);
    void disable();
    bool enabled();
    float getFocus();
    void setFocus(float focus);
    float getBias();
    void setBias(float bias);
    float getBlurRadius();
    void setBlurRadius(float blurRadius);
};

#endif