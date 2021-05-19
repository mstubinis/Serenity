#include <serenity/ecs/ECSSystemPool.h>

void Engine::priv::ECSSystemPool::update(const float dt, Scene& scene) {
    for (int i = 0; i < m_Order.size(); ++i) {
        m_Systems[m_Order[i].typeID - 1]->onUpdate(dt, scene);
    }
}
void Engine::priv::ECSSystemPool::onComponentAddedToEntity(uint32_t componentTypeID, void* component, Entity entity) {
    if (m_ComponentIDToSystems.size() < componentTypeID) {
        return;
    }
    for (int i = 0; i < m_ComponentIDToSystems[componentTypeID - 1].size(); ++i) {
        m_ComponentIDToSystems[componentTypeID - 1][i]->onComponentAddedToEntity(component, entity);
    }
    for (int i = 0; i < m_ComponentIDToSystems[componentTypeID - 1].size(); ++i) {
        m_ComponentIDToSystems[componentTypeID - 1][i]->addEntity(entity);
    }
}
void Engine::priv::ECSSystemPool::onComponentRemovedFromEntity(uint32_t componentTypeID, Entity entity) {
    ASSERT(m_ComponentIDToSystems.size() >= componentTypeID, __FUNCTION__ << "(): m_ComponentIDToSystems did not have componentTypeID");
    //if (m_ComponentIDToSystems.size() < componentTypeID) {
    //    return;
    //}
    for (int i = 0; i < m_ComponentIDToSystems[componentTypeID - 1].size(); ++i) {
        m_ComponentIDToSystems[componentTypeID - 1][i]->removeEntity(entity);
    }
    for (int i = 0; i < m_ComponentIDToSystems[componentTypeID - 1].size(); ++i) {
        m_ComponentIDToSystems[componentTypeID - 1][i]->onComponentRemovedFromEntity(entity);
    }
}
void Engine::priv::ECSSystemPool::onComponentRemovedFromEntity(Entity entity) {
    for (int i = 0; i < m_Order.size(); ++i) {
        m_Systems[m_Order[i].typeID - 1]->removeEntity(entity);
    }
    for (int i = 0; i < m_Order.size(); ++i) {
        m_Systems[m_Order[i].typeID - 1]->onComponentRemovedFromEntity(entity);
    }
}
void Engine::priv::ECSSystemPool::onSceneEntered(Scene& scene) noexcept {
    for (int i = 0; i < m_Order.size(); ++i) {
        m_Systems[m_Order[i].typeID - 1]->onSceneEntered(scene);
    }
}
void Engine::priv::ECSSystemPool::onSceneLeft(Scene& scene) noexcept {
    for (int i = 0; i < m_Order.size(); ++i) {
        m_Systems[m_Order[i].typeID - 1]->onSceneLeft(scene);
    }
}