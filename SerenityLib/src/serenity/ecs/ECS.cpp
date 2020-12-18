#include <serenity/ecs/ECS.h>

Engine::priv::ECS::~ECS() {
    SAFE_DELETE_VECTOR(m_ComponentPools);
    SAFE_DELETE_VECTOR(m_Systems);
}

Entity Engine::priv::ECS::createEntity(Scene& scene) {
    Entity entity = m_EntityPool.addEntity(scene);

#ifndef ENGINE_PRODUCTION
    for (auto e : m_JustAddedEntities) {
        if (e == entity) {
            ASSERT(false, __FUNCTION__ << "(): " << entity.m_Data << " already in m_JustAddedEntities!");
        }
    }
#endif

    m_JustAddedEntities.emplace_back(entity);
    return entity;
}

void Engine::priv::ECS::init(const SceneOptions& options) {
    m_SceneOptions = options;
    m_EntityPool.init(m_SceneOptions);
}

void Engine::priv::ECS::clearAllEntities() noexcept {
    m_JustAddedEntities.clear();
    m_DestroyedEntities.clear();
    m_EntityPool.clear();
    for (auto& pool : m_ComponentPools) {
        pool->clear();
    }
}

void Engine::priv::ECS::removeEntity(Entity entity) {
#ifndef ENGINE_PRODUCTION
    for (auto e : m_DestroyedEntities) {
        if (e == entity) {
            ASSERT(false, __FUNCTION__ << "(): " << entity.m_Data << " already in m_DestroyedEntities!");
        }
    }
#endif


    m_DestroyedEntities.emplace_back(entity);
}
void Engine::priv::ECS::update(const float dt, Scene& scene) {
    for (size_t i = 0; i < m_Systems.size(); ++i) {
        m_Systems[i]->onUpdate(dt, scene);
    }
}
void Engine::priv::ECS::onComponentAddedToEntity(void* component, Entity entity, uint32_t type_slot) {
    m_Systems[type_slot]->onComponentAddedToEntity(component, entity);
}
void Engine::priv::ECS::onComponentRemovedFromEntity(Entity entity, uint32_t type_slot) {
    m_Systems[type_slot]->onComponentRemovedFromEntity(entity);
}
void Engine::priv::ECS::onSceneEntered(Scene& scene) {
    for (size_t i = 0; i < m_Systems.size(); ++i) {
        m_Systems[i]->onSceneEntered(scene);
    }
}
void Engine::priv::ECS::onSceneLeft(Scene& scene) {
    for (size_t i = 0; i < m_Systems.size(); ++i) {
        m_Systems[i]->onSceneLeft(scene);
    }
}
void Engine::priv::ECS::preUpdate(Scene& scene, const float dt) {
    if (m_JustAddedEntities.size() > 0) {
        for (size_t i = 0; i < m_Systems.size(); ++i) {
            for (size_t j = 0; j < m_JustAddedEntities.size(); ++j) {
                m_Systems[i]->onEntityAddedToScene(m_JustAddedEntities[j], scene);
            }
        }
        m_JustAddedEntities.clear();
    }
}
void Engine::priv::ECS::postUpdate(Scene& scene, const float dt) {
    if (m_DestroyedEntities.size() > 0) {
        for (const auto entity : m_DestroyedEntities) {
            const auto id = entity.id();
            for (size_t i = 0; i < m_ComponentPools.size(); ++i) {
                m_ComponentPools[i]->remove(id);
                m_Systems[i]->onComponentRemovedFromEntity(entity);
            }
            m_EntityPool.destroyFlaggedEntity(id);
        }
        m_DestroyedEntities.clear();
        for (auto& component_pool : m_ComponentPools) {
            component_pool->reserve(150);
        }
    }
}