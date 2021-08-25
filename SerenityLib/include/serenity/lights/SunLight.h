#pragma once
#ifndef ENGINE_LIGHT_SUN_H
#define ENGINE_LIGHT_SUN_H

class Scene;
class SunLight;
namespace Engine::priv {
    class RenderModule;
};

#include <serenity/lights/LightIncludes.h>
#include <serenity/ecs/entity/EntityBody.h>

class SunLightShadowData {

};

class SunLight : public EntityBody, public LightBaseData<SunLight> {
    friend class ::Engine::priv::RenderModule;
    public:
        SunLight() = delete;
        SunLight(Scene*, const glm_vec3& position);
        SunLight(Scene*, decimal x, decimal y, decimal z);

        bool setShadowCaster(bool castsShadow) noexcept;

};
#endif