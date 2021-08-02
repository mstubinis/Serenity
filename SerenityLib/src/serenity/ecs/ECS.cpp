#include <serenity/ecs/ECS.h>

Entity Engine::priv::ECS::createEntity(Scene& scene) {
    Entity entity = m_EntityPool.createEntity(scene);
    #ifndef ENGINE_PRODUCTION
        for (auto e : m_JustAddedEntities) {
            if (e == entity) {
                ASSERT(false, __FUNCTION__ << "(): " << entity.toString() << " already in m_JustAddedEntities!");
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
void Engine::priv::ECS::removeEntity(Entity entity) {
    #ifndef ENGINE_PRODUCTION
        for (auto e : m_DestroyedEntities) {
            if (e == entity) {
                ASSERT(false, __FUNCTION__ << "(): " << entity.toString() << " already in m_DestroyedEntities!");
            }
        }
    #endif
    m_DestroyedEntities.emplace_back(entity);
}
void Engine::priv::ECS::preUpdate(Scene& scene, const float dt) {
    if (m_JustAddedEntities.size() > 0) {
        for (const auto& system : m_SystemPool) {
            for (const auto entity : m_JustAddedEntities) {
                system->onEntityAddedToScene(scene, entity);
            }
        }
        m_JustAddedEntities.clear();
    }
}
void Engine::priv::ECS::postUpdate(Scene& scene, const float dt) {
    if (m_DestroyedEntities.size() > 0) {
        for (const auto entity : m_DestroyedEntities) {
            const auto id = entity.id();
            m_SystemPool.onComponentRemovedFromEntity(entity);
            for (size_t i = 0; i < m_ComponentPools.size(); ++i) {
                m_ComponentPools[i]->remove(id);
            }
            m_EntityPool.destroyFlaggedEntity(id);
        }
        m_DestroyedEntities.clear();
        for (auto& component_pool : m_ComponentPools) {
            component_pool->reserve(150);
        }
    }
}