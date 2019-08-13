#include <ecs/EntityDataRequest.h>
#include <ecs/Entity.h>

using namespace Engine;

EntityDataRequest::EntityDataRequest(const uint& data) {
    serialize(data); 
}
EntityDataRequest::EntityDataRequest(const Entity& entity) {
    serialize(entity.data);
}
EntityDataRequest::EntityDataRequest(EntityDataRequest&& other) noexcept {
    if (&other != this) {
        ID        = other.ID;
        sceneID   = other.sceneID;
        versionID = other.versionID;
    }
}
EntityDataRequest& EntityDataRequest::operator=(EntityDataRequest&& other) noexcept {
    if (&other != this) {
        ID        = other.ID;
        sceneID   = other.sceneID;
        versionID = other.versionID;
    }
    return *this;
}
epriv::EntityPOD::EntityPOD(const uint& entityID, Scene& scene) {
    ID        = entityID;
    sceneID   = scene.id();
    versionID = 0; 
}
epriv::EntityPOD::EntityPOD(const uint& entityID, const uint& _sceneID) {
    ID        = entityID;
    sceneID   = _sceneID;
    versionID = 0; 
}
epriv::EntityPOD::EntityPOD(epriv::EntityPOD&& other) noexcept {
    //using std::swap;
    if (&other != this) {
        ID        = other.ID;
        sceneID   = other.sceneID;
        versionID = other.versionID;
    }
}
epriv::EntityPOD& epriv::EntityPOD::operator=(epriv::EntityPOD&& other) noexcept {
    if (&other != this) {
        ID        = other.ID;
        sceneID   = other.sceneID;
        versionID = other.versionID;
    }
    return *this;
}