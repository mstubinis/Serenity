#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_HDR_H
#define ENGINE_RENDERER_POSTPROCESS_HDR_H

#include <serenity/core/engine/system/TypeDefs.h>

class  ShaderProgram;
class  Shader;
class  Viewport;

struct HDRAlgorithm { enum Algorithm : uint8_t {
    None = 0,
    Reinhard, 
    Filmic, 
    Exposure, 
    Uncharted,
};};

#include <serenity/core/engine/resources/Handle.h>
#include <string>

namespace Engine::priv {
    class  GBuffer;
    class  RenderModule;
    class  HDR final {
        private:
            Handle                    m_Vertex_Shader;
            Handle                    m_Fragment_Shader;
            Handle                    m_Shader_Program;

            std::string               m_GLSL_frag_code    = "";
        public:
            HDRAlgorithm::Algorithm   m_Algorithm  = HDRAlgorithm::Uncharted;
            float                     m_Exposure   = 3.0f;

            bool init_shaders();

            void pass(GBuffer&, const Viewport& viewport, uint32_t outTexture, uint32_t outTexture2, bool godRays, float godRaysFactor, const RenderModule& renderer);

            static HDR STATIC_HDR;
    };
};
namespace Engine::Renderer::hdr {
    inline float getExposure() noexcept { return Engine::priv::HDR::STATIC_HDR.m_Exposure; }
    inline void setExposure(float exposure) noexcept { Engine::priv::HDR::STATIC_HDR.m_Exposure = exposure; }
    inline void setAlgorithm(HDRAlgorithm::Algorithm algorithm) noexcept { Engine::priv::HDR::STATIC_HDR.m_Algorithm = algorithm; }
    inline HDRAlgorithm::Algorithm getAlgorithm() noexcept { return Engine::priv::HDR::STATIC_HDR.m_Algorithm; }
};



#endif