#include <core/engine/utils/PrecompiledHeader.h>
#include <ecs/ECSEntityPool.h>

#include <core/engine/scene/Scene.h>

void Engine::priv::ECSEntityPool::init(const SceneOptions& options) {
    m_Pool.reserve(options.maxAmountOfEntities);
    m_Freelist.reserve(options.maxAmountOfEntities);
}

bool Engine::priv::ECSEntityPool::isEntityVersionDifferent(Entity entity) const noexcept {
    uint32_t index = entity.id() - 1U;
    return (index >= m_Pool.size()) ? true : (m_Pool[index].versionID() != entity.versionID());
}

void Engine::priv::ECSEntityPool::destroyFlaggedEntity(uint32_t entityID) {
    auto index             = entityID - 1U;
    Entity storedEntity    = m_Pool[index];
    uint32_t storedVersion = storedEntity.versionID();
    ++storedVersion;
    Entity updatedEntity(storedEntity.id(), storedEntity.sceneID(), storedVersion);
    m_Pool[index] = std::move(updatedEntity);
    m_Freelist.emplace_back(index);
}
Entity Engine::priv::ECSEntityPool::addEntity(const Scene& scene) noexcept {
    if (m_Freelist.empty()) {
        m_Pool.emplace_back(0U, 0U, 0U);
        m_Freelist.emplace_back(static_cast<uint32_t>(m_Pool.size()) - 1U);
    }
    auto id = m_Freelist.back();
    Entity old = m_Pool[id];
    Entity entity = Entity{ id + 1U, scene.id(), old.versionID() };
    m_Freelist.pop_back();
    m_Pool[id] = entity;
    return entity;
}
Entity Engine::priv::ECSEntityPool::getEntity(uint32_t entityData) const noexcept {
    if (entityData == 0) {
        return Entity{};
    }
    auto index = Entity::id(entityData) - 1U;
    if (index < m_Pool.size()) {
        Entity e = m_Pool[index];
        if (e.versionID() == Entity::versionID(entityData)) {
            return e;
        }
    }
    return Entity{};
}