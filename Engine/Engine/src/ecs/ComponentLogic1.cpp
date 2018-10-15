#include "ComponentLogic1.h"

using namespace Engine;
using namespace std;

#pragma region Component

struct ComponentLogic1EmptyFunctor final {void operator()(ComponentLogic1& _component, const float& dt) const {

}};

ComponentLogic1::ComponentLogic1(Entity& _entity) : ComponentBaseClass(_entity) {
    ComponentLogic1EmptyFunctor f;
    setFunctor(f);
}
ComponentLogic1::~ComponentLogic1() {}
void ComponentLogic1::call(const float& dt) { _functor(dt); }

#pragma endregion

#pragma region System

struct epriv::ComponentLogic1UpdateFunction final {
    void operator()(void* _componentPool, const float& dt) const {
        auto& pool = *(ECSComponentPool<Entity, ComponentLogic1>*)_componentPool;
        auto& components = pool.pool();
        for (uint i = 0; i < components.size(); ++i) {
            auto& component = components[i];
            component.call(dt);
        }
    }
};
struct epriv::ComponentLogic1ComponentAddedToEntityFunction final {
    void operator()(void* _component, Entity& _entity) const {

    }
};
struct epriv::ComponentLogic1EntityAddedToSceneFunction final {
    void operator()(void* _componentPool, Entity& _entity, Scene& _scene) const {

    }
};
struct epriv::ComponentLogic1SceneEnteredFunction final {
    void operator()(void* _componentPool, Scene& _scene) const {

    }
};
struct epriv::ComponentLogic1SceneLeftFunction final {
    void operator()(void* _componentPool, Scene& _scene) const {

    }
};

ComponentLogic1System::ComponentLogic1System() {
    setUpdateFunction(epriv::ComponentLogic1UpdateFunction());
    setOnComponentAddedToEntityFunction(epriv::ComponentLogic1ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(epriv::ComponentLogic1EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(epriv::ComponentLogic1SceneEnteredFunction());
    setOnSceneLeftFunction(epriv::ComponentLogic1SceneLeftFunction());
}

#pragma endregion