#include <ecs/ComponentLogic.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

#pragma region Component

ComponentLogic::ComponentLogic(const Entity& entity) : ComponentBaseClass(entity) {
    m_UserPointer  = nullptr;
    m_UserPointer1 = nullptr;
    m_UserPointer2 = nullptr;
    setFunctor(Engine::epriv::ComponentLogic_EmptyFunctor());
}

ComponentLogic::~ComponentLogic(){

}

ComponentLogic::ComponentLogic(const ComponentLogic& other){
    m_Owner        = other.m_Owner;
    m_Functor      = other.m_Functor;
    m_UserPointer  = other.m_UserPointer;
    m_UserPointer1 = other.m_UserPointer1;
    m_UserPointer2 = other.m_UserPointer2;
}
ComponentLogic& ComponentLogic::operator=(const ComponentLogic& other){
    if (&other == this)
        return *this;
    m_Owner        = other.m_Owner;
    m_Functor      = other.m_Functor;
    m_UserPointer  = other.m_UserPointer;
    m_UserPointer1 = other.m_UserPointer1;
    m_UserPointer2 = other.m_UserPointer2;
    return *this;
}
ComponentLogic::ComponentLogic(ComponentLogic&& other) noexcept{
    using std::swap;
    swap(m_Owner, other.m_Owner);
    swap(m_Functor, other.m_Functor);
    swap(m_UserPointer, other.m_UserPointer);
    swap(m_UserPointer1, other.m_UserPointer1);
    swap(m_UserPointer2, other.m_UserPointer2);
}
ComponentLogic& ComponentLogic::operator=(ComponentLogic&& other) noexcept{
    using std::swap;
    swap(m_Owner, other.m_Owner);
    swap(m_Functor, other.m_Functor);
    swap(m_UserPointer, other.m_UserPointer);
    swap(m_UserPointer1, other.m_UserPointer1);
    swap(m_UserPointer2, other.m_UserPointer2);
    return *this;
}
void ComponentLogic::setUserPointer(void* UserPointer) {
    m_UserPointer = UserPointer;
}
void ComponentLogic::setUserPointer1(void* UserPointer1) {
    m_UserPointer1 = UserPointer1;
}
void ComponentLogic::setUserPointer2(void* UserPointer2) {
    m_UserPointer2 = UserPointer2;
}
void* ComponentLogic::getUserPointer() const {
    return m_UserPointer;
}
void* ComponentLogic::getUserPointer1() const {
    return m_UserPointer1;
}
void* ComponentLogic::getUserPointer2() const {
    return m_UserPointer2;
}

void ComponentLogic::call(const double& dt) { 
    m_Functor(dt); 
}

#pragma endregion

#pragma region System

struct epriv::ComponentLogic_UpdateFunction final { void operator()(void* _componentPool, const double& dt, Scene& _scene) const {
    auto& pool = *(ECSComponentPool<Entity, ComponentLogic>*)_componentPool;
    auto& components = pool.pool();
	for (auto& component : components) {
		component.call(dt);
	}
}};
struct epriv::ComponentLogic_ComponentAddedToEntityFunction final { void operator()(void* _component, Entity& _entity) const {
}};
struct epriv::ComponentLogic_EntityAddedToSceneFunction final { void operator()(void* _componentPool, Entity& _entity, Scene& _scene) const {
}};
struct epriv::ComponentLogic_SceneEnteredFunction final { void operator()(void* _componentPool, Scene& _scene) const {
}};
struct epriv::ComponentLogic_SceneLeftFunction final { void operator()(void* _componentPool, Scene& _scene) const {
}};

ComponentLogic_System_CI::ComponentLogic_System_CI() {
    setUpdateFunction(ComponentLogic_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentLogic_ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentLogic_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentLogic_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentLogic_SceneLeftFunction());
}

#pragma endregion