#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_HDR_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_HDR_H_INCLUDE_GUARD

class  ShaderProgram;
class  Shader;

struct HDRAlgorithm {enum Algorithm {
    None,
    Reinhard, 
    Filmic, 
    Exposure, 
    Uncharted,
};};

#include <string>

namespace Engine::priv {
    class  GBuffer;
    class  HDR final {
        private:
            Shader*          m_Vertex_Shader;
            Shader*          m_Fragment_Shader;
            ShaderProgram*   m_Shader_Program;

            std::string      m_GLSL_frag_code;
        public:
            bool                      hdr_active;
            float                     exposure;
            HDRAlgorithm::Algorithm   algorithm;

            HDR();
            ~HDR();

            const bool init_shaders();

            void pass(GBuffer&,const unsigned int& fboWidth,const unsigned int& fboHeight,const bool& godRays,const bool& lighting,const float& godRaysFactor);

            static HDR hdr;
    };
};
namespace Engine::Renderer::hdr {
    //const bool enabled();
    //void enable(const bool b = true);
    //void disable();
    const float getExposure();
    void setExposure(const float e);
    void setAlgorithm(const HDRAlgorithm::Algorithm a);
    const HDRAlgorithm::Algorithm getAlgorithm();
};



#endif