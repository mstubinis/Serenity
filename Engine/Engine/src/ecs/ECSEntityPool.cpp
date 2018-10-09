#include "ecs/ECSEntityPool.h"
#include "ecs/Entity.h"

using namespace Engine;

Entity& epriv::ECSEntityPool::addEntity(Scene& _scene) {
    if (!_freelist.empty()) {
        _pool.push_back(EntityPOD(0));
        _freelist.push_back(uint(_pool.size() - 1));
    }
    uint _id = _freelist.back();
    _freelist.pop_back();
    _pool[_id].ID = _id;
    _pool[_id].sceneID = _scene.id();
    Entity e = Entity(_id, _scene.id(), _pool[_id].versionID);
    return e;
}
bool epriv::ECSEntityPool::removeEntity(const uint& _id) {
    EntitySerialization _s(_id);
    uint index = _s.ID;
    ++_pool[index].versionID;
    _freelist.push_back(index);
}
bool epriv::ECSEntityPool::removeEntity(const Entity& _entity) { return removeEntity(_entity.data); }
epriv::EntityPOD* epriv::ECSEntityPool::getEntity(const uint& _id) {
    EntitySerialization _s(_id);
    if (_s.ID < _pool.size() && _pool[_s.ID].versionID == _s.versionID) {
        return &_pool[_s.ID];
    }
    return nullptr;
}
epriv::EntityPOD* epriv::ECSEntityPool::getEntity(const Entity& _entity) { return getEntity(_entity.data); }