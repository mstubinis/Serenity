#include "ecs/EntitySerialization.h"
#include "ecs/Entity.h"

using namespace Engine;

/*
uint extractInt(const uint& origBitWord, unsigned from, unsigned to){
    unsigned mask = ((1 << (to - from + 1)) - 1) << from;
    return (origBitWord & mask) >> from;
}
*/
void epriv::EntitySerialization::serialize(const uint& _data) {
    //ID = extractInt(_data,0,21);
    //sceneID = extractInt(_data,21,28);
    //versionID = extractInt(_data,28,32);

    ID        = (_data & 0x003FFFFF) >> 0;
    sceneID   = (_data & 0x1FE00000) >> 21;
    versionID = (_data & 4026531840) >> 28;
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