#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_HDR_H
#define ENGINE_RENDERER_POSTPROCESS_HDR_H

class  ShaderProgram;
class  Shader;
class  Viewport;

struct HDRAlgorithm { enum Algorithm : unsigned char {
    None,
    Reinhard, 
    Filmic, 
    Exposure, 
    Uncharted,
};};

namespace Engine::priv {
    class  GBuffer;
    class  Renderer;
    class  HDR final {
        private:
            std::unique_ptr<Shader>          m_Vertex_Shader;
            std::unique_ptr<Shader>          m_Fragment_Shader;
            std::unique_ptr<ShaderProgram>   m_Shader_Program;

            std::string                      m_GLSL_frag_code    = "";

            HDR() = default;
            ~HDR() = default;
        public:
            bool                      hdr_active = true;
            float                     exposure   = 3.0f;
            HDRAlgorithm::Algorithm   algorithm  = HDRAlgorithm::Uncharted;

            bool init_shaders();

            void pass(GBuffer&, const Viewport& viewport, bool godRays, bool lighting, float godRaysFactor, const Engine::priv::Renderer& renderer);

            static HDR STATIC_HDR;
    };
};
namespace Engine::Renderer::hdr {
    float getExposure();
    void setExposure(float e);
    void setAlgorithm(HDRAlgorithm::Algorithm a);
    HDRAlgorithm::Algorithm getAlgorithm();
};



#endif