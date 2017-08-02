#pragma once
#ifndef PLANET_H
#define PLANET_H

#include "ObjectDisplay.h"

class SunLight;
class Ring;

enum PlanetType { 
    PLANET_TYPE_ROCKY, 
    PLANET_TYPE_ICE,
    PLANET_TYPE_GAS_GIANT, 
    PLANET_TYPE_ICE_GIANT,
    PLANET_TYPE_MOON, 
    PLANET_TYPE_STAR, 
    PLANET_TYPE_ASTEROID 
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
        boost::weak_ptr<Object> parent;
        OrbitInfo(float eccentricity, float days, float majorRadius,float angle,std::string parent);
        ~OrbitInfo(){}
        void setOrbitalPosition(float angle,Object* thisPlanet);
        glm::vec3 getOrbitalPosition(float angle,Object* thisPlanet);
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

class Planet: public ObjectDisplay{
    protected:
        std::vector<Ring*> m_Rings;
        PlanetType m_Type;
        OrbitInfo* m_OrbitInfo;
        RotationInfo* m_RotationInfo;
        float m_AtmosphereHeight;
    public:
        Planet(
            std::string = "",               //Material
            PlanetType = PLANET_TYPE_ROCKY, //Type
            glm::vec3 = glm::vec3(0),       //Position
            float = 1,                      //Radius
            std::string = "Planet",         //Name
            float = 0,                      //Atmosphere size
            Scene* = nullptr
        );
        virtual ~Planet();

        glm::vec2 getGravityInfo(){ return glm::vec2(getRadius()*5,getRadius()*7); }

        OrbitInfo* getOrbitInfo() const { return m_OrbitInfo; }
        const glm::vec3& getRadiusBox() const { return m_BoundingBoxRadius + (m_BoundingBoxRadius*m_AtmosphereHeight); }
        float getDefaultRadius(){ return m_Radius; }
        float getRadius() { return m_Radius + (m_Radius * m_AtmosphereHeight); }
        float getAtmosphereHeight(){ return m_AtmosphereHeight; }

        void addRing(Ring*);

        void setOrbit(OrbitInfo* o){ 
            m_OrbitInfo = o; 
            update(0);
        }
        void setRotation(RotationInfo* r){ 
            m_RotationInfo = r;
            rotate(0,0,-m_RotationInfo->tilt,false);
        }

        void update(float);
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

        void update(float);
        void draw(GLuint shader);
};
#endif
