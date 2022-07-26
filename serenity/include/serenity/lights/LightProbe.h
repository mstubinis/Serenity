#pragma once
#ifndef ENGINE_LIGHTS_LIGHT_PROBE_H
#define ENGINE_LIGHTS_LIGHT_PROBE_H

class  Scene;
class  LightProbe;

#include <serenity/scene/Camera.h>
#include <serenity/scene/Viewport.h>

class LightProbe : public Entity {
    public: struct ProbeType final { enum Type : uint8_t {
        Static,   //no updating probe
        Dynamic1, //update 1 cube side per frame, MODERATELY EXPENSIVE
        Dynamic2, //update 2 cube sides per frame, MODERATELY EXPENSIVE
        Dynamic3, //update 3 cube sides per frame, MODERATELY EXPENSIVE
        Dynamic4, //update 4 cube sides per frame, MODERATELY EXPENSIVE
        Dynamic5, //update 5 cube sides per frame, MODERATELY EXPENSIVE
        Dynamic,  //update all cube sides per frame, MODERATELY EXPENSIVE

        Dynamic1WithPrefilter, //update 1 cube side per frame, VERY EXPENSIVE
        Dynamic2WithPrefilter, //update 2 cube sides per frame, VERY EXPENSIVE
        Dynamic3WithPrefilter, //update 3 cube sides per frame, VERY EXPENSIVE
        Dynamic4WithPrefilter, //update 4 cube sides per frame, VERY EXPENSIVE
        Dynamic5WithPrefilter, //update 5 cube sides per frame, VERY EXPENSIVE
        DynamicWithPrefilter,  //update all cube sides per frame, VERY EXPENSIVE

        Dynamic1WithPrefilterAndIrradiance, //update 1 cube side per frame, EXTREMELY EXPENSIVE
        Dynamic2WithPrefilterAndIrradiance, //update 2 cube sides per frame, EXTREMELY EXPENSIVE
        Dynamic3WithPrefilterAndIrradiance, //update 3 cube sides per frame, EXTREMELY EXPENSIVE
        Dynamic4WithPrefilterAndIrradiance, //update 4 cube sides per frame, EXTREMELY EXPENSIVE
        Dynamic5WithPrefilterAndIrradiance, //update 5 cube sides per frame, EXTREMELY EXPENSIVE
        DynamicWithPrefilterAndIrradiance,  //update all cube sides per frame, EXTREMELY EXPENSIVE
    };};

    private:
        bool                m_Active      = true;
        ProbeType::Type     m_ProbeType   = ProbeType::Static;
        uint8_t             m_FrameCount  = 0;
        Camera              m_Camera;
        std::vector<Entity> m_Ignored; //ignore these entities when rendering

        //these two are not owned in any way by the scene, they are simply used to fill in values for the rendering function
        Viewport            m_Viewport;
    public:
        LightProbe(Scene*, ProbeType::Type, const glm::vec3& position = glm::vec3{ 0.0f });
        ~LightProbe();

        void addIgnoredEntity(Entity);

        [[nodiscard]] inline const Camera& getCamera() const noexcept { return m_Camera; }
        [[nodiscard]] inline const Viewport& getViewport() const noexcept { return m_Viewport; }

        inline void setProbeType(ProbeType::Type type) noexcept { m_ProbeType = type; }
        [[nodiscard]] inline LightProbe::ProbeType::Type getProbeType() const noexcept { return m_ProbeType; }

        void update(const float dt);

        void activate(bool active = true) noexcept { m_Active = active; }
        void deactivate() noexcept { m_Active = false; }
};

#endif