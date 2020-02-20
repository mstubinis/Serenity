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

    m_Functor.swap(other.m_Functor);

    //make it empty
    other.m_Functor = std::function<void(const float&)>();
}
ComponentLogic& ComponentLogic::operator=(ComponentLogic&& other) noexcept{
    if(&other != this){
        m_UserPointer  = std::exchange(other.m_UserPointer, nullptr);
        m_UserPointer1 = std::exchange(other.m_UserPointer1, nullptr);
        m_UserPointer2 = std::exchange(other.m_UserPointer2, nullptr);
        m_Owner        = std::move(other.m_Owner);

        m_Functor.swap(other.m_Functor);

        //make it empty
        other.m_Functor = std::function<void(const float&)>();
    }
    return *this;
}
void ComponentLogic::setUserPointer(void* UserPointer) {
    m_UserPointer  = UserPointer;
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

void ComponentLogic::call(const float& dt) const { 
    if (m_Functor) {
        m_Functor(dt);
    }
}

#pragma endregion

#pragma region System

struct priv::ComponentLogic_UpdateFunction final { void operator()(void* componentPool, const float& dt, Scene& scene) const {
    auto& pool = *static_cast<ECSComponentPool<Entity, ComponentLogic>*>(componentPool);
    const auto& components = pool.data();
	for (auto& component : components) {
		component.call(dt);
	}
}};
struct priv::ComponentLogic_ComponentAddedToEntityFunction final { void operator()(void* component, Entity& entity) const {
}};
struct priv::ComponentLogic_EntityAddedToSceneFunction final { void operator()(void* componentPool, Entity& entity, Scene& scene) const {
}};
struct priv::ComponentLogic_SceneEnteredFunction final { void operator()(void* componentPool, Scene& scene) const {
}};
struct priv::ComponentLogic_SceneLeftFunction final { void operator()(void* componentPool, Scene& scene) const {
}};

ComponentLogic_System_CI::ComponentLogic_System_CI() {
    setUpdateFunction(ComponentLogic_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentLogic_ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentLogic_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentLogic_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentLogic_SceneLeftFunction());
}

#pragma endregion