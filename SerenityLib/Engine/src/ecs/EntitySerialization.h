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
            inline void serialize(const uint& _entityData) {
                       ID = (_entityData & 0x003FFFFF) >> 0;
                  sceneID = (_entityData & 0x1FE00000) >> 21;
                versionID = (_entityData & 4026531840) >> 28;
            }
            EntitySerialization() = delete;                                                    //no default constructor
            EntitySerialization(const uint& _entityData);
            EntitySerialization(const Entity& _entity);
            EntitySerialization(const EntitySerialization& _other) = delete;                   //not copyable
            EntitySerialization& operator=(const EntitySerialization& _other) = delete;        //not copyable
            EntitySerialization(EntitySerialization&& _other) noexcept;
            EntitySerialization& operator=(EntitySerialization&& _other) noexcept;
            ~EntitySerialization() = default;
        };
        struct EntityPOD final {
            uint        ID : 21;
            uint   sceneID : 7;
            uint versionID : 4;

            EntityPOD() = delete;                                        //no default constructor
            EntityPOD(uint _id, Scene& _scene);
            EntityPOD(uint _id, uint _sceneID);
            EntityPOD(const EntityPOD& _other) = delete;                 //not copyable
            EntityPOD& operator=(const EntityPOD& _other) = delete;      //not copyable
            EntityPOD(EntityPOD&& _other) noexcept;
            EntityPOD& operator=(EntityPOD&& _other) noexcept;
            ~EntityPOD() = default;
        };
    };
};

#endif