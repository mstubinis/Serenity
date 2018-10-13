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

            void serialize(const uint& _data);
            EntitySerialization(const uint& _data);
            EntitySerialization(const Entity& _input);
            EntitySerialization(const EntitySerialization& other) = delete;
            EntitySerialization& operator=(const EntitySerialization& other) = delete;
            EntitySerialization(EntitySerialization&& other) noexcept;
            EntitySerialization& operator=(EntitySerialization&& other) noexcept;
            ~EntitySerialization();
        };
        struct EntityPOD final {
            uint        ID : 21;
            uint   sceneID : 7;
            uint versionID : 4;

            EntityPOD();
            EntityPOD(uint _id);
            EntityPOD(uint _id, Scene& _scene);
            EntityPOD(const EntityPOD& other) = delete;
            EntityPOD& operator=(const EntityPOD& other) = delete;
            EntityPOD(EntityPOD&& other) noexcept;
            EntityPOD& operator=(EntityPOD&& other) noexcept;
            ~EntityPOD();
        };
    };
};

#endif