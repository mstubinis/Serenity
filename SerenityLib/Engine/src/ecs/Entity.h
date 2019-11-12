#pragma once
#ifndef ENGINE_ECS_ENTITY_H
#define ENGINE_ECS_ENTITY_H

#include <core/engine/scene/Scene.h>
#include <ecs/ECS.h>
#include <ecs/EntityDataRequest.h>

struct Entity{   
    uint data;

    Entity();
    Entity(const uint entityID, const uint sceneID, const uint versionID);
    ~Entity();
    void process(const uint entityID, const uint sceneID, const uint versionID);
    Entity(const Entity& other)                = default;
    Entity& operator=(const Entity& other)     = default;
    Entity(Entity&& other) noexcept            = default;
    Entity& operator=(Entity&& other) noexcept = default;

    const bool operator==(const Entity& other) const;
    const bool operator!=(const Entity& other) const;
    Scene& scene();
    void destroy();
    void move(const Scene& destination);
    const bool null();
    template<typename TComponent, typename... ARGS> inline TComponent* addComponent(ARGS&&... args) {
        auto& _this = *this;
        auto& ecs = Engine::epriv::InternalEntityPublicInterface::GetECS(_this);
        return ecs.addComponent<TComponent>(_this, std::forward<ARGS>(args)...);
    }
    template<typename TComponent, typename... ARGS> inline TComponent* addComponent(EntityDataRequest& request, ARGS&&... args) {
        auto& _this = *this;
        auto& ecs = Engine::epriv::InternalEntityPublicInterface::GetECS(_this);
        return ecs.addComponent<TComponent>(request, _this, std::forward<ARGS>(args)...);
    }
    template<typename TComponent> inline const bool removeComponent() {
        auto& _this = *this;
        auto& ecs = Engine::epriv::InternalEntityPublicInterface::GetECS(_this);
        return ecs.removeComponent<TComponent>(_this);
    }
    template<typename TComponent> inline TComponent* getComponent() {
        auto& _this = *this;
        auto& ecs = Engine::epriv::InternalEntityPublicInterface::GetECS(_this);
        return ecs.getComponent<TComponent>(_this);
    }
    template<typename TComponent> inline TComponent* getComponent(const EntityDataRequest& dataRequest) {
        auto& ecs = Engine::epriv::InternalEntityPublicInterface::GetECS(*this);
        return ecs.getComponent<TComponent>(dataRequest);
    }
    static Entity _null;
};



namespace Engine {
    namespace epriv {
        struct InternalEntityPublicInterface final {
            static ECS<Entity>& GetECS(Entity& entity) {
			    const EntityDataRequest dataRequest(entity);
                return Engine::epriv::InternalScenePublicInterface::GetECS(entity.scene());
            }
        };
    };
};

#endif