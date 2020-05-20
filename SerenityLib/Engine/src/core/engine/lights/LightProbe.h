#pragma once
#ifndef ENGINE_LIGHTS_LIGHT_PROBE_INCLUDE_GUARD
#define ENGINE_LIGHTS_LIGHT_PROBE_INCLUDE_GUARD

class  Scene;

#include <glm/vec3.hpp>
#include <ecs/Entity.h>
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
            const ProbeType::Type type,
            const glm::vec3& position   = glm::vec3(0.0f, 0.0f, 0.0f),
            Scene* scene = nullptr
        );
        virtual ~LightProbe();

        void addIgnoredEntity(const Entity entity);

        const Camera& getCamera() const;
        const Viewport& getViewport() const;

        void setProbeType(const ProbeType::Type type);
        LightProbe::ProbeType::Type getProbeType() const;

        void update(const float dt);

        void activate(const bool = true);
        void deactivate();
};

#endif