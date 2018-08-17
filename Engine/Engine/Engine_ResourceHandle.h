#pragma once
#ifndef ENGINE_RESOURCE_HANDLE_H
#define ENGINE_RESOURCE_HANDLE_H

#include <cstdint>

typedef std::uint32_t uint32;

class ResourceType final{public: enum Type{
    Empty, //do NOT move this specific enum
    Texture,
    Mesh,
    Material,
    Sound,
    MeshInstance,
    Object,
    Font,
    Camera,
    Shader,
    ShaderProgram,
    SoundData,
    Scene,
_TOTAL};};

struct Handle final{
    uint32 index : 12;
    uint32 counter : 15;
    uint32 type : 5;
    Handle(){
        index = 0; counter = 0; type = 0;
    }
    Handle(uint32 _index, uint32 _counter, uint32 _type){
        index = _index; counter = _counter; type = _type;
    }
    inline operator uint32() const{
        return type << 27 | counter << 12 | index;
    }
    const bool null() const{ if (type == ResourceType::Empty) return true; return false; }
};

#endif