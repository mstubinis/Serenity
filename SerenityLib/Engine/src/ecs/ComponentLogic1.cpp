#include <ecs/ComponentLogic1.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

#pragma region Component

ComponentLogic1::ComponentLogic1(const Entity& _e) : ComponentBaseClass(_e) {
    _userPtr  = nullptr;
    _userPtr1 = nullptr;
    _userPtr2 = nullptr;
    setFunctor(Engine::epriv::ComponentLogic1_EmptyFunctor());
}
ComponentLogic1::~ComponentLogic1() {
}

ComponentLogic1::ComponentLogic1(const ComponentLogic1& other) {
    owner = other.owner;
    _functor = other._functor;
    _userPtr = other._userPtr;
    _userPtr1 = other._userPtr1;
    _userPtr2 = other._userPtr2;
}
ComponentLogic1& ComponentLogic1::operator=(const ComponentLogic1& other) {
    if (&other == this)
        return *this;
    owner = other.owner;
    _functor = other._functor;
    _userPtr = other._userPtr;
    _userPtr1 = other._userPtr1;
    _userPtr2 = other._userPtr2;
    return *this;
}
ComponentLogic1::ComponentLogic1(ComponentLogic1&& other) noexcept {
    using std::swap;
    swap(owner, other.owner);
    swap(_functor, other._functor);
    swap(_userPtr, other._userPtr);
    swap(_userPtr1, other._userPtr1);
    swap(_userPtr2, other._userPtr2);
}
ComponentLogic1& ComponentLogic1::operator=(ComponentLogic1&& other) noexcept {
    using std::swap;
    swap(owner, other.owner);
    swap(_functor, other._functor);
    swap(_userPtr, other._userPtr);
    swap(_userPtr1, other._userPtr1);
    swap(_userPtr2, other._userPtr2);
    return *this;
}
void ComponentLogic1::setUserPointer(void* ptr) {
    _userPtr = ptr;
}
void ComponentLogic1::setUserPointer1(void* ptr) {
    _userPtr1 = ptr;
}
void ComponentLogic1::setUserPointer2(void* ptr) {
    _userPtr2 = ptr;
}
void* ComponentLogic1::getUserPointer() const {
    return _userPtr;
}
void* ComponentLogic1::getUserPointer1() const {
    return _userPtr1;
}
void* ComponentLogic1::getUserPointer2() const {
    return _userPtr2;
}
void ComponentLogic1::call(const double& dt) { 
    _functor(dt); 
}

#pragma endregion

#pragma region System

struct epriv::ComponentLogic1_UpdateFunction final {void operator()(void* _componentPool, const double& dt, Scene& _scene) const {
    auto& pool = *(ECSComponentPool<Entity, ComponentLogic1>*)_componentPool;
    auto& components = pool.pool();
    for (auto& component: components) {
        component.call(dt);
    }
}};
struct epriv::ComponentLogic1_ComponentAddedToEntityFunction final {void operator()(void* _component, Entity& _entity) const {

}};
struct epriv::ComponentLogic1_EntityAddedToSceneFunction final {void operator()(void* _componentPool, Entity& _entity, Scene& _scene) const {
}};
struct epriv::ComponentLogic1_SceneEnteredFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};
struct epriv::ComponentLogic1_SceneLeftFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};

ComponentLogic1_System::ComponentLogic1_System() {
    setUpdateFunction(ComponentLogic1_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentLogic1_ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentLogic1_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentLogic1_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentLogic1_SceneLeftFunction());
}

#pragma endregion