#pragma once
#ifndef ENGINE_ECS_ENTITY_SERIALIZATION_H_INCLUDE_GUARD
#define ENGINE_ECS_ENTITY_SERIALIZATION_H_INCLUDE_GUARD

class Entity;

namespace Engine::priv {
    template<typename T> class ECSEntityPool;
};

#include <ecs/ECSIncludes.h>

struct EntityDataRequest final : public Engine::NonCopyable {
    std::uint32_t        ID : ID_BIT_POSITIONS;
    std::uint32_t   sceneID : SCENE_BIT_POSITIONS;
    std::uint32_t versionID : VERSION_BIT_POSITIONS;

    EntityDataRequest()                                          = delete;
    EntityDataRequest(std::uint32_t entityData);
    EntityDataRequest(Entity entity);
    ~EntityDataRequest();

    EntityDataRequest(EntityDataRequest&& other) noexcept;
    EntityDataRequest& operator=(EntityDataRequest&& other) noexcept;

    void serialize(std::uint32_t entityData);
};

#endif