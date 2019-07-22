#include <ecs/ComponentLogic2.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

#pragma region Component

ComponentLogic2::ComponentLogic2(const Entity& _e) : ComponentBaseClass(_e) {
    _userPtr  = nullptr;
    _userPtr1 = nullptr;
    _userPtr2 = nullptr;
    setFunctor(Engine::epriv::ComponentLogic2_EmptyFunctor());
}
ComponentLogic2::~ComponentLogic2() {
}

ComponentLogic2::ComponentLogic2(const ComponentLogic2& other) {
    owner = other.owner;
    _functor = other._functor;
    _userPtr = other._userPtr;
    _userPtr1 = other._userPtr1;
    _userPtr2 = other._userPtr2;
}
ComponentLogic2& ComponentLogic2::operator=(const ComponentLogic2& other) {
    if (&other == this)
        return *this;
    owner = other.owner;
    _functor = other._functor;
    _userPtr = other._userPtr;
    _userPtr1 = other._userPtr1;
    _userPtr2 = other._userPtr2;
    return *this;
}
ComponentLogic2::ComponentLogic2(ComponentLogic2&& other) noexcept {
    using std::swap;
    swap(owner, other.owner);
    swap(_functor, other._functor);
    swap(_userPtr, other._userPtr);
    swap(_userPtr1, other._userPtr1);
    swap(_userPtr2, other._userPtr2);
}
ComponentLogic2& ComponentLogic2::operator=(ComponentLogic2&& other) noexcept {
    using std::swap;
    swap(owner, other.owner);
    swap(_functor, other._functor);
    swap(_userPtr, other._userPtr);
    swap(_userPtr1, other._userPtr1);
    swap(_userPtr2, other._userPtr2);
    return *this;
}
void ComponentLogic2::setUserPointer(void* ptr) {
    _userPtr = ptr;
}
void ComponentLogic2::setUserPointer1(void* ptr) {
    _userPtr1 = ptr;
}
void ComponentLogic2::setUserPointer2(void* ptr) {
    _userPtr2 = ptr;
}
void* ComponentLogic2::getUserPointer() const {
    return _userPtr;
}
void* ComponentLogic2::getUserPointer1() const {
    return _userPtr1;
}
void* ComponentLogic2::getUserPointer2() const {
    return _userPtr2;
}
void ComponentLogic2::call(const double& dt) { 
    _functor(dt); 
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