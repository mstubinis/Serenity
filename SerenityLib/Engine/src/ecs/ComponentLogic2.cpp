#include <ecs/ComponentLogic2.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

#pragma region Component

ComponentLogic2::ComponentLogic2(const Entity& entity) : ComponentBaseClass(entity) {
    m_UserPointer  = nullptr;
    m_UserPointer1 = nullptr;
    m_UserPointer2 = nullptr;
    setFunctor(Engine::epriv::ComponentLogic2_EmptyFunctor());
}
ComponentLogic2::~ComponentLogic2() {
}

ComponentLogic2::ComponentLogic2(const ComponentLogic2& other) {
    m_Owner        = other.m_Owner;
    m_Functor      = other.m_Functor;
    m_UserPointer  = other.m_UserPointer;
    m_UserPointer1 = other.m_UserPointer1;
    m_UserPointer2 = other.m_UserPointer2;
}
ComponentLogic2& ComponentLogic2::operator=(const ComponentLogic2& other) {
    if (&other == this)
        return *this;
    m_Owner        = other.m_Owner;
    m_Functor      = other.m_Functor;
    m_UserPointer  = other.m_UserPointer;
    m_UserPointer1 = other.m_UserPointer1;
    m_UserPointer2 = other.m_UserPointer2;
    return *this;
}
ComponentLogic2::ComponentLogic2(ComponentLogic2&& other) noexcept {
    using std::swap;
    swap(m_Owner, other.m_Owner);
    swap(m_Functor, other.m_Functor);
    swap(m_UserPointer, other.m_UserPointer);
    swap(m_UserPointer1, other.m_UserPointer1);
    swap(m_UserPointer2, other.m_UserPointer2);
}
ComponentLogic2& ComponentLogic2::operator=(ComponentLogic2&& other) noexcept {
    using std::swap;
    swap(m_Owner, other.m_Owner);
    swap(m_Functor, other.m_Functor);
    swap(m_UserPointer, other.m_UserPointer);
    swap(m_UserPointer1, other.m_UserPointer1);
    swap(m_UserPointer2, other.m_UserPointer2);
    return *this;
}
void ComponentLogic2::setUserPointer(void* UserPointer) {
    m_UserPointer = UserPointer;
}
void ComponentLogic2::setUserPointer1(void* UserPointer1) {
    m_UserPointer1 = UserPointer1;
}
void ComponentLogic2::setUserPointer2(void* UserPointer2) {
    m_UserPointer2 = UserPointer2;
}
void* ComponentLogic2::getUserPointer() const {
    return m_UserPointer;
}
void* ComponentLogic2::getUserPointer1() const {
    return m_UserPointer1;
}
void* ComponentLogic2::getUserPointer2() const {
    return m_UserPointer2;
}
void ComponentLogic2::call(const double& dt) { 
    m_Functor(dt); 
}

#pragma endregion

#pragma region System

struct epriv::ComponentLogic2_UpdateFunction final {void operator()(void* _componentPool, const double& dt, Scene& _scene) const {
    auto& pool = *(ECSComponentPool<Entity, ComponentLogic2>*)_componentPool;
    auto& components = pool.pool();
	for (auto& component : components) {
		component.call(dt);
	}
}};
struct epriv::ComponentLogic2_ComponentAddedToEntityFunction final {void operator()(void* _component, Entity& _entity) const {

}};
struct epriv::ComponentLogic2_EntityAddedToSceneFunction final {void operator()(void* _componentPool, Entity& _entity, Scene& _scene) const {
}};
struct epriv::ComponentLogic2_SceneEnteredFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};
struct epriv::ComponentLogic2_SceneLeftFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};

ComponentLogic2_System::ComponentLogic2_System() {
    setUpdateFunction(ComponentLogic2_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentLogic2_ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentLogic2_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentLogic2_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentLogic2_SceneLeftFunction());
}

#pragma endregion