#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_DOF_H
#define ENGINE_RENDERER_POSTPROCESS_DOF_H

class  ShaderProgram;
class  Shader;
class  Viewport;

#include <core/engine/resources/Handle.h>

namespace Engine::priv {
    class  GBuffer;
    class  RenderModule;
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

            bool init_shaders();

            void pass(GBuffer&, const Viewport& viewport, uint32_t sceneTexture, const Engine::priv::RenderModule& renderer);

            static DepthOfField STATIC_DOF;
    };
};
namespace Engine::Renderer::depthOfField {
    inline void enable(bool enabled = true) noexcept { Engine::priv::DepthOfField::STATIC_DOF.dof = enabled; }
    inline void disable() noexcept { Engine::priv::DepthOfField::STATIC_DOF.dof = false; }
    inline bool enabled() noexcept { return Engine::priv::DepthOfField::STATIC_DOF.dof; }
    inline float getFocus() noexcept { return Engine::priv::DepthOfField::STATIC_DOF.focus; }
    inline void setFocus(float focus) noexcept { Engine::priv::DepthOfField::STATIC_DOF.focus = glm::max(0.0f, focus); }
    inline float getBias() noexcept { return Engine::priv::DepthOfField::STATIC_DOF.bias; }
    inline void setBias(float bias) noexcept { Engine::priv::DepthOfField::STATIC_DOF.bias = bias; }
    inline float getBlurRadius() noexcept { return Engine::priv::DepthOfField::STATIC_DOF.blur_radius; }
    inline void setBlurRadius(float blurRadius) noexcept { Engine::priv::DepthOfField::STATIC_DOF.blur_radius = glm::max(0.0f, blurRadius); }
};

#endif