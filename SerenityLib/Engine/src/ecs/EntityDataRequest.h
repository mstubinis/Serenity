#pragma once
#ifndef ENGINE_ECS_ENTITY_SERIALIZATION_H_INCLUDE_GUARD
#define ENGINE_ECS_ENTITY_SERIALIZATION_H_INCLUDE_GUARD

#include <core/engine/utils/Utils.h>

struct Entity;
class  Scene;
struct EntityDataRequest final {
    uint        ID : 21; //2,097,152  max
    uint   sceneID : 7;  //128        max
    uint versionID : 4;  //16         max
    EntityDataRequest()                                          = delete;
    EntityDataRequest(const uint& entityData);
    EntityDataRequest(const Entity& entity);
    EntityDataRequest(const EntityDataRequest& other)            = delete;
    EntityDataRequest& operator=(const EntityDataRequest& other) = delete;
    EntityDataRequest(EntityDataRequest&& other) noexcept;
    EntityDataRequest& operator=(EntityDataRequest&& other) noexcept;
    ~EntityDataRequest()                                         = default;

    void serialize(const uint& entityData);
};
namespace Engine {
    namespace epriv {
        struct EntityPOD final {
            uint        ID : 21;
            uint   sceneID : 7;
            uint versionID : 4;

            EntityPOD()                                  = delete;
            EntityPOD(const uint& entityID, Scene& scene);
            EntityPOD(const uint& entityID, const uint& sceneID);
            EntityPOD(const EntityPOD& other)            = delete;
            EntityPOD& operator=(const EntityPOD& other) = delete;
            EntityPOD(EntityPOD&& other) noexcept;
            EntityPOD& operator=(EntityPOD&& other) noexcept;
            ~EntityPOD()                                 = default;
        };
    };
};

#endif