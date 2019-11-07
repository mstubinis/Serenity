#pragma once
#ifndef ENGINE_RESOURCES_INCLUDES_H
#define ENGINE_RESOURCES_INCLUDES_H

struct ResourceType final { enum Type {
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
    _TOTAL,
};};

#endif