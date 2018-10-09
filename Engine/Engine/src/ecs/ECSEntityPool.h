#pragma once
#ifndef ENGINE_ECS_ENTITY_POOL_H_INCLUDE_GUARD
#define ENGINE_ECS_ENTITY_POOL_H_INCLUDE_GUARD

#include "ecs/EntitySerialization.h"
#include "core/Scene.h"
#include <vector>

struct Entity;
namespace Engine {
    namespace epriv {
        class ECSEntityPool{
            private:
                std::vector<EntityPOD>    _pool;
                std::vector<uint>         _freelist;
            public:
                ECSEntityPool() = default;
                ~ECSEntityPool() = default;

                Entity& addEntity(Scene& _scene);
                bool removeEntity(const uint& _id);
                bool removeEntity(const Entity& _entity);
                EntityPOD* getEntity(const uint& _id);
                EntityPOD* getEntity(const Entity& _entity);
        };
    };
};
#endif