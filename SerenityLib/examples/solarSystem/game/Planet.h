#pragma once
#ifndef GAME_PLANET_H
#define GAME_PLANET_H

class  SunLight;
class  Ring;
class  Planet;
class  Map;

struct PlanetLogicFunctor;
struct PlanetaryRingModelInstanceBindFunctor;
struct AtmosphericScatteringGroundModelInstanceBindFunctor;
struct AtmosphericScatteringGroundModelInstanceUnbindFunctor;
struct AtmosphericScatteringSkyModelInstanceBindFunctor;
struct AtmosphericScatteringSkyModelInstanceUnbindFunctor;
struct StarModelInstanceBindFunctor;
struct StarModelInstanceUnbindFunctor;

#include <ecs/Components.h>
#include <ecs/EntityWrapper.h>
#include <core/engine/resources/Handle.h>
#include <unordered_map>

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

    RotationInfo(const RotationInfo& other)                = delete;
    RotationInfo& operator=(const RotationInfo& other)     = delete;
    RotationInfo(RotationInfo&& other) noexcept            = delete;
    RotationInfo& operator=(RotationInfo&& other) noexcept = delete;
};
class OrbitInfo final{
    public:
        glm::vec4    info; //x = eccentricity, y = days, z = minorRadius, w = majorRadius
        decimal      inclination;
        decimal      angle;
        Planet* parent;

        OrbitInfo(const float eccentricity, const float days, const float majorRadius, const decimal angle, Planet& parent, const decimal inclination = 0);

        OrbitInfo(const OrbitInfo& other)                = delete;
        OrbitInfo& operator=(const OrbitInfo& other)     = delete;
        OrbitInfo(OrbitInfo&& other) noexcept            = delete;
        OrbitInfo& operator=(OrbitInfo&& other) noexcept = delete;

        void setOrbitalPosition(const decimal angle, Planet& planet);
        const glm_vec3 getOrbitalPosition(const decimal angle, Planet& planet);
};
struct RingInfo final{
    uint       position;
    uint       size;
    uint       alphaBreakpoint;
    glm::ivec3 color;
    RingInfo(uint p, uint s, glm::ivec3 col,uint ab = 1){ 
        position = p;
        size = s; 
        color = col; 
        alphaBreakpoint = ab;
    }

    RingInfo(const RingInfo& other)                = delete;
    RingInfo& operator=(const RingInfo& other)     = delete;
    RingInfo(RingInfo&& other) noexcept            = default;
    RingInfo& operator=(RingInfo&& other) noexcept = default;
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

        const std::string getPlanetTypeNameAsString(const std::string& in_type_name) const;

    public:
        Planet(
            Handle& meshHandle,
            Handle& materialHandle,                     //Material
            const PlanetType::Type = PlanetType::Rocky, //Type
            const glm_vec3& = glm_vec3(0),               //Position
            const decimal = 1.0,                        //Radius
            const std::string& = "Planet",              //Name
            const float = 0,                            //Atmosphere size
            Map * = nullptr,
            const std::string& planet_type_name = ""
        );
        virtual ~Planet();

        Planet(const Planet& other)                = delete;
        Planet& operator=(const Planet& other)     = delete;
        Planet(Planet&& other) noexcept            = delete;
        Planet& operator=(Planet&& other) noexcept = delete;

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
            Handle& meshHandle,
            Handle& materialHandle,                     //Material
            const glm::vec3& starColor = glm::vec3(1.0f,1.0f,0.0f), //Star Color
            const glm::vec3& lightColor = glm::vec3(1.0f,1.0f,1.0f), //Sunlight Color
            const glm::vec3& godRaysColor = glm::vec3(1.0f,1.0f,1.0f), //GodRays Color
            const glm_vec3& = glm_vec3(0.0f),             //Position
            const decimal = 1.0,                         //Scale
            const std::string& = "Star",                  //Name
            Map * = nullptr,
            const std::string& star_type_name = ""
        );
        virtual ~Star();

        Star(const Star& other)                = delete;
        Star& operator=(const Star& other)     = delete;
        Star(Star&& other) noexcept            = delete;
        Star& operator=(Star&& other) noexcept = delete;
};
class Ring final{
    friend class ::Planet;
    private:
        Handle m_MaterialHandle;
        Planet* m_Parent;

        void internal_make_ring_image(const std::vector<RingInfo>& ring_list);
    public:
        Ring(std::vector<RingInfo>&, Planet*);
        ~Ring();

        Ring(const Ring& other)                = delete;
        Ring& operator=(const Ring& other)     = delete;
        Ring(Ring&& other) noexcept            = delete;
        Ring& operator=(Ring&& other) noexcept = delete;
};
#endif
