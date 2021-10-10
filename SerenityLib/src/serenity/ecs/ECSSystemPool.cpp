#include <serenity/ecs/ECSSystemPool.h>


Engine::priv::ECSSystemPool::~ECSSystemPool() {
    SAFE_DELETE_VECTOR(m_Systems);
}
void Engine::priv::ECSSystemPool::update(const float dt, Scene& scene) {
    for (int i = 0; i < m_Order.size(); ++i) {
        m_Systems[m_Order[i].typeID]->onUpdate(dt, scene);
    }
}
void Engine::priv::ECSSystemPool::onComponentAddedToEntity(uint32_t componentTypeID, void* component, Entity entity) {
    if (m_ComponentIDToAssociatedSystems.size() <= componentTypeID) {
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
    ASSERT(m_ComponentIDToAssociatedSystems.size() > componentTypeID, __FUNCTION__ << "(): m_ComponentIDToSystems did not have componentTypeID");
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
    for (int i = 0; i < m_Order.size(); ++i) {
        m_Systems[m_Order[i].typeID]->removeEntity(entity);
    }
    for (int i = 0; i < m_Order.size(); ++i) {
        m_Systems[m_Order[i].typeID]->onComponentRemovedFromEntity(entity);
    }
}
void Engine::priv::ECSSystemPool::onSceneEntered(Scene& scene) noexcept {
    for (int i = 0; i < m_Order.size(); ++i) {
        m_Systems[m_Order[i].typeID]->onSceneEntered(scene);
    }
}
void Engine::priv::ECSSystemPool::onSceneLeft(Scene& scene) noexcept {
    for (int i = 0; i < m_Order.size(); ++i) {
        m_Systems[m_Order[i].typeID]->onSceneLeft(scene);
    }
}