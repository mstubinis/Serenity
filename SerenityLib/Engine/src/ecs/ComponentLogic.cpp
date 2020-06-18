#include <ecs/ComponentLogic.h>
#include <ecs/ECSComponentPool.h>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

#pragma region Component

ComponentLogic::ComponentLogic(Entity entity){
    m_Owner = entity;
}

ComponentLogic::~ComponentLogic(){

}

ComponentLogic::ComponentLogic(ComponentLogic&& other) noexcept {
    m_UserPointer      = std::exchange(other.m_UserPointer, nullptr);
    m_UserPointer1     = std::exchange(other.m_UserPointer1, nullptr);
    m_UserPointer2     = std::exchange(other.m_UserPointer2, nullptr);
    m_Owner            = std::move(other.m_Owner);
    m_Functor          = std::move(other.m_Functor);
}
ComponentLogic& ComponentLogic::operator=(ComponentLogic&& other) noexcept{
    if(&other != this){
        m_UserPointer      = std::exchange(other.m_UserPointer, nullptr);
        m_UserPointer1     = std::exchange(other.m_UserPointer1, nullptr);
        m_UserPointer2     = std::exchange(other.m_UserPointer2, nullptr);
        m_Owner            = std::move(other.m_Owner);
        m_Functor          = std::move(other.m_Functor);
    }
    return *this;
}
const Entity ComponentLogic::getOwner() const {
    return m_Owner;
}
void ComponentLogic::setFunctor(std::function<void(const ComponentLogic*, const float)> functor) {
    m_Functor.setFunctor(functor);
}
void ComponentLogic::setFunctor(luabridge::LuaRef luaFunction) {
    m_Functor.setFunctor(luaFunction);
}
void ComponentLogic::setUserPointer1(void* UserPointer1) {
    m_UserPointer1 = UserPointer1;
}
void ComponentLogic::setUserPointer2(void* UserPointer2) {
    m_UserPointer2 = UserPointer2;
}
void* ComponentLogic::getUserPointer1() const {
    return m_UserPointer1;
}
void* ComponentLogic::getUserPointer2() const {
    return m_UserPointer2;
}

void ComponentLogic::call(const float dt) const { 
    m_Functor(this, dt);
}

#pragma endregion

#pragma region System

struct priv::ComponentLogic_UpdateFunction final { void operator()(void* system, void* componentPool, const float dt, Scene& scene) const {
    auto& pool = *static_cast<ECSComponentPool<Entity, ComponentLogic>*>(componentPool);
    const auto& components = pool.data();
	for (auto& component : components) {
		component.call(dt);
	}
}};
struct priv::ComponentLogic_ComponentAddedToEntityFunction final { void operator()(void* system, void* component, Entity entity) const {
}};
struct priv::ComponentLogic_ComponentRemovedFromEntityFunction final { void operator()(void* system, Entity entity) const {
}};
struct priv::ComponentLogic_EntityAddedToSceneFunction final { void operator()(void* system, void* componentPool, Entity entity, Scene& scene) const {
}};
struct priv::ComponentLogic_SceneEnteredFunction final { void operator()(void* system, void* componentPool, Scene& scene) const {
}};
struct priv::ComponentLogic_SceneLeftFunction final { void operator()(void* system, void* componentPool, Scene& scene) const {
}};

ComponentLogic_System_CI::ComponentLogic_System_CI() {
    setUpdateFunction(ComponentLogic_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentLogic_ComponentAddedToEntityFunction());
    setOnComponentRemovedFromEntityFunction(ComponentLogic_ComponentRemovedFromEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentLogic_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentLogic_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentLogic_SceneLeftFunction());
}

#pragma endregion