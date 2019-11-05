#pragma once
#ifndef GAME_PLANET_H
#define GAME_PLANET_H

#include <ecs/Components.h>

class SunLight;
class Ring;
class Planet;
class Map;

struct PlanetLogicFunctor;
struct PlanetaryRingModelInstanceBindFunctor;
struct AtmosphericScatteringGroundModelInstanceBindFunctor;
struct AtmosphericScatteringGroundModelInstanceUnbindFunctor;
struct AtmosphericScatteringSkyModelInstanceBindFunctor;
struct AtmosphericScatteringSkyModelInstanceUnbindFunctor;
struct StarModelInstanceBindFunctor;
struct StarModelInstanceUnbindFunctor;



struct PlanetType{ enum Type {
    Rocky,
    GasGiant,
    GasGiantRinged,
    Moon,
    Star,
    Asteroid,
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
        Planet* parent;
        OrbitInfo(float eccentricity, float days, float majorRadius,float angle, Planet& parent, float inclination = 0);
        ~OrbitInfo(){}
        void setOrbitalPosition(float angle, Planet& planet);
        glm::vec3 getOrbitalPosition(float angle, Planet& planet);
};
struct RingInfo final{
    uint position;
    uint size;
    uint alphaBreakpoint;
    glm::ivec3 color;
    RingInfo(uint p, uint s, glm::ivec3 col,uint ab = 1){ 
        position = p;
        size = s; 
        color = col; 
        alphaBreakpoint = ab;
    }
};
class Planets final {
    public:
        static std::unordered_map<PlanetType::Type, Handle> IconDatabase;
        static void init();
};

class Planet: public EntityWrapper {
    friend class  ::Ring;
    friend struct ::PlanetLogicFunctor;
    friend struct ::PlanetaryRingModelInstanceBindFunctor;
    friend struct ::AtmosphericScatteringGroundModelInstanceBindFunctor;
    friend struct ::AtmosphericScatteringGroundModelInstanceUnbindFunctor;
    friend struct ::AtmosphericScatteringSkyModelInstanceBindFunctor;
    friend struct ::AtmosphericScatteringSkyModelInstanceUnbindFunctor;
    friend struct ::StarModelInstanceBindFunctor;
    friend struct ::StarModelInstanceUnbindFunctor;
    private:
    protected:
        std::string         m_TypeName;
        std::vector<Ring*>  m_Rings;
        PlanetType::Type    m_Type;
        OrbitInfo*          m_OrbitInfo;
        RotationInfo*       m_RotationInfo;
        float               m_AtmosphereHeight;

        const std::string getPlanetTypeNameAsString() const;

    public:
        Planet(
            Handle& materialHandle,               //Material
            PlanetType::Type = PlanetType::Rocky, //Type
            glm_vec3 = glm_vec3(0),               //Position
            decimal = 1.0,                        //Radius
            std::string = "Planet",               //Name
            float = 0,                            //Atmosphere size
            Map * = nullptr,
            std::string planet_type_name = ""
        );
        virtual ~Planet();

        const std::string getName();
        const glm_vec3 getPosition();
        const glm::vec2 getGravityInfo();
        OrbitInfo* getOrbitInfo() const;
        const float getGroundRadius();
        const float getRadius();
        const float& getAtmosphereHeight() const;
        const PlanetType::Type& getType() const;
        const std::string& getTypeName() const;

        void setPosition(const decimal& x, const decimal& y, const decimal& z);
        void setPosition(const glm_vec3& pos);

        void addRing(Ring*);

        void setOrbit(OrbitInfo* o);
        void setRotation(RotationInfo* r);
};
class Star: public Planet{
    private:
        SunLight* m_Light;
    public:
        Star(
            glm::vec3 = glm::vec3(1.0f,1.0f,0.0f), //Star Color
            glm::vec3 = glm::vec3(1.0f,1.0f,1.0f), //Sunlight Color
            glm::vec3 = glm::vec3(1.0f,1.0f,1.0f), //GodRays Color
            glm_vec3 = glm_vec3(0.0f),             //Position
            decimal = 1.0,                         //Scale
            std::string = "Star",                  //Name
            Map * = nullptr,
            std::string star_type_name = ""
        );
        virtual ~Star();
};
class Ring final{
    friend class ::Planet;
    private:
        Handle m_MaterialHandle;
        Planet* m_Parent;
        void _makeRingImage(const std::vector<RingInfo>&);
    public:
        Ring(std::vector<RingInfo>&,Planet*);
        ~Ring();
};
#endif
