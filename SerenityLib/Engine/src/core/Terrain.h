#pragma once
#ifndef ENGINE_TERRAIN_H
#define ENINGE_TERRAIN_H

namespace sf{ 
    class Image;
}
class  Scene;
class  btRigidBody;
class  Handle;
class  btHeightfieldTerrainShape;
class  Terrain;

#include <glm/vec3.hpp>
#include <string>
#include <ecs/Entity.h>

class Terrain: public EventObserver, public Entity {
/*
    struct Axis final { enum Type : unsigned char {
        Up,
        Down,
        Left,
        Right,
        Front,
        Back
    };};
*/

    class Data {
        friend class Terrain;
        private:
            float                           m_HeightScale        = 1.0f;
            float                           m_MinHeight          = 0.0f;
            float                           m_MaxHeight          = 1.0f;
            std::vector<std::vector<float>> m_Data;
            btHeightfieldTerrainShape*      m_BtHeightfieldShape = nullptr;

            btHeightfieldTerrainShape* generate_bt_shape();
            void calculate_data(sf::Image& heightmapImage);

            void subdivide(unsigned int levels);
        public:
            Data() = default;
    };

    private:
        Mesh*                        m_Mesh               = nullptr;
        Data                         m_TerrainData;
    public:
        Terrain(const std::string& name, sf::Image& heightmapImage, Handle& material, Scene* scene = nullptr);
        virtual ~Terrain();

        void subdivide(unsigned int levels = 1);

        virtual void setPosition(float x, float y, float z); 
        virtual void setPosition(glm::vec3 position);

        virtual void setScale(float x, float y, float z); 
        virtual void setScale(glm::vec3 scale);

        virtual void update(const float dt);
};
#endif