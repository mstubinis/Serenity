#pragma once
#ifndef ENGINE_RESOURCE_HANDLE_INCLUDE_GUARD
#define ENGINE_RESOURCE_HANDLE_INCLUDE_GUARD

#include <core/engine/Engine_Utils.h>

class  EngineResource;

struct ResourceType final {enum Type {
    Empty, //do NOT move this specific enum
    Texture,
    Mesh,
    Material,
    Sound,
    Object,
    Font,
    Camera,
    Shader,
    ShaderProgram,
    SoundData,
    Scene,
    _TOTAL
};};

struct Handle final {
    uint index : 12;
    uint version : 15;
    uint type : 5;

    explicit Handle();
    explicit Handle(uint _index, uint _version, uint _type);
    operator uint() const;
    const bool null() const;
    const EngineResource* get() const;
    const EngineResource* operator ->() const;
};

#endif