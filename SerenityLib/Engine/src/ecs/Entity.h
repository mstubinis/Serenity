#pragma once
#ifndef ENGINE_ECS_ENTITY_H
#define ENGINE_ECS_ENTITY_H

#include <core/engine/scene/Scene.h>
#include <ecs/ECS.h>
#include <ecs/EntityDataRequest.h>

struct Entity{   
    std::uint32_t data = 0;

    Entity();
    Entity(const unsigned int& entityID, const unsigned int& sceneID, const unsigned int& versionID);
    ~Entity();

    Entity(const Entity& other);
    Entity& operator=(const Entity& other);
    Entity(Entity&& other) noexcept;
    Entity& operator=(Entity&& other) noexcept;

    const bool operator==(const Entity& other) const;
    const bool operator!=(const Entity& other) const;

    void process(const unsigned int& entityID, const unsigned int& sceneID, const unsigned int& versionID);

    Scene& scene() const;
    void destroy();
    void move(const Scene& destination);
    const bool null() const;
    template<typename T, typename... ARGS> inline void addComponent(ARGS&&... args) {
        auto& ecs = Engine::priv::InternalEntityPublicInterface::GetECS(*this);
        ecs.addComponent<T>(*this, std::forward<ARGS>(args)...);
    }
    template<typename T, typename... ARGS> inline void addComponent(EntityDataRequest& request, ARGS&&... args) {
        auto& ecs = Engine::priv::InternalEntityPublicInterface::GetECS(*this);
        ecs.addComponent<T>(request, *this, std::forward<ARGS>(args)...);
    }
    template<typename T> inline const bool removeComponent() {
        auto& ecs = Engine::priv::InternalEntityPublicInterface::GetECS(*this);
        return ecs.removeComponent<T>(*this);
    }
    template<typename T> inline T* getComponent() const {
        const auto& ecs = Engine::priv::InternalEntityPublicInterface::GetECS(*this);
        return ecs.getComponent<T>(*this);
    }
    template<typename T> inline T* getComponent(const EntityDataRequest& dataRequest) const {
        const auto& ecs = Engine::priv::InternalEntityPublicInterface::GetECS(*this);
        return ecs.getComponent<T>(dataRequest);
    }
    static Entity null_;
};



namespace Engine::priv {
    struct InternalEntityPublicInterface final {
        static ECS<Entity>& GetECS(const Entity& entity) {
			const EntityDataRequest dataRequest(entity);
            return Engine::priv::InternalScenePublicInterface::GetECS(entity.scene());
        }
    };
};

#endif