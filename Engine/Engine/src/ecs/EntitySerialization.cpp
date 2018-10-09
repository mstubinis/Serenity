#include "ecs/EntitySerialization.h"
#include "ecs/Entity.h"

using namespace Engine;

void epriv::EntitySerialization::serialize(const uint& _data) {
    ID = _data & 0x200000;
    sceneID = _data >> 11;
    versionID = _data >> 4;
}
epriv::EntitySerialization::EntitySerialization(const uint& _data) { 
    serialize(_data); 
}
epriv::EntitySerialization::EntitySerialization(const Entity& _input) { 
    serialize(_input.data);
}
epriv::EntitySerialization::~EntitySerialization() { 
    ID = sceneID = versionID = 0; 
}



epriv::EntityPOD::EntityPOD() {
    ID = sceneID = versionID = 0;
}
epriv::EntityPOD::EntityPOD(uint _id) {
    ID = _id; sceneID = versionID = 0;
}
epriv::EntityPOD::EntityPOD(uint _id, Scene& _scene) {
    ID = _id; sceneID = _scene.id();  versionID = 0;
}
epriv::EntityPOD::~EntityPOD() {
    ID = sceneID = versionID = 0; 
}