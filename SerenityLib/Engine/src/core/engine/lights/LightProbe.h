#pragma once
#ifndef ENGINE_LIGHTS_LIGHT_PROBE_INCLUDE_GUARD
#define ENGINE_LIGHTS_LIGHT_PROBE_INCLUDE_GUARD

class  Scene;

#include <glm/vec3.hpp>

class LightProbe{
    public: struct ProbeType final { enum Type : unsigned int {
        Static,   //no updating probe
        Dynamic1, //update 1 cube side per frame
        Dynamic2, //update 2 cube sides per frame
        Dynamic3, //update 3 cube sides per frame
        Dynamic4, //update 4 cube sides per frame
        Dynamic5, //update 5 cube sides per frame
        Dynamic,  //update all cube sides per frame
    };};

    private:
        bool              m_Active;
        ProbeType::Type   m_ProbeType;
    public:
        LightProbe(
            const ProbeType::Type type,
            const glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
            Scene* scene = nullptr
        );
        virtual ~LightProbe();

        void setProbeType(const ProbeType::Type& type);
        const LightProbe::ProbeType::Type& getProbeType() const;

        void update(const float& dt);

        void activate(const bool = true);
        void deactivate();
};

#endif