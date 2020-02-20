#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_DOF_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_DOF_H_INCLUDE_GUARD

class  ShaderProgram;
class  Shader;
class  Viewport;

#include <string>

namespace Engine::priv {
    class  GBuffer;
    class  DepthOfField final {
        private:
            Shader*        m_Vertex_Shader;
            Shader*        m_Fragment_Shader;
            ShaderProgram* m_Shader_Program;

            std::string    m_GLSL_frag_code;
        public:
            float bias;
            float focus;
            float blur_radius;
            bool  dof;

            DepthOfField();
            ~DepthOfField();

            const bool init_shaders();

            void pass(GBuffer&, const Viewport& viewport, const unsigned int& sceneTextureEnum);

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