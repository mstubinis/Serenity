#pragma once
#ifndef ENGINE_ECS_ENTITY_SERIALIZATION_H_INCLUDE_GUARD
#define ENGINE_ECS_ENTITY_SERIALIZATION_H_INCLUDE_GUARD

#include <cstdint>

typedef std::uint32_t uint;

struct Entity;
class Scene;
namespace Engine {
    namespace epriv {
        struct EntitySerialization final {
            uint        ID : 21;
            uint   sceneID : 7;
            uint versionID : 4;

            void serialize(const uint& _entityData);
            EntitySerialization(const uint& _entityData);
            EntitySerialization(const Entity& _entity);
            EntitySerialization(const EntitySerialization& _other) = delete;
            EntitySerialization& operator=(const EntitySerialization& _other) = delete;
            EntitySerialization(EntitySerialization&& _other) noexcept;
            EntitySerialization& operator=(EntitySerialization&& _other) noexcept;
            ~EntitySerialization();
        };
        struct EntityPOD final {
            uint        ID : 21;
            uint   sceneID : 7;
            uint versionID : 4;

            EntityPOD();
            EntityPOD(uint _id);
            EntityPOD(uint _id, Scene& _scene);
            EntityPOD(const EntityPOD& _other) = delete;
            EntityPOD& operator=(const EntityPOD& _other) = delete;
            EntityPOD(EntityPOD&& _other) noexcept;
            EntityPOD& operator=(EntityPOD&& _other) noexcept;
            ~EntityPOD();
        };
    };
};

#endif