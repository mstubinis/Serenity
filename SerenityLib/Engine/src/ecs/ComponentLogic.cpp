#include <ecs/ComponentLogic.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

#pragma region Component

ComponentLogic::ComponentLogic(const Entity& _e) : ComponentBaseClass(_e) {
    _userPtr  = nullptr;
    _userPtr1 = nullptr;
    _userPtr2 = nullptr;
    setFunctor(Engine::epriv::ComponentLogic_EmptyFunctor());
}

ComponentLogic::~ComponentLogic(){

}

ComponentLogic::ComponentLogic(const ComponentLogic& other){
    owner = other.owner;
    _functor = other._functor;
    _userPtr = other._userPtr;
    _userPtr1 = other._userPtr1;
    _userPtr2 = other._userPtr2;
}
ComponentLogic& ComponentLogic::operator=(const ComponentLogic& other){
    if (&other == this)
        return *this;
    owner = other.owner;
    _functor = other._functor;
    _userPtr = other._userPtr;
    _userPtr1 = other._userPtr1;
    _userPtr2 = other._userPtr2;
    return *this;
}
ComponentLogic::ComponentLogic(ComponentLogic&& other) noexcept{
    using std::swap;
    swap(owner, other.owner);
    swap(_functor, other._functor);
    swap(_userPtr, other._userPtr);
    swap(_userPtr1, other._userPtr1);
    swap(_userPtr2, other._userPtr2);
}
ComponentLogic& ComponentLogic::operator=(ComponentLogic&& other) noexcept{
    using std::swap;
    swap(owner, other.owner);
    swap(_functor, other._functor);
    swap(_userPtr, other._userPtr);
    swap(_userPtr1, other._userPtr1);
    swap(_userPtr2, other._userPtr2);
    return *this;
}
void ComponentLogic::setUserPointer(void* ptr) {
    _userPtr = ptr;
}
void ComponentLogic::setUserPointer1(void* ptr) {
    _userPtr1 = ptr;
}
void ComponentLogic::setUserPointer2(void* ptr) {
    _userPtr2 = ptr;
}
void* ComponentLogic::getUserPointer() const {
    return _userPtr; 
}
void* ComponentLogic::getUserPointer1() const {
    return _userPtr1; 
}
void* ComponentLogic::getUserPointer2() const {
    return _userPtr2; 
}

void ComponentLogic::call(const double& dt) { 
    _functor(dt); 
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

ComponentLogic_System::ComponentLogic_System() {
    setUpdateFunction(ComponentLogic_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentLogic_ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentLogic_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentLogic_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentLogic_SceneLeftFunction());
}

#pragma endregion