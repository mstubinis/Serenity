#pragma once
#ifndef ENGINE_ECS_ENTITY_SERIALIZATION_H_INCLUDE_GUARD
#define ENGINE_ECS_ENTITY_SERIALIZATION_H_INCLUDE_GUARD

struct Entity;
class  Scene;
namespace Engine::priv {
    template<typename T> class ECSEntityPool;
};

#include <cstdint>

struct EntityDataRequest final : public Engine::NonCopyable {
    std::uint32_t        ID : 21; //2,097,152  max
    std::uint32_t   sceneID : 7;  //128        max
    std::uint32_t versionID : 4;  //16         max

    EntityDataRequest()                                          = delete;
    EntityDataRequest(const unsigned int& entityData);
    EntityDataRequest(const Entity& entity);
    ~EntityDataRequest();

    EntityDataRequest(EntityDataRequest&& other) noexcept;
    EntityDataRequest& operator=(EntityDataRequest&& other) noexcept;


    void serialize(const unsigned int& entityData);
};
namespace Engine::priv {
    class EntityPOD final : public Engine::NonCopyable {
        friend class Scene;
        friend class Engine::priv::ECSEntityPool<Entity>;
        private:
            std::uint32_t        ID : 21;
            std::uint32_t   sceneID : 7;
            std::uint32_t versionID : 4;

            EntityPOD() = delete;
        public:
            EntityPOD(const unsigned int& entityID, Scene& scene);
            EntityPOD(const unsigned int& entityID, const unsigned int& sceneID);
            ~EntityPOD();

            EntityPOD(EntityPOD&& other) noexcept;
            EntityPOD& operator=(EntityPOD&& other) noexcept;
    };
};

#endif