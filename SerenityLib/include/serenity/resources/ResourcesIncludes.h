#pragma once
#ifndef ENGINE_RESOURCES_INCLUDES_H
#define ENGINE_RESOURCES_INCLUDES_H

enum class ResourceType : unsigned char {
    Unknown = 0,
    Texture,
    Mesh,
    Material,
    Font,
    Shader,
    ShaderProgram,
    UniformBufferObject,
    SoundData,
};

#endif