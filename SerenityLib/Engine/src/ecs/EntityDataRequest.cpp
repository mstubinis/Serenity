#include <ecs/EntityDataRequest.h>
#include <ecs/Entity.h>

using namespace Engine;
using namespace Engine::priv;

#pragma region EntityDataRequest

EntityDataRequest::EntityDataRequest(std::uint32_t data) {
    serialize(data); 
}
EntityDataRequest::EntityDataRequest(Entity entity) {
    serialize(entity.m_Data);
}
EntityDataRequest::~EntityDataRequest() {

}
EntityDataRequest::EntityDataRequest(EntityDataRequest&& other) noexcept {
    ID        = (other.ID);
    sceneID   = (other.sceneID);
    versionID = (other.versionID);

    other.ID         = 0U;
    other.sceneID    = 0U;
    other.versionID  = 0U;
}
EntityDataRequest& EntityDataRequest::operator=(EntityDataRequest&& other) noexcept {
    if (&other != this) {
        ID        = (other.ID);
        sceneID   = (other.sceneID);
        versionID = (other.versionID);

        other.ID         = 0U;
        other.sceneID    = 0U;
        other.versionID  = 0U;
    }
    return *this;
}
void EntityDataRequest::serialize(std::uint32_t entityData) {
    ID        = (entityData & 4'194'303U)     >> (ENTITY_SIZE - VERSION_BIT_POSITIONS - SCENE_BIT_POSITIONS - ID_BIT_POSITIONS);  //   = 21    (2 ^ 22 = 419,304)
    sceneID   = (entityData & 534'773'760U)   >> (ENTITY_SIZE - VERSION_BIT_POSITIONS - SCENE_BIT_POSITIONS);                     //+7 = 28    (2 ^ 29 = 536,870,912)
    versionID = (entityData & 4'026'531'840U) >> (ENTITY_SIZE - VERSION_BIT_POSITIONS);                                           //+4 = 32    (2 ^ 32 = 4,294,967,296)
}
#pragma endregion
