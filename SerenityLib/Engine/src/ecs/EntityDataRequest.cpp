#include <ecs/EntityDataRequest.h>
#include <ecs/Entity.h>

using namespace Engine;
using namespace Engine::epriv;

EntityDataRequest::EntityDataRequest(const unsigned int& data) {
    serialize(data); 
}
EntityDataRequest::EntityDataRequest(const Entity& entity) {
    serialize(entity.data);
}
EntityDataRequest::EntityDataRequest(EntityDataRequest&& other) noexcept {
    ID        = other.ID;
    sceneID   = other.sceneID;
    versionID = other.versionID;
}
EntityDataRequest& EntityDataRequest::operator=(EntityDataRequest&& other) noexcept {
    if (&other != this) {
        ID        = other.ID;
        sceneID   = other.sceneID;
        versionID = other.versionID;
    }
    return *this;
}

void EntityDataRequest::serialize(const unsigned int& entityData) {
    ID        = (entityData & 4194303)    >> 0;  //   = 21    (2 ^ 22 = 419,304)
    sceneID   = (entityData & 534773760)  >> 21; //+7 = 28    (2 ^ 29 = 536,870,912)
    versionID = (entityData & 4026531840) >> 28; //+4 = 32    (2 ^ 32 = 4,294,967,296)
}

EntityPOD::EntityPOD(const unsigned int& entityID, Scene& scene) {
    ID        = entityID;
    sceneID   = scene.id();
    versionID = 0; 
}
EntityPOD::EntityPOD(const unsigned int& entityID, const unsigned int& _sceneID) {
    ID        = entityID;
    sceneID   = _sceneID;
    versionID = 0; 
}
EntityPOD::EntityPOD(EntityPOD&& other) noexcept {
    using std::swap;
    ID        = other.ID;
    sceneID   = other.sceneID;
    versionID = other.versionID;
}
EntityPOD& epriv::EntityPOD::operator=(EntityPOD&& other) noexcept {
    if (&other != this) {
        ID        = other.ID;
        sceneID   = other.sceneID;
        versionID = other.versionID;
    }
    return *this;
}