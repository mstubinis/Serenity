#include "ECS.h"

using namespace Engine::epriv;

ECSEntityPool::ECSEntityPool() :lastIndex(0) {
}
ECSEntityPool::~ECSEntityPool() {
    lastIndex = 0;
    pool.clear();
}
Entity* ECSEntityPool::createEntity(Scene& _scene) {
    Entity e = Entity(lastIndex + 1, _scene);
    addEntity(e);
    return &pool[pool.size() - 1];
}
void ECSEntityPool::addEntity(const Entity& _entity) {
    pool.push_back(std::move(_entity));
    ++lastIndex;
}
void ECSEntityPool::removeEntity(uint _entityID) {
    uint indexToRemove = _entityID - 1;
    if (indexToRemove != lastIndex) {
        std::swap(pool[indexToRemove], pool[lastIndex]);
    }
    Entity& e = pool[pool.size()];
    /*
    for(auto system : componentSystems){
        system.removeComponent(this);
    }
    */
    pool.pop_back();
    --lastIndex;
}
void ECSEntityPool::removeEntity(Entity& _entity) { removeEntity(_entity.ID); }
Entity* ECSEntityPool::getEntity(uint _entityID) {
    if (_entityID == 0) return nullptr;
    return &pool[_entityID - 1];
}

void ECSEntityPool::moveEntity(ECSEntityPool& other, uint _entityID) {
    Entity& e = pool[_entityID - 1];
    other.addEntity(e);
    removeEntity(e);
}
void ECSEntityPool::moveEntity(ECSEntityPool& other, Entity& _entity) { moveEntity(other, _entity.ID); }



ECS::ECS() {

}
ECS::~ECS() {

}