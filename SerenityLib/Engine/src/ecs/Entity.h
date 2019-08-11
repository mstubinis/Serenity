#pragma once
#ifndef ENGINE_ECS_ENTITY_H
#define ENGINE_ECS_ENTITY_H

#include <core/engine/Engine.h>
#include <core/engine/scene/Scene.h>
#include <ecs/ECS.h>
#include <ecs/EntityDataRequest.h>


const uint MAX_ENTITIES  = 2097152;
const uint MAX_SCENES    = 128;
const uint MAX_VERSIONS  = 16;

struct Entity{   
    uint data;
    inline Entity() { 
        data = 0; 
    }
    inline Entity(const uint entityID, const uint sceneID, const uint versionID) {
        process(entityID, sceneID, versionID);
    }
    ~Entity() {
        data = 0;
    }
    inline void process(const uint& entityID, const uint& sceneID, const uint& versionID) {
        data = versionID << 28 | sceneID << 21 | entityID;
    }
    Entity(const Entity& other) = default;
    Entity& operator=(const Entity& other) = default;
    Entity(Entity&& other) noexcept = default;
    Entity& operator=(Entity&& other) noexcept = default;

    const bool operator==(const Entity& other) const { 
        return (data == other.data) ? true : false; 
    }
    const bool operator!=(const Entity& other) const { 
        return (data == other.data) ? false : true; 
    }
    Scene& scene();
    void destroy();
    inline bool null() { 
        return data == 0 ? true : false; 
    }
    template<typename TComponent, typename... ARGS> inline TComponent* addComponent(ARGS&&... args) {
        auto& _this = *this;
        auto& ecs = Engine::epriv::InternalEntityPublicInterface::GetECS(_this);
        return ecs.addComponent<TComponent>(_this, std::forward<ARGS>(args)...);
    }
    template<typename TComponent> inline bool removeComponent() {
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
    void move(const Scene& destination);
    static Entity _null;
};

class EntityWrapper {
    protected:
        Entity m_Entity;
    public:
        inline EntityWrapper(Scene& scene) { 
            m_Entity = scene.createEntity(); 
        }
        virtual ~EntityWrapper() { 
            m_Entity = Entity::_null; 
        }
        inline void destroy() { 
            m_Entity.destroy();
        }
        inline Entity& entity() {
            return m_Entity; 
        }
        inline bool null() {
            return m_Entity.null();
        }
        template<typename TComponent, typename... ARGS> inline TComponent* addComponent(ARGS&& ... args) {
            return m_Entity.addComponent<TComponent>(std::forward<ARGS>(args)...);
        }
        template<typename TComponent> inline bool removeComponent() {
            return m_Entity.removeComponent<TComponent>();
        }
        template<typename TComponent> inline TComponent* getComponent() {
            return m_Entity.getComponent<TComponent>();
        }
        template<typename TComponent> inline TComponent* getComponent(const EntityDataRequest& dataRequest) {
            return m_Entity.getComponent<TComponent>(dataRequest);
        }
};

namespace Engine {
namespace epriv {
    struct InternalEntityPublicInterface final {
        static ECS<Entity>& GetECS(Entity& entity) {
			const EntityDataRequest dataRequest(entity);
			Scene& s = epriv::Core::m_Engine->m_ResourceManager._getSceneByID(dataRequest.sceneID);
            return Engine::epriv::InternalScenePublicInterface::GetECS(s);
        }
    };
};
};

#endif