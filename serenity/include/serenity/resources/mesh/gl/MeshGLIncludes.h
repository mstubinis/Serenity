#pragma once
#ifndef ENGINE_GL_MESH_INCLUDES
#define ENGINE_GL_MESH_INCLUDES

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <vector>
#include <serenity/system/TypeDefs.h>
#include <serenity/system/Macros.h>

class BufferDataDrawType final {
    public:
        enum Type : uint32_t {
            Unassigned = 0,
            Static = GL_STATIC_DRAW,
            Dynamic = GL_DYNAMIC_DRAW,
            Stream = GL_STREAM_DRAW,
        };
        BUILD_ENUM_CLASS_MEMBERS(BufferDataDrawType, Type)
};

#endif