#include <serenity/ecs/ECS.h>

Engine::priv::ECS::~ECS() {
    SAFE_DELETE_VECTOR(m_ComponentPools);
}
Entity Engine::priv::ECS::createEntity(Scene& scene) {
    Entity newEntity = m_EntityPool.createEntity(scene);
    #ifndef ENGINE_PRODUCTION
        for (const auto entity : m_JustAddedEntities) {
            ASSERT(entity != newEntity, __FUNCTION__ << "(): " << newEntity.toString() << " already in m_JustAddedEntities!");
        }
    #endif
    m_JustAddedEntities.emplace_back(newEntity);
    return newEntity;
}
void Engine::priv::ECS::init(const SceneOptions& options) {
    m_SceneOptions = options;
    m_EntityPool.init(m_SceneOptions);
}
void Engine::priv::ECS::removeEntity(Entity removedEntity) {
    #ifndef ENGINE_PRODUCTION
        for (const auto entity : m_DestroyedEntities) {
            ASSERT(entity != removedEntity, __FUNCTION__ << "(): " << removedEntity.toString() << " already in m_DestroyedEntities!");
        }
    #endif
    m_DestroyedEntities.emplace_back(removedEntity);
}
void Engine::priv::ECS::update(const float dt, Scene& scene) noexcept {
    m_SystemPool.update(dt, scene);
}
void Engine::priv::ECS::onSceneEntered(Scene& scene) noexcept {
    m_SystemPool.onSceneEntered(scene);
}
void Engine::priv::ECS::onSceneLeft(Scene& scene) noexcept {
    m_SystemPool.onSceneLeft(scene);
}
void Engine::priv::ECS::preUpdate(Scene& scene, const float dt) {
    if (m_JustAddedEntities.size() > 0) {
        for (const auto& system : m_SystemPool) {
            for (const auto addedEntity : m_JustAddedEntities) {
                system->onEntityAddedToScene(scene, addedEntity);
            }
        }
        m_JustAddedEntities.clear();
    }
}
void Engine::priv::ECS::postUpdate(Scene& scene, const float dt) {
    if (m_DestroyedEntities.size() > 0) {
        for (const auto destroyedEntity : m_DestroyedEntities) {
            const auto id = destroyedEntity.id();
            m_SystemPool.onComponentRemovedFromEntity(destroyedEntity);
            for (auto& componentPool : m_ComponentPools) {
                componentPool->remove(id);
            }
            m_EntityPool.destroyFlaggedEntity(id);
        }
        m_DestroyedEntities.clear();
        for (auto& componentPool : m_ComponentPools) {
            componentPool->reserve(150);
        }
    }
}