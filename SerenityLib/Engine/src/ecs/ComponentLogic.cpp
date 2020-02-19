#include <ecs/ComponentLogic.h>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

#pragma region Component

ComponentLogic::ComponentLogic(const Entity& entity) : ComponentBaseClass(entity) {
    m_UserPointer  = nullptr;
    m_UserPointer1 = nullptr;
    m_UserPointer2 = nullptr;
}

ComponentLogic::~ComponentLogic(){

}

ComponentLogic::ComponentLogic(ComponentLogic&& other) noexcept{
    m_UserPointer  = std::exchange(other.m_UserPointer, nullptr);
    m_UserPointer1 = std::exchange(other.m_UserPointer1, nullptr);
    m_UserPointer2 = std::exchange(other.m_UserPointer2, nullptr);
    m_Owner        = std::move(other.m_Owner);
    m_Functor      = std::move(other.m_Functor);
}
ComponentLogic& ComponentLogic::operator=(ComponentLogic&& other) noexcept{
    if(&other != this){
        m_UserPointer  = std::exchange(other.m_UserPointer, nullptr);
        m_UserPointer1 = std::exchange(other.m_UserPointer1, nullptr);
        m_UserPointer2 = std::exchange(other.m_UserPointer2, nullptr);
        m_Owner        = std::move(other.m_Owner);
        m_Functor      = std::move(other.m_Functor);
    }
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

void ComponentLogic::call(const float& dt) { 
    if(m_Functor)
        m_Functor(dt); 
}

#pragma endregion

#pragma region System

struct priv::ComponentLogic_UpdateFunction final { void operator()(void* _componentPool, const float& dt, Scene& _scene) const {
    auto& pool = *(ECSComponentPool<Entity, ComponentLogic>*)_componentPool;
    auto& components = pool.data();
	for (auto& component : components) {
		component.call(dt);
	}
}};
struct priv::ComponentLogic_ComponentAddedToEntityFunction final { void operator()(void* _component, Entity& _entity) const {
}};
struct priv::ComponentLogic_EntityAddedToSceneFunction final { void operator()(void* _componentPool, Entity& _entity, Scene& _scene) const {
}};
struct priv::ComponentLogic_SceneEnteredFunction final { void operator()(void* _componentPool, Scene& _scene) const {
}};
struct priv::ComponentLogic_SceneLeftFunction final { void operator()(void* _componentPool, Scene& _scene) const {
}};

ComponentLogic_System_CI::ComponentLogic_System_CI() {
    setUpdateFunction(ComponentLogic_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentLogic_ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentLogic_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentLogic_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentLogic_SceneLeftFunction());
}

#pragma endregion