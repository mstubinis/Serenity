#pragma once
#ifndef ENGINE_LIGHTS_LIGHT_PROBE_H
#define ENGINE_LIGHTS_LIGHT_PROBE_H

class  Scene;

#include <glm/vec3.hpp>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Viewport.h>

class LightProbe : public Entity, public Engine::NonCopyable, public Engine::NonMoveable {
    public: struct ProbeType final { enum Type : unsigned char {
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
        unsigned char       m_FrameCount  = 0_uc;
        Camera              m_Camera;
        std::vector<Entity> m_Ignored; //ignore these entities when rendering

        //these two are not owned in any way by the scene, they are simply used to fill in values for the rendering function
        Viewport            m_Viewport;
    public:
        LightProbe(
            ProbeType::Type type,
            const glm::vec3& position   = glm::vec3(0.0f, 0.0f, 0.0f),
            Scene* scene = nullptr
        );
        virtual ~LightProbe();

        void addIgnoredEntity(Entity entity);

        constexpr const Camera& getCamera() const noexcept { return m_Camera; }
        constexpr const Viewport& getViewport() const noexcept { return m_Viewport; }

        void setProbeType(ProbeType::Type type) noexcept { m_ProbeType = type; }
        constexpr LightProbe::ProbeType::Type getProbeType() const noexcept { return m_ProbeType; }

        void update(const float dt);

        void activate(bool active = true) noexcept { m_Active = active; }
        void deactivate() noexcept { m_Active = false; }
};

#endif