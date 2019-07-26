#pragma once
#ifndef ENGINE_RESOURCE_HANDLE_INCLUDE_GUARD
#define ENGINE_RESOURCE_HANDLE_INCLUDE_GUARD

#include <core/engine/utils/Utils.h>

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
    uint32_t index : 12;
    uint32_t version : 15;
    uint32_t type : 5;

    explicit Handle();
    explicit Handle(uint32_t _index, uint32_t _version, uint32_t _type);
    operator uint32_t() const;
    const bool null() const;
    const EngineResource* get() const;
    const EngineResource* operator ->() const;
};

#endif