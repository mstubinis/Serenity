#include <ecs/ComponentLogic3.h>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

#pragma region Component

ComponentLogic3::ComponentLogic3(const Entity& entity) : ComponentBaseClass(entity) {
    m_UserPointer  = nullptr;
    m_UserPointer1 = nullptr;
    m_UserPointer2 = nullptr;
}
ComponentLogic3::~ComponentLogic3() {
}
/*
ComponentLogic3::ComponentLogic3(const ComponentLogic3& other) {
    m_Owner        = other.m_Owner;
    m_Functor      = other.m_Functor;
    m_UserPointer  = other.m_UserPointer;
    m_UserPointer1 = other.m_UserPointer1;
    m_UserPointer2 = other.m_UserPointer2;
}
ComponentLogic3& ComponentLogic3::operator=(const ComponentLogic3& other) {
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
ComponentLogic3::ComponentLogic3(ComponentLogic3&& other) noexcept {
    m_Owner        = std::move(other.m_Owner);
    m_Functor      = std::move(other.m_Functor);
    m_UserPointer  = std::exchange(other.m_UserPointer, nullptr);
    m_UserPointer1 = std::exchange(other.m_UserPointer1, nullptr);
    m_UserPointer2 = std::exchange(other.m_UserPointer2, nullptr);
}
ComponentLogic3& ComponentLogic3::operator=(ComponentLogic3&& other) noexcept {
    if (&other != this) {
        m_Owner        = std::move(other.m_Owner);
        m_Functor      = std::move(other.m_Functor);
        m_UserPointer  = std::exchange(other.m_UserPointer, nullptr);
        m_UserPointer1 = std::exchange(other.m_UserPointer1, nullptr);
        m_UserPointer2 = std::exchange(other.m_UserPointer2, nullptr);
    }
    return *this;
}
void ComponentLogic3::setUserPointer(void* UserPointer) {
    m_UserPointer = UserPointer;
}
void ComponentLogic3::setUserPointer1(void* UserPointer1) {
    m_UserPointer1 = UserPointer1;
}
void ComponentLogic3::setUserPointer2(void* UserPointer2) {
    m_UserPointer2 = UserPointer2;
}
void* ComponentLogic3::getUserPointer() const {
    return m_UserPointer;
}
void* ComponentLogic3::getUserPointer1() const {
    return m_UserPointer1;
}
void* ComponentLogic3::getUserPointer2() const {
    return m_UserPointer2;
}
void ComponentLogic3::call(const float& dt) { 
    if(m_Functor)
        m_Functor(dt); 
}

#pragma endregion

#pragma region System

struct priv::ComponentLogic3_UpdateFunction final {void operator()(void* _componentPool, const float& dt, Scene& _scene) const {
    auto& pool = *(ECSComponentPool<Entity, ComponentLogic3>*)_componentPool;
    auto& components = pool.data();
	for (auto& component : components) {
		component.call(dt);
	}
}};
struct priv::ComponentLogic3_ComponentAddedToEntityFunction final {void operator()(void* _component, Entity& _entity) const {

}};
struct priv::ComponentLogic3_EntityAddedToSceneFunction final {void operator()(void* _componentPool, Entity& _entity, Scene& _scene) const {
}};
struct priv::ComponentLogic3_SceneEnteredFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};
struct priv::ComponentLogic3_SceneLeftFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};

ComponentLogic3_System_CI::ComponentLogic3_System_CI() {
    setUpdateFunction(ComponentLogic3_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentLogic3_ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentLogic3_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentLogic3_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentLogic3_SceneLeftFunction());
}

#pragma endregion