#pragma once
#ifndef ENGINE_RESOURCES_INCLUDES_H
#define ENGINE_RESOURCES_INCLUDES_H

#include <serenity/system/Macros.h>

class ResourceType {
    public:
        enum Type : uint8_t {
            Unknown = 0,
            Texture,
            TextureCubemap,
            Mesh,
            MeshStatic,
            Material,
            Font,
            Shader,
            ShaderProgram,
            UniformBufferObject,
            SoundData,
        };
        BUILD_ENUM_CLASS_MEMBERS(ResourceType, Type)
};

#endif