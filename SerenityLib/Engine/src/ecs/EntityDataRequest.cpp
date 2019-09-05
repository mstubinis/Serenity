#include <ecs/EntityDataRequest.h>
#include <ecs/Entity.h>

using namespace Engine;
using namespace Engine::epriv;

EntityDataRequest::EntityDataRequest(const uint& data) {
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
/*
            ID : 21; //2,097,152  max
       sceneID : 7;  //128        max
     versionID : 4;  //16         max
*/
void EntityDataRequest::serialize(const uint& entityData) {
    /*
    auto lambdaExtract = [&](uint number, uint k, uint p) {
        return ((1 << k) - 1) & (number >> (p - 1));
    };
    ID        = lambdaExtract(entityData, 21, 1);
    sceneID   = lambdaExtract(entityData, 7, 22);
    versionID = lambdaExtract(entityData, 4, 29);
    */
    ID        = (entityData & 4194303)    >> 0;  //   = 21    (2 ^ 22 = 419,304)
    sceneID   = (entityData & 534773760)  >> 21; //+7 = 28    (2 ^ 29 = 536,870,912)
    versionID = (entityData & 4026531840) >> 28; //+4 = 32    (2 ^ 32 = 4,294,967,296)
}



EntityPOD::EntityPOD(const uint& entityID, Scene& scene) {
    ID        = entityID;
    sceneID   = scene.id();
    versionID = 0; 
}
EntityPOD::EntityPOD(const uint& entityID, const uint& _sceneID) {
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