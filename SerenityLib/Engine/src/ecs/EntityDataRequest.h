#pragma once
#ifndef ENGINE_ECS_ENTITY_SERIALIZATION_H_INCLUDE_GUARD
#define ENGINE_ECS_ENTITY_SERIALIZATION_H_INCLUDE_GUARD

#include <cstdint>
#include <core/engine/Engine_Utils.h>


struct Entity;
class Scene;

struct EntityDataRequest final {
    uint        ID : 21;
    uint   sceneID : 7;
    uint versionID : 4;
    inline void serialize(const uint& _entityData) {
        ID = (_entityData & 0x003FFFFF) >> 0;
        sceneID = (_entityData & 0x1FE00000) >> 21;
        versionID = (_entityData & 4026531840) >> 28;
    }
    EntityDataRequest() = delete;
    EntityDataRequest(const uint& _entityData);
    EntityDataRequest(const Entity& _entity);
    EntityDataRequest(const EntityDataRequest& _other) = delete;
    EntityDataRequest& operator=(const EntityDataRequest& _other) = delete;
    EntityDataRequest(EntityDataRequest&& _other) noexcept;
    EntityDataRequest& operator=(EntityDataRequest&& _other) noexcept;
    ~EntityDataRequest() = default;
};

namespace Engine {
namespace epriv {
    struct EntityPOD final {
        uint        ID : 21;
        uint   sceneID : 7;
        uint versionID : 4;

        EntityPOD() = delete;
        EntityPOD(uint _id, Scene& _scene);
        EntityPOD(uint _id, uint _sceneID);
        EntityPOD(const EntityPOD& _other) = delete;
        EntityPOD& operator=(const EntityPOD& _other) = delete;
        EntityPOD(EntityPOD&& _other) noexcept;
        EntityPOD& operator=(EntityPOD&& _other) noexcept;
        ~EntityPOD() = default;
    };
};
};

#endif