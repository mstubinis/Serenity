#pragma once
#ifndef ENGINE_ECS_ENTITY_WRAPPER_H
#define ENGINE_ECS_ENTITY_WRAPPER_H

#include <ecs/Entity.h>

class EntityWrapper {
    protected:
        Entity m_Entity;
    public:
        EntityWrapper(Scene& scene);
        virtual ~EntityWrapper();

        EntityWrapper(const EntityWrapper& other);
        EntityWrapper& operator=(const EntityWrapper& other);
        EntityWrapper(EntityWrapper&& other) noexcept;
        EntityWrapper& operator=(EntityWrapper&& other) noexcept;

        virtual void destroy();
        Entity& entity();
        const bool null();
        template<typename TComponent, typename... ARGS> inline TComponent* addComponent(ARGS&& ... args) {
            return m_Entity.addComponent<TComponent>(std::forward<ARGS>(args)...);
        }
        template<typename TComponent, typename... ARGS> inline TComponent* addComponent(EntityDataRequest& request, ARGS&& ... args) {
            return m_Entity.addComponent<TComponent>(request, std::forward<ARGS>(args)...);
        }
        template<typename TComponent> inline const bool removeComponent() {
            return m_Entity.removeComponent<TComponent>();
        }
        template<typename TComponent> inline TComponent* getComponent() {
            return m_Entity.getComponent<TComponent>();
        }
        template<typename TComponent> inline TComponent* getComponent(const EntityDataRequest& dataRequest) {
            return m_Entity.getComponent<TComponent>(dataRequest);
        }
};

#endif