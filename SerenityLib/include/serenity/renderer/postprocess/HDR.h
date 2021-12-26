#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_HDR_H
#define ENGINE_RENDERER_POSTPROCESS_HDR_H

#include <serenity/system/TypeDefs.h>
#include <serenity/system/Macros.h>

class  ShaderProgram;
class  Shader;
class  Viewport;

class HDRAlgorithm { 
    public:
        enum Type : uint8_t {
            None = 0,
            Reinhard, 
            Filmic, 
            Exposure, 
            Uncharted,
        };
        BUILD_ENUM_CLASS_MEMBERS(HDRAlgorithm, Type)
};

#include <serenity/resources/Handle.h>
#include <string>

namespace Engine::priv {
    class  GBuffer;
    class  RenderModule;
    class  HDR final {
        private:
            Handle                    m_Vertex_Shader;
            Handle                    m_Fragment_Shader;
            Handle                    m_Shader_Program;

            std::string               m_GLSL_frag_code;

            void internal_init_fragment_code();
        public:
            HDRAlgorithm              m_Algorithm  = HDRAlgorithm::Uncharted;
            float                     m_Exposure   = 3.0f;

            static bool init();

            void pass(GBuffer&, const Viewport&, uint32_t outTexture, uint32_t outTexture2, bool godRays, float godRaysFactor, const RenderModule&);

            static HDR STATIC_HDR;
    };
};
namespace Engine::Renderer::hdr {
    [[nodiscard]] inline float getExposure() noexcept { return Engine::priv::HDR::STATIC_HDR.m_Exposure; }
    inline void setExposure(float exposure) noexcept { Engine::priv::HDR::STATIC_HDR.m_Exposure = exposure; }
    inline void setAlgorithm(HDRAlgorithm algorithm) noexcept { Engine::priv::HDR::STATIC_HDR.m_Algorithm = algorithm; }
    [[nodiscard]] inline HDRAlgorithm getAlgorithm() noexcept { return Engine::priv::HDR::STATIC_HDR.m_Algorithm; }
};



#endif