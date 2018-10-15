#include "ComponentLogic2.h"

using namespace Engine;
using namespace std;

#pragma region Component

struct ComponentLogic2EmptyFunctor final {void operator()(ComponentLogic2& _component, const float& dt) const {

}};

ComponentLogic2::ComponentLogic2(Entity& _entity) : ComponentBaseClass(_entity) {
    ComponentLogic2EmptyFunctor f;
    setFunctor(f);
}
ComponentLogic2::~ComponentLogic2() {}
void ComponentLogic2::call(const float& dt) { _functor(dt); }

#pragma endregion

#pragma region System

struct epriv::ComponentLogic2UpdateFunction final {
    void operator()(void* _componentPool, const float& dt) const {
        auto& pool = *(ECSComponentPool<Entity, ComponentLogic2>*)_componentPool;
        auto& components = pool.pool();
        for (uint i = 0; i < components.size(); ++i) {
            auto& component = components[i];
            component.call(dt);
        }
    }
};
struct epriv::ComponentLogic2ComponentAddedToEntityFunction final {
    void operator()(void* _component, Entity& _entity) const {

    }
};
struct epriv::ComponentLogic2EntityAddedToSceneFunction final {
    void operator()(void* _componentPool, Entity& _entity, Scene& _scene) const {

    }
};
struct epriv::ComponentLogic2SceneEnteredFunction final {
    void operator()(void* _componentPool, Scene& _scene) const {

    }
};
struct epriv::ComponentLogic2SceneLeftFunction final {
    void operator()(void* _componentPool, Scene& _scene) const {

    }
};

ComponentLogic2System::ComponentLogic2System() {
    setUpdateFunction(epriv::ComponentLogic2UpdateFunction());
    setOnComponentAddedToEntityFunction(epriv::ComponentLogic2ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(epriv::ComponentLogic2EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(epriv::ComponentLogic2SceneEnteredFunction());
    setOnSceneLeftFunction(epriv::ComponentLogic2SceneLeftFunction());
}

#pragma endregion