#include <ecs/ComponentLogic1.h>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

#pragma region Component

ComponentLogic1::ComponentLogic1(const Entity& entity) : ComponentBaseClass(entity) {
    m_UserPointer  = nullptr;
    m_UserPointer1 = nullptr;
    m_UserPointer2 = nullptr;
}
ComponentLogic1::~ComponentLogic1() {
}
/*
ComponentLogic1::ComponentLogic1(const ComponentLogic1& other) {
    m_Owner        = other.m_Owner;
    m_Functor      = other.m_Functor;
    m_UserPointer  = other.m_UserPointer;
    m_UserPointer1 = other.m_UserPointer1;
    m_UserPointer2 = other.m_UserPointer2;
}
ComponentLogic1& ComponentLogic1::operator=(const ComponentLogic1& other) {
    if (&other != this){
        m_Owner        = other.m_Owner;
        m_Functor      = other.m_Functor;
        m_UserPointer  = other.m_UserPointer;
        m_UserPointer1 = other.m_UserPointer1;
        m_UserPointer2 = other.m_UserPointer2;
    }
    return *this;
}
*/
ComponentLogic1::ComponentLogic1(ComponentLogic1&& other) noexcept {
    m_Owner        = std::move(other.m_Owner);
    m_Functor      = std::move(other.m_Functor);
    m_UserPointer  = std::exchange(other.m_UserPointer, nullptr);
    m_UserPointer1 = std::exchange(other.m_UserPointer1, nullptr);
    m_UserPointer2 = std::exchange(other.m_UserPointer2, nullptr);
}
ComponentLogic1& ComponentLogic1::operator=(ComponentLogic1&& other) noexcept {
    if (&other != this) {
        m_Owner        = std::move(other.m_Owner);
        m_Functor      = std::move(other.m_Functor);
        m_UserPointer  = std::exchange(other.m_UserPointer, nullptr);
        m_UserPointer1 = std::exchange(other.m_UserPointer1, nullptr);
        m_UserPointer2 = std::exchange(other.m_UserPointer2, nullptr);
    }
    return *this;
}
void ComponentLogic1::setUserPointer(void* UserPointer) {
    m_UserPointer = UserPointer;
}
void ComponentLogic1::setUserPointer1(void* UserPointer1) {
    m_UserPointer1 = UserPointer1;
}
void ComponentLogic1::setUserPointer2(void* UserPointer2) {
    m_UserPointer2 = UserPointer2;
}
void* ComponentLogic1::getUserPointer() const {
    return m_UserPointer;
}
void* ComponentLogic1::getUserPointer1() const {
    return m_UserPointer1;
}
void* ComponentLogic1::getUserPointer2() const {
    return m_UserPointer2;
}
void ComponentLogic1::call(const float& dt) { 
    if(m_Functor)
        m_Functor(dt); 
}

#pragma endregion

#pragma region System

struct priv::ComponentLogic1_UpdateFunction final {void operator()(void* _componentPool, const float& dt, Scene& _scene) const {
    auto& pool = *(ECSComponentPool<Entity, ComponentLogic1>*)_componentPool;
    auto& components = pool.data();
    for (auto& component: components) {
        component.call(dt);
    }
}};
struct priv::ComponentLogic1_ComponentAddedToEntityFunction final {void operator()(void* _component, Entity& _entity) const {

}};
struct priv::ComponentLogic1_EntityAddedToSceneFunction final {void operator()(void* _componentPool, Entity& _entity, Scene& _scene) const {
}};
struct priv::ComponentLogic1_SceneEnteredFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};
struct priv::ComponentLogic1_SceneLeftFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};

ComponentLogic1_System_CI::ComponentLogic1_System_CI() {
    setUpdateFunction(ComponentLogic1_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentLogic1_ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentLogic1_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentLogic1_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentLogic1_SceneLeftFunction());
}

#pragma endregion