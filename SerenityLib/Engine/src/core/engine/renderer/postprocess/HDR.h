#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_HDR_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_HDR_H_INCLUDE_GUARD

class  ShaderP;

struct HDRAlgorithm {enum Algorithm {
    Reinhard, Filmic, Exposure, Uncharted,
};};

namespace Engine {
namespace epriv {
    class  GBuffer;
    class  Postprocess_HDR final {
        public:
            bool                      hdr;
            float                     exposure;
            HDRAlgorithm::Algorithm   algorithm;


            Postprocess_HDR();
            ~Postprocess_HDR();

            void init();
            void cleanup();

            void pass(
                ShaderP&,
                GBuffer&,
                const unsigned int& fboWidth,
                const unsigned int& fboHeight,
                const bool& godRays,
                const bool& lighting,
                const float& godRaysFactor
            );

            static Postprocess_HDR HDR;
    };
};
namespace Renderer {
namespace hdr {
    bool enabled();
    void enable(bool b = true);
    void disable();
    float getExposure();
    void setExposure(float e);
    void setAlgorithm(HDRAlgorithm::Algorithm a);
};
};
};



#endif