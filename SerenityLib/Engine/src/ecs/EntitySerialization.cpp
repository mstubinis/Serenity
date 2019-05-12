#include <ecs/EntitySerialization.h>
#include <ecs/Entity.h>

using namespace Engine;

epriv::EntitySerialization::EntitySerialization(const uint& _data) { serialize(_data); }
epriv::EntitySerialization::EntitySerialization(const Entity& _input) { serialize(_input.data); }
epriv::EntitySerialization::EntitySerialization(epriv::EntitySerialization&& other) noexcept {
    using std::swap;
    if (&other != this) {
        ID = other.ID;
        sceneID = other.sceneID;
        versionID = other.versionID;
    }
}
epriv::EntitySerialization& epriv::EntitySerialization::operator=(epriv::EntitySerialization&& other) noexcept {
    using std::swap;
    if (&other != this) {
        ID = other.ID;
        sceneID = other.sceneID;
        versionID = other.versionID;
    }
    return *this;
}


epriv::EntityPOD::EntityPOD(uint _id, Scene& _scene) { ID = _id; sceneID = _scene.id();  versionID = 0; }
epriv::EntityPOD::EntityPOD(uint _id, uint _sceneID) { ID = _id; sceneID = _sceneID;  versionID = 0; }
epriv::EntityPOD::EntityPOD(epriv::EntityPOD&& other) noexcept {
    using std::swap;
    if (&other != this) {
        ID = other.ID;
        sceneID = other.sceneID;
        versionID = other.versionID;
    }
}
epriv::EntityPOD& epriv::EntityPOD::operator=(epriv::EntityPOD&& other) noexcept {
    using std::swap;
    if (&other != this) {
        ID = other.ID;
        sceneID = other.sceneID;
        versionID = other.versionID;
    }
    return *this;
}