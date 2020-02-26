#include <ecs/EntityDataRequest.h>
#include <ecs/Entity.h>

using namespace Engine;
using namespace Engine::priv;

#pragma region EntityDataRequest

EntityDataRequest::EntityDataRequest(const unsigned int data) {
    serialize(data); 
}
EntityDataRequest::EntityDataRequest(const Entity& entity) {
    serialize(entity.data);
}
EntityDataRequest::~EntityDataRequest() {

}
EntityDataRequest::EntityDataRequest(EntityDataRequest&& other) noexcept {
    ID        = (other.ID);
    sceneID   = (other.sceneID);
    versionID = (other.versionID);

    other.ID         = 0;
    other.sceneID    = 0;
    other.versionID  = 0;
}
EntityDataRequest& EntityDataRequest::operator=(EntityDataRequest&& other) noexcept {
    if (&other != this) {
        ID        = (other.ID);
        sceneID   = (other.sceneID);
        versionID = (other.versionID);

        other.ID         = 0;
        other.sceneID    = 0;
        other.versionID  = 0;
    }
    return *this;
}
void EntityDataRequest::serialize(const unsigned int entityData) {
    ID        = (entityData & 4'194'303)    >> 0;  //   = 21    (2 ^ 22 = 419,304)
    sceneID   = (entityData & 534'773'760)  >> 21; //+7 = 28    (2 ^ 29 = 536,870,912)
    versionID = (entityData & 4'026'531'840) >> 28; //+4 = 32    (2 ^ 32 = 4,294,967,296)
}
#pragma endregion


#pragma region EntityPOD

EntityPOD::EntityPOD(const unsigned int entityID, Scene& scene) {
    ID        = entityID;
    sceneID   = scene.id();
    versionID = 0; 
}
EntityPOD::EntityPOD(const unsigned int entityID, const unsigned int sceneID_) {
    ID        = entityID;
    sceneID   = sceneID_;
    versionID = 0; 
}
EntityPOD::~EntityPOD() {

}

EntityPOD::EntityPOD(EntityPOD&& other) noexcept {
    ID        = (other.ID);
    sceneID   = (other.sceneID);
    versionID = (other.versionID);

    other.ID         = 0;
    other.sceneID    = 0;
    other.versionID  = 0;
}
EntityPOD& priv::EntityPOD::operator=(EntityPOD&& other) noexcept {
    if (&other != this) {
        ID        = (other.ID);
        sceneID   = (other.sceneID);
        versionID = (other.versionID);

        other.ID         = 0;
        other.sceneID    = 0;
        other.versionID  = 0;
    }
    return *this;
}
#pragma endregion