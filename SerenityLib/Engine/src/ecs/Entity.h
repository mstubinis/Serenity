#pragma once
#ifndef ENGINE_ECS_ENTITY_H
#define ENGINE_ECS_ENTITY_H

#include <core/engine/scene/Scene.h>
#include <ecs/ECS.h>
#include <ecs/EntityDataRequest.h>

struct Entity{   
    std::uint32_t data;

    Entity();
    Entity(const unsigned int& entityID, const unsigned int& sceneID, const unsigned int& versionID);
    ~Entity();
    void process(const unsigned int& entityID, const unsigned int& sceneID, const unsigned int& versionID);
    Entity(const Entity& other)                = default;
    Entity& operator=(const Entity& other)     = default;
    Entity(Entity&& other) noexcept            = default;
    Entity& operator=(Entity&& other) noexcept = default;

    const bool operator==(const Entity& other) const;
    const bool operator!=(const Entity& other) const;
    Scene& scene() const;
    void destroy();
    void move(const Scene& destination);
    const bool null() const;
    template<typename TComponent, typename... ARGS> 
    TComponent* addComponent(ARGS&&... args) {
        auto& _this = *this;
        auto& ecs = Engine::priv::InternalEntityPublicInterface::GetECS(_this);
        return ecs.addComponent<TComponent>(_this, std::forward<ARGS>(args)...);
    }
    template<typename TComponent, typename... ARGS> 
    TComponent* addComponent(EntityDataRequest& request, ARGS&&... args) {
        auto& _this = *this;
        auto& ecs = Engine::priv::InternalEntityPublicInterface::GetECS(_this);
        return ecs.addComponent<TComponent>(request, _this, std::forward<ARGS>(args)...);
    }
    template<typename TComponent> 
    const bool removeComponent() {
        auto& _this = *this;
        auto& ecs = Engine::priv::InternalEntityPublicInterface::GetECS(_this);
        return ecs.removeComponent<TComponent>(_this);
    }
    template<typename TComponent> 
    TComponent* getComponent() const {
        auto& _this = *this;
        auto& ecs = Engine::priv::InternalEntityPublicInterface::GetECS(_this);
        return ecs.getComponent<TComponent>(_this);
    }
    template<typename TComponent> 
    TComponent* getComponent(const EntityDataRequest& dataRequest) const {
        auto& ecs = Engine::priv::InternalEntityPublicInterface::GetECS(*this);
        return ecs.getComponent<TComponent>(dataRequest);
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