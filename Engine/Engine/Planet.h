#pragma once
#ifndef PLANET_H
#define PLANET_H

#include "Components.h"

class SunLight;
class Ring;
class Planet;

struct AtmosphericScatteringMeshInstanceBindFunctor;

enum PlanetType { 
    PLANET_TYPE_ROCKY, 
    PLANET_TYPE_ICE,
    PLANET_TYPE_GAS_GIANT, 
    PLANET_TYPE_ICE_GIANT,
    PLANET_TYPE_MOON, 
    PLANET_TYPE_STAR, 
    PLANET_TYPE_ASTEROID,
};

struct RotationInfo final{
    float tilt;
    float days;
    RotationInfo(float _tilt, float _days){
        tilt = _tilt;
        days = _days;
    }
};

class OrbitInfo final{
    public:
        float eccentricity;
        float days;
        float minorRadius; float majorRadius;
        float angle;
        uint parent;
        OrbitInfo(float eccentricity, float days, float majorRadius,float angle,uint parent);
        ~OrbitInfo(){}
        void setOrbitalPosition(float angle,Planet* planet);
        glm::vec3 getOrbitalPosition(float angle,Planet* planet);
};

struct RingInfo final{
    uint position;
    uint size;
    uint alphaBreakpoint;
    glm::uvec3 color;
    RingInfo(uint p, uint s, glm::uvec3 col,uint ab = 1){ 
        position = p;
        size = s; 
        color = col; 
        alphaBreakpoint = ab;
    }
};

class Planet: public Entity{
    friend struct ::AtmosphericScatteringMeshInstanceBindFunctor;
    protected:
        ComponentModel* m_Model;
        ComponentBasicBody* m_Body;
        std::vector<Ring*> m_Rings;
        PlanetType m_Type;
        OrbitInfo* m_OrbitInfo;
        RotationInfo* m_RotationInfo;
        float m_AtmosphereHeight;
    public:
        Planet(
            Handle& materialHandle,         //Material
            PlanetType = PLANET_TYPE_ROCKY, //Type
            glm::vec3 = glm::vec3(0),       //Position
            float = 1,                      //Radius
            std::string = "Planet",         //Name
            float = 0,                      //Atmosphere size
            Scene* = nullptr
        );
        virtual ~Planet();

        glm::vec3 getPosition();
        glm::vec2 getGravityInfo();
        OrbitInfo* getOrbitInfo() const;
        float getDefaultRadius();
        float getRadius();
        float getAtmosphereHeight();

        void setPosition(float x,float y,float z);
        void setPosition(glm::vec3& pos);

        void addRing(Ring*);

        void setOrbit(OrbitInfo* o);
        void setRotation(RotationInfo* r);

        void update(const float& dt);
};

class Star: public Planet{
    private:
        SunLight* m_Light;
    public:
        Star(
            glm::vec3 = glm::vec3(1,1,0),   //Star Color
            glm::vec3 = glm::vec3(1,1,1),   //Sunlight Color
            glm::vec3 = glm::vec3(0),       //Position
            float = 1,                      //Scale
            std::string = "Star",           //Name
            Scene* = nullptr
        );
        virtual ~Star();
};

class Ring final{
    private:
        Material* material;
        Planet* m_Parent;
        void _makeRingImage(std::vector<RingInfo>&,Planet*);
    public:
        Ring(std::vector<RingInfo>&,Planet*);
        ~Ring();
};
#endif
