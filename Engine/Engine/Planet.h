#pragma once
#ifndef PLANET_H
#define PLANET_H

#include "Components.h"

class SunLight;
class Ring;
class Planet;

struct PlanetaryRingMeshInstanceBindFunctor;
struct AtmosphericScatteringGroundMeshInstanceBindFunctor;
struct AtmosphericScatteringGroundMeshInstanceUnbindFunctor;
struct AtmosphericScatteringSkyMeshInstanceBindFunctor;
struct AtmosphericScatteringSkyMeshInstanceUnbindFunctor;
struct StarMeshInstanceBindFunctor;

class PlanetType{ public:enum Type { 
    Rocky,Ice,GasGiant,IceGiant,Moon,Star,Asteroid,
};};

struct RotationInfo final{
    float tilt;
    float speed;
    RotationInfo(float _tilt, float _days){
        tilt = _tilt;
        speed = 1.0f / (_days * 86400.0f * 360.0f);
    }
};

class OrbitInfo final{
    public:
        glm::vec4 info; //x = eccentricity, y = days, z = minorRadius, w = majorRadius
        float inclination;
        float angle;
        uint parent;
        OrbitInfo(float eccentricity, float days, float majorRadius,float angle,uint parent,float inclination = 0);
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
    friend class ::Ring;
    friend struct ::PlanetaryRingMeshInstanceBindFunctor;
    friend struct ::AtmosphericScatteringGroundMeshInstanceBindFunctor;
    friend struct ::AtmosphericScatteringGroundMeshInstanceUnbindFunctor;
    friend struct ::AtmosphericScatteringSkyMeshInstanceBindFunctor;
	friend struct ::AtmosphericScatteringSkyMeshInstanceUnbindFunctor;
	friend struct ::StarMeshInstanceBindFunctor;
    protected:
        ComponentModel* m_Model;
        ComponentBody* m_Body;
        std::vector<Ring*> m_Rings;
        PlanetType::Type m_Type;
        OrbitInfo* m_OrbitInfo;
        RotationInfo* m_RotationInfo;
        float m_AtmosphereHeight;
    public:
        Planet(
            Handle& materialHandle,               //Material
            PlanetType::Type = PlanetType::Rocky, //Type
            glm::vec3 = glm::vec3(0),             //Position
            float = 1,                            //Radius
            std::string = "Planet",               //Name
            float = 0,                            //Atmosphere size
            Scene* = nullptr
        );
        virtual ~Planet();

        glm::vec3 getPosition();
        glm::vec2 getGravityInfo();
        OrbitInfo* getOrbitInfo() const;
        float getGroundRadius();
        float getRadius();
        float getAtmosphereHeight();

        void setPosition(float x,float y,float z);
        void setPosition(glm::vec3 pos);

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
            glm::vec3 = glm::vec3(1.0f,1.0f,0.0f),   //Star Color
            glm::vec3 = glm::vec3(1.0f,1.0f,1.0f),   //Sunlight Color
            glm::vec3 = glm::vec3(0.0f),             //Position
            float = 1,                               //Scale
            std::string = "Star",                    //Name
            Scene* = nullptr
        );
        virtual ~Star();
};

class Ring final{
    friend class ::Planet;
    private:
        Handle m_MaterialHandle;
        Planet* m_Parent;
        void _makeRingImage(std::vector<RingInfo>&,Planet*);
    public:
        Ring(std::vector<RingInfo>&,Planet*);
        ~Ring();
};
#endif
