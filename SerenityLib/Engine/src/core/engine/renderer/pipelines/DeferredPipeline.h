#pragma once
#ifndef ENGINE_RENDERER_DEFERRED_PIPELINE_H
#define ENGINE_RENDERER_DEFERRED_PIPELINE_H

#include <core/engine/renderer/pipelines/IRenderingPipeline.h>

namespace Engine {
    namespace epriv {
        class DeferredPipeline {
            public:
                DeferredPipeline();
                ~DeferredPipeline();

                void update(const double& dt);
                void render();

                void renderSunLight(Camera& c, SunLight& s);
                void renderPointLight(Camera& c, PointLight& p);
                void renderDirectionalLight(Camera& c, DirectionalLight& d);
                void renderSpotLight(Camera& c, SpotLight& s);
                void renderRodLight(Camera& c, RodLight& r);
        };
    };
};

#endif