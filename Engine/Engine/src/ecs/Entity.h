#pragma once
#ifndef ENGINE_ECS_ENTITY_H
#define ENGINE_ECS_ENTITY_H

#include "core/engine/Engine.h"
#include "core/Scene.h"
#include "ecs/ECS.h"
#include "ecs/EntitySerialization.h"

typedef std::uint32_t uint;

const uint MAX_ENTITIES = 2097152;
const uint MAX_SCENES = 128;
const uint MAX_VERSIONS = 16;

class Scene;
struct Entity{   
    uint data;

    Entity() { data = 0; }
    Entity(uint _id, uint _sceneID, uint _versionID) { serialize(_id, _sceneID, _versionID); }

    void serialize(const uint& _id, const uint& _sceneID, const uint& _versionID) { data = _versionID << 28 | _sceneID << 21 | _id; }

    Entity(const Entity& other) = default;
    Entity& operator=(const Entity& other) = default;
    Entity(Entity&& other) noexcept = default;
    Entity& operator=(Entity&& other) noexcept = default;

    ~Entity() { data = 0; }

    Scene& scene();
    void destroy();
    bool null();
    template<typename TComponent, typename... ARGS> TComponent* addComponent(ARGS&&... _args) {
        auto& _this = *this; auto& _ecs = Engine::epriv::InternalEntityPublicInterface::GetECS(_this);
        return _ecs.addComponent<TComponent>(_this, std::forward<ARGS>(_args)...);
    }
    template<typename TComponent> bool removeComponent() {
        auto& _this = *this; auto& _ecs = Engine::epriv::InternalEntityPublicInterface::GetECS(_this);
        return _ecs.removeComponent<TComponent>(_this);
    }
    template<typename TComponent> TComponent* getComponent() {
        auto& _this = *this; auto& _ecs = Engine::epriv::InternalEntityPublicInterface::GetECS(_this);
        return _ecs.getComponent<TComponent>(_this);
    }
    void move(Scene& destination);
    static Entity _null;
};

namespace Engine {
    namespace epriv {
        struct InternalEntityPublicInterface final {
            static ECS<Entity>& GetECS(Entity& _entity) {
                EntitySerialization _s(_entity);
                Scene& s = epriv::Core::m_Engine->m_ResourceManager._getSceneByID(_s.sceneID);
                return Engine::epriv::InternalScenePublicInterface::GetECS(s);
            }
        };
    };
};

#endif