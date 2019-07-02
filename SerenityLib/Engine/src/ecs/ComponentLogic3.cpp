#include <ecs/ComponentLogic3.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

#pragma region Component

ComponentLogic3::~ComponentLogic3() {
}

ComponentLogic3::ComponentLogic3(const ComponentLogic3& other) {
    owner = other.owner;
    _functor = other._functor;
    _userPtr = other._userPtr;
}
ComponentLogic3& ComponentLogic3::operator=(const ComponentLogic3& other) {
    if (&other == this)
        return *this;
    owner = other.owner;
    _functor = other._functor;
    _userPtr = other._userPtr;
    return *this;
}
ComponentLogic3::ComponentLogic3(ComponentLogic3&& other) noexcept {
    using std::swap;
    swap(owner, other.owner);
    swap(_functor, other._functor);
    swap(_userPtr, other._userPtr);
}
ComponentLogic3& ComponentLogic3::operator=(ComponentLogic3&& other) noexcept {
    using std::swap;
    swap(owner, other.owner);
    swap(_functor, other._functor);
    swap(_userPtr, other._userPtr);
    return *this;
}

void ComponentLogic3::call(const double& dt) { _functor(dt); }

#pragma endregion

#pragma region System

struct epriv::ComponentLogic3_UpdateFunction final {void operator()(void* _componentPool, const double& dt, Scene& _scene) const {
    auto& pool = *(ECSComponentPool<Entity, ComponentLogic3>*)_componentPool;
    auto& components = pool.pool();
	for (auto& component : components) {
		component.call(dt);
	}
}};
struct epriv::ComponentLogic3_ComponentAddedToEntityFunction final {void operator()(void* _component, Entity& _entity) const {

}};
struct epriv::ComponentLogic3_EntityAddedToSceneFunction final {void operator()(void* _componentPool, Entity& _entity, Scene& _scene) const {
}};
struct epriv::ComponentLogic3_SceneEnteredFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};
struct epriv::ComponentLogic3_SceneLeftFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};

ComponentLogic3_System::ComponentLogic3_System() {
    setUpdateFunction(ComponentLogic3_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentLogic3_ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentLogic3_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentLogic3_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentLogic3_SceneLeftFunction());
}

#pragma endregion