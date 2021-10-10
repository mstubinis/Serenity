#include <serenity/ecs/ECSEntityPool.h>
#include <serenity/scene/Scene.h>

void Engine::priv::ECSEntityPool::init(const SceneOptions& options) {
    m_Pool.reserve(options.maxAmountOfEntities);
    m_Freelist.reserve(options.maxAmountOfEntities);
}
bool Engine::priv::ECSEntityPool::isEntityVersionDifferent(Entity entity) const noexcept {
    const uint32_t index = entity.id();
    return (index >= m_Pool.size()) ? true : (m_Pool[index].versionID() != entity.versionID());
}
void Engine::priv::ECSEntityPool::destroyFlaggedEntity(uint32_t entityID) {
#ifndef ENGINE_PRODUCTION
    //check for double destroy
    for (const auto idx : m_Freelist) {
        if (idx == entityID) {
            ASSERT(false, __FUNCTION__ << "(): " << entityID << " already in m_Freelist!");
        }
    }
#endif
    m_Pool[entityID].m_VersionID++;
    m_Freelist.emplace_back(entityID);
}
Entity Engine::priv::ECSEntityPool::getEntityFromID(uint32_t entityID) const noexcept {
    return m_Pool[entityID];
}
Entity Engine::priv::ECSEntityPool::createEntity(const Scene& scene) noexcept {
    if (m_Freelist.empty()) {
        m_Pool.emplace_back();
        m_Freelist.emplace_back((uint32_t)m_Pool.size() - 1);
    }
    const auto id = m_Freelist.back();
    m_Freelist.pop_back();
    ASSERT(id >= 0 && id < m_Pool.size(), __FUNCTION__ << "(): id was not in the range of m_Pool!");
    Entity entity { id, scene.id(), m_Pool[id].versionID() };
    m_Pool[id]    = entity;
    return entity;
}
