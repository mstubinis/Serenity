#include <serenity/ecs/ECSEntityPool.h>
#include <serenity/scene/Scene.h>

void Engine::priv::ECSEntityPool::init(const SceneOptions& options) {
    m_Pool.reserve(options.maxAmountOfEntities);
    m_FreelistEntityIDs.reserve(options.maxAmountOfEntities);
}
bool Engine::priv::ECSEntityPool::isEntityVersionDifferent(Entity entity) const noexcept {
    const uint32_t index = entity.id();
    return (index >= m_Pool.size()) ? true : (m_Pool[index].versionID() != entity.versionID());
}
void Engine::priv::ECSEntityPool::destroyFlaggedEntity(uint32_t entityID) {
#ifndef ENGINE_PRODUCTION
    //check for double destroy
    for (const auto idx : m_FreelistEntityIDs) {
        ASSERT(idx != entityID, __FUNCTION__ << "(): " << entityID << " already in m_Freelist!");
    }
#endif
    m_Pool[entityID].m_VersionID++;
    m_FreelistEntityIDs.emplace_back(entityID);
}
Entity Engine::priv::ECSEntityPool::getEntityFromID(uint32_t entityID) const noexcept {
    return m_Pool[entityID];
}
Entity Engine::priv::ECSEntityPool::createEntity(const Scene& scene) noexcept {
    if (m_FreelistEntityIDs.empty()) {
        const auto id = uint32_t(m_Pool.size());
        Entity entity{ id, scene.id(), 0 };
        {
            std::lock_guard lock{ m_Mutex };
            m_Pool.emplace_back(entity);
        }
        return entity;
    } else {
        const auto id = m_FreelistEntityIDs.back();
        ASSERT(id >= 0 && id < m_Pool.size(), __FUNCTION__ << "(): id was not in the range of m_Pool!");
        Entity entity{ id, scene.id(), m_Pool[id].versionID() };
        {
            std::lock_guard lock{ m_Mutex };
            m_FreelistEntityIDs.pop_back();
            m_Pool[id] = entity;
        }
        return entity;
    }
    return {};
}
std::vector<Entity> Engine::priv::ECSEntityPool::createEntity(const Scene& scene, uint32_t amount) noexcept {
    auto resultEntities = Engine::create_and_reserve<std::vector<Entity>>(amount);
    {
        if (m_FreelistEntityIDs.size() > 0) {
            std::lock_guard lock{ m_Mutex };
            size_t until = std::min(resultEntities.capacity(), m_FreelistEntityIDs.size());
            for (size_t i = 0; i < until; ++i) {
                const auto id = m_FreelistEntityIDs.back();
                ASSERT(id >= 0 && id < m_Pool.size(), __FUNCTION__ << "(): id was not in the range of m_Pool!");
                resultEntities.emplace_back(id, scene.id(), m_Pool[id].versionID());

                m_FreelistEntityIDs.pop_back();
                m_Pool[id] = resultEntities.back();
            }
        }
    }
    if (resultEntities.size() < resultEntities.capacity()) {
        size_t j = resultEntities.size();
        uint32_t id = uint32_t(m_Pool.size());
        while (resultEntities.size() < resultEntities.capacity()) {
            resultEntities.emplace_back(id++, scene.id(), 0);
        }
        {
            std::lock_guard lock{ m_Mutex };
            for (; j < resultEntities.capacity(); ++j) {
                m_Pool.emplace_back(resultEntities[j]);
            }
        }
    }
    return resultEntities;
}
std::vector<Entity> Engine::priv::ECSEntityPool::createEntities(const Scene& scene, uint32_t amount) noexcept {
    return Engine::priv::ECSEntityPool::createEntity(scene, amount);
}