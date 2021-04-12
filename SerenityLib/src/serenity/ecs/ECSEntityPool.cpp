#include <serenity/ecs/ECSEntityPool.h>
#include <serenity/scene/Scene.h>

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

    m_Pool[index].m_VersionID++;
    m_Freelist.emplace_back(index);
}
Entity Engine::priv::ECSEntityPool::getEntityFromID(uint32_t entityID) const noexcept {
    const auto index = entityID - 1U;
    return m_Pool[index];
}
Entity Engine::priv::ECSEntityPool::createEntity(const Scene& scene) noexcept {
    if (m_Freelist.empty()) {
        m_Pool.emplace_back(0, 0, 0);
        m_Freelist.emplace_back((uint32_t)m_Pool.size() - 1);
    }
    const auto id = m_Freelist.back();
    m_Freelist.pop_back();
    ASSERT(id >= 0 && id < m_Pool.size(), __FUNCTION__ << "(): id was not in the range of m_Pool!");
    Entity entity { id + 1, scene.id(), m_Pool[id].versionID() };
    m_Pool[id]    = entity;
    return entity;
}
