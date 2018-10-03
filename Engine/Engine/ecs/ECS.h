#pragma once
#ifndef ENGINE_ECS_H
#define ENGINE_ECS_H

#include "Entity.h"
#include <algorithm> //std::swap (until C++11)
#include <utility>   //std::swap (since C++11)
#include <vector>

//per scene basis
//the hub of the ECS system. have a pool for entities here as well as the component pools here too
class Scene;

namespace Engine {
    namespace epriv {
        class ECSEntityPool final {
            private:
                uint lastIndex;
                std::vector<Entity> pool;
            public:
                ECSEntityPool();
                ~ECSEntityPool();

                Entity* getEntity(uint _entityID);         //retrieve the entity object from the scene using the entityID       
                Entity* createEntity(Scene& _scene);       //construct an entity object for the parameter scene and add it to the scene
                void    addEntity(const Entity& _entity);  //adds an already created entity object to the scene.

                void removeEntity(uint _entityID);    //remove entity from the scene based on its id
                void removeEntity(Entity& _entity);   //remove entity from the scene

                void moveEntity(ECSEntityPool& other, uint _entityID);  //moves the entity corresponding to the parameter ID to another ECSEntityPool in a different scene
                void moveEntity(ECSEntityPool& other, Entity& _entity); //moves the entity to another ECSEntityPool in a different scene
        };
        class ECS final {
            private:
                ECSEntityPool entityPool;
            public:
                ECS();
                ~ECS();
        };
    };
};

#endif