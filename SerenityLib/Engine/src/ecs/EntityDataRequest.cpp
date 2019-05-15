#include <ecs/EntityDataRequest.h>
#include <ecs/Entity.h>

using namespace Engine;

EntityDataRequest::EntityDataRequest(const uint& _data) {
    serialize(_data); 
}
EntityDataRequest::EntityDataRequest(const Entity& _entity) {
    serialize(_entity.data);
}
EntityDataRequest::EntityDataRequest(EntityDataRequest&& other) noexcept {
    //using std::swap;
    if (&other != this) {
        ID = other.ID;
        sceneID = other.sceneID;
        versionID = other.versionID;
    }
}
EntityDataRequest& EntityDataRequest::operator=(EntityDataRequest&& other) noexcept {
    //using std::swap;
    if (&other != this) {
        ID = other.ID;
        sceneID = other.sceneID;
        versionID = other.versionID;
    }
    return *this;
}


epriv::EntityPOD::EntityPOD(uint _id, Scene& _scene) { 
    ID = _id;
    sceneID = _scene.id();
    versionID = 0; 
}
epriv::EntityPOD::EntityPOD(uint _id, uint _sceneID) { 
    ID = _id;
    sceneID = _sceneID;
    versionID = 0; 
}
epriv::EntityPOD::EntityPOD(epriv::EntityPOD&& other) noexcept {
    //using std::swap;
    if (&other != this) {
        ID = other.ID;
        sceneID = other.sceneID;
        versionID = other.versionID;
    }
}
epriv::EntityPOD& epriv::EntityPOD::operator=(epriv::EntityPOD&& other) noexcept {
    //using std::swap;
    if (&other != this) {
        ID = other.ID;
        sceneID = other.sceneID;
        versionID = other.versionID;
    }
    return *this;
}