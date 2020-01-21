#include <ecs/ComponentLogic1.h>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

#pragma region Component

ComponentLogic1::ComponentLogic1(const Entity& entity) : ComponentBaseClass(entity) {
    m_UserPointer  = nullptr;
    m_UserPointer1 = nullptr;
    m_UserPointer2 = nullptr;
    setFunctor(Engine::priv::ComponentLogic1_EmptyFunctor());
}
ComponentLogic1::~ComponentLogic1() {
}

ComponentLogic1::ComponentLogic1(const ComponentLogic1& other) {
    m_Owner        = other.m_Owner;
    m_Functor      = other.m_Functor;
    m_UserPointer  = other.m_UserPointer;
    m_UserPointer1 = other.m_UserPointer1;
    m_UserPointer2 = other.m_UserPointer2;
}
ComponentLogic1& ComponentLogic1::operator=(const ComponentLogic1& other) {
    if (&other == this)
        return *this;
    m_Owner        = other.m_Owner;
    m_Functor      = other.m_Functor;
    m_UserPointer  = other.m_UserPointer;
    m_UserPointer1 = other.m_UserPointer1;
    m_UserPointer2 = other.m_UserPointer2;
    return *this;
}
ComponentLogic1::ComponentLogic1(ComponentLogic1&& other) noexcept {
    using std::swap;
    swap(m_Owner, other.m_Owner);
    swap(m_Functor, other.m_Functor);
    swap(m_UserPointer, other.m_UserPointer);
    swap(m_UserPointer1, other.m_UserPointer1);
    swap(m_UserPointer2, other.m_UserPointer2);
}
ComponentLogic1& ComponentLogic1::operator=(ComponentLogic1&& other) noexcept {
    using std::swap;
    swap(m_Owner, other.m_Owner);
    swap(m_Functor, other.m_Functor);
    swap(m_UserPointer, other.m_UserPointer);
    swap(m_UserPointer1, other.m_UserPointer1);
    swap(m_UserPointer2, other.m_UserPointer2);
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
void ComponentLogic1::call(const double& dt) { 
    m_Functor(dt); 
}

#pragma endregion

#pragma region System

struct priv::ComponentLogic1_UpdateFunction final {void operator()(void* _componentPool, const double& dt, Scene& _scene) const {
    auto& pool = *(ECSComponentPool<Entity, ComponentLogic1>*)_componentPool;
    auto& components = pool.pool();
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