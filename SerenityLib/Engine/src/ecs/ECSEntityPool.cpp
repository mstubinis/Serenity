#include <core/engine/utils/PrecompiledHeader.h>
#include <ecs/ECSEntityPool.h>
#include <core/engine/scene/Scene.h>

void Engine::priv::ECSEntityPool::init(const SceneOptions& options) {
    m_Pool.reserve(options.maxAmountOfEntities);
    m_Freelist.reserve(options.maxAmountOfEntities);
}
bool Engine::priv::ECSEntityPool::isEntityVersionDifferent(Entity entity) const noexcept {
    const uint32_t index = entity.id() - 1U;
    return (index >= m_Pool.size()) ? true : (m_Pool[index].versionID() != entity.versionID());
}
void Engine::priv::ECSEntityPool::destroyFlaggedEntity(uint32_t entityID) {
    const auto index          = entityID - 1U;

#ifndef ENGINE_PRODUCTION
    //check for double destroy
    for (auto idx : m_Freelist) {
        if (idx == index) {
            ASSERT(false, __FUNCTION__ << "(): " << index << " already in m_Freelist!");
        }
    }
#endif

    const Entity storedEntity = m_Pool[index];
    m_Pool[index]             = Entity{ storedEntity.id(), storedEntity.sceneID(), storedEntity.versionID() + 1 };
    m_Freelist.emplace_back(index);
}
Entity Engine::priv::ECSEntityPool::addEntity(const Scene& scene) noexcept {
    if (m_Freelist.empty()) {
        m_Pool.emplace_back(0U, 0U, 0U);
        m_Freelist.emplace_back((uint32_t)m_Pool.size() - 1U);
    }
    const auto id = m_Freelist.back();
    m_Freelist.pop_back();
    ASSERT(id >= 0 && id < m_Pool.size(), __FUNCTION__ << "(): id was not in the range of m_Pool!");
    Entity entity = Entity{ id + 1U, scene.id(), m_Pool[id].versionID() };
    m_Pool[id]    = entity;
    return entity;
}
Entity Engine::priv::ECSEntityPool::getEntity(uint32_t entityData) const noexcept {
    if (entityData == 0) {
        return Entity{};
    }
    const auto index = Entity::id(entityData) - 1U;
    if (index < m_Pool.size()) {
        if (m_Pool[index].versionID() == Entity::versionID(entityData)) {
            return m_Pool[index];
        }
    }
    return Entity{};
}