#pragma once
#ifndef ENGINE_TERRAIN_H
#define ENINGE_TERRAIN_H

#include <ecs/Components.h>
#include <SFML/System.hpp>
#include <core/engine/events/Engine_EventObject.h>

namespace sf{ 
    class Image;
}
class Scene;
class btRigidBody;

class Terrain: public EventObserver, public EntityWrapper {
    private:
        std::vector<float> m_Pixels;
    public:
        Terrain(const std::string& name,sf::Image& heightmapImage,Handle& material,Scene* scene = nullptr);
        virtual ~Terrain();

        virtual void setPosition(float,float,float); 
        virtual void setPosition(glm::vec3);

        virtual void setScale(float,float,float); 
        virtual void setScale(glm::vec3);

        virtual void update(float);
};
#endif