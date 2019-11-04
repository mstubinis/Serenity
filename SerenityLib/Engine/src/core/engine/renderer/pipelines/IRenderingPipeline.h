#pragma once
#ifndef ENGINE_RENDERER_RENDERING_PIPELINE_H
#define ENGINE_RENDERER_RENDERING_PIPELINE_H

class Camera;
class SunLight;
class PointLight;
class DirectionalLight;
class SpotLight;
class RodLight;
namespace Engine {
    namespace epriv {
        class IRenderingPipeline {
            public:
                virtual ~IRenderingPipeline() = 0;

                virtual void update(const double& dt) = 0;
                virtual void render() = 0;

                virtual void renderSunLight(Camera& c, SunLight& s) = 0;
                virtual void renderPointLight(Camera& c, PointLight& p) = 0;
                virtual void renderDirectionalLight(Camera& c, DirectionalLight& d) = 0;
                virtual void renderSpotLight(Camera& c, SpotLight& s) = 0;
                virtual void renderRodLight(Camera& c, RodLight& r) = 0;


        };
    };
};

#endif