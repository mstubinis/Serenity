#pragma once
#ifndef ENGINE_ECS_ENTITY_WRAPPER_H
#define ENGINE_ECS_ENTITY_WRAPPER_H

#include <ecs/Entity.h>

class EntityWrapper {
    protected:
        Entity   m_Entity;

        EntityWrapper() = delete;
    public:
        EntityWrapper(Scene& scene);
        virtual ~EntityWrapper();

        EntityWrapper(const EntityWrapper& other) = delete;
        EntityWrapper& operator=(const EntityWrapper& other) = delete;
        EntityWrapper(EntityWrapper&& other) noexcept;
        EntityWrapper& operator=(EntityWrapper&& other) noexcept;

        virtual void destroy();
        const Entity& entity() const;
        const bool null() const;

        template<typename T, typename... ARGS> inline void addComponent(ARGS&& ... args) {
            m_Entity.addComponent<T>(std::forward<ARGS>(args)...);
        }
        template<typename T, typename... ARGS> inline void addComponent(EntityDataRequest& request, ARGS&& ... args) {
            m_Entity.addComponent<T>(request, std::forward<ARGS>(args)...);
        }
        template<typename T> inline const bool removeComponent() {
            return m_Entity.removeComponent<T>();
        }
        template<typename T> inline T* getComponent() const {
            return m_Entity.getComponent<T>();
        }
        template<typename T> inline T* getComponent(const EntityDataRequest& dataRequest) const {
            return m_Entity.getComponent<T>(dataRequest);
        }
};
#endif