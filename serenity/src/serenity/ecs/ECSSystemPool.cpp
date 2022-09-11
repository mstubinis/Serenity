#include <serenity/ecs/ECSSystemPool.h>


Engine::priv::ECSSystemPool::~ECSSystemPool() {
    SAFE_DELETE_VECTOR(m_Systems);
}
void Engine::priv::ECSSystemPool::sort() {
    std::sort(std::begin(m_Order), std::end(m_Order));
}
void Engine::priv::ECSSystemPool::update(const float dt, Scene& scene) {
    for (const auto& order : m_Order) {
        const size_t idx = order.typeID;
        auto& system     = m_Systems[idx];
        if (system->isEnabled()) {
            system->onUpdate(dt, scene);
        }
    }
}
void Engine::priv::ECSSystemPool::onComponentAddedToEntity(uint32_t componentTypeID, void* component, Entity entity) {
    if (std::size(m_ComponentIDToAssociatedSystems) <= componentTypeID) {
        return;
    }
    for (auto& system : m_ComponentIDToAssociatedSystems[componentTypeID]) {
        system->onComponentAddedToEntity(component, entity);
    }
    for (auto& system : m_ComponentIDToAssociatedSystems[componentTypeID]) {
        system->addEntity(entity);
    }
}
void Engine::priv::ECSSystemPool::onComponentRemovedFromEntity(uint32_t componentTypeID, Entity entity) {
    ASSERT(std::size(m_ComponentIDToAssociatedSystems) > componentTypeID, __FUNCTION__ << "(): m_ComponentIDToSystems did not have componentTypeID");
    //if (m_ComponentIDToSystems.size() <= componentTypeID) {
    //    return;
    //}
    for (auto& system : m_ComponentIDToAssociatedSystems[componentTypeID]) {
        system->removeEntity(entity);
    }
    for (auto& system : m_ComponentIDToAssociatedSystems[componentTypeID]) {
        system->onComponentRemovedFromEntity(entity);
    }
}
void Engine::priv::ECSSystemPool::onComponentRemovedFromEntity(Entity entity) {
    for (const auto& order : m_Order) {
        const size_t idx = order.typeID;
        auto& system     = m_Systems[idx];
        system->removeEntity(entity);
    }
    for (const auto& order : m_Order) {
        const size_t idx = order.typeID;
        auto& system     = m_Systems[idx];
        system->onComponentRemovedFromEntity(entity);
    }
}
void Engine::priv::ECSSystemPool::onSceneEntered(Scene& scene) noexcept {
    for (const auto& order : m_Order) {
        const size_t idx = order.typeID;
        auto& system     = m_Systems[idx];
        system->onSceneEntered(scene);
    }
}
void Engine::priv::ECSSystemPool::onSceneLeft(Scene& scene) noexcept {
    for (const auto& order : m_Order) {
        const size_t idx = order.typeID;
        auto& system     = m_Systems[idx];
        system->onSceneLeft(scene);
    }
}