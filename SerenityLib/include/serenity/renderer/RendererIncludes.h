#pragma once
#ifndef ENGINE_RENDERER_ENUMS_H
#define ENGINE_RENDERER_ENUMS_H

#include <gl/glew.h>
#include <SFML/OpenGL.hpp>
#include <serenity/system/TypeDefs.h>
#include <serenity/dependencies/glm.h>
#include <serenity/system/Macros.h>

constexpr const glm::vec4 NO_SCISSOR = glm::vec4{ -1.0f };

struct ScreenBoxCoordinates {
    bool      inBounds    = false;
    glm::vec2 topLeft     = glm::vec2{ 0.0f, 0.0f };
    glm::vec2 topRight    = glm::vec2{ 0.0f, 0.0f };
    glm::vec2 bottomLeft  = glm::vec2{ 0.0f, 0.0f };
    glm::vec2 bottomRight = glm::vec2{ 0.0f, 0.0f };
};

class SortingMode {
    public:
        enum Type : uint32_t {
            None = 0,
            FrontToBack,
            BackToFront,
        };
        BUILD_ENUM_CLASS_MEMBERS(SortingMode, Type)
};
class DepthFunc {
    public:
        enum Type : uint32_t {
            Never = GL_NEVER,
            Less = GL_LESS,
            Equal = GL_EQUAL,
            LEqual = GL_LEQUAL,
            Greater = GL_GREATER,
            NotEqual = GL_NOTEQUAL,
            GEqual = GL_GEQUAL,
            Always = GL_ALWAYS,
        };
        BUILD_ENUM_CLASS_MEMBERS(DepthFunc, Type)
};
class AntiAliasingAlgorithm {
    public:
        enum Type : uint32_t {
            None = 0,
            FXAA,
            SMAA_LOW,
            SMAA_MED,
            SMAA_HIGH,
            SMAA_ULTRA,
        };
        BUILD_ENUM_CLASS_MEMBERS(AntiAliasingAlgorithm, Type)
};
class LightingAlgorithm {
    public:
        enum Type : uint32_t {
            Basic = 0,
            PBR,
        };
        BUILD_ENUM_CLASS_MEMBERS(LightingAlgorithm, Type)
};
class Alignment {
    public:
        enum Type : uint32_t {
            TopLeft = 0,
            TopCenter,
            TopRight,
            Left,
            Center,
            Right,
            BottomLeft,
            BottomCenter,
            BottomRight,
        };
        BUILD_ENUM_CLASS_MEMBERS(Alignment, Type)
};
class RenderStage { 
    public: 
        enum Type : uint32_t {
            GeometryOpaque = 0,
            GeometryOpaque_2,
            GeometryOpaque_3,
            GeometryOpaque_4,

            GeometryTransparent,
            GeometryTransparent_2,
            GeometryTransparent_3,
            GeometryTransparent_4,

            GeometryTransparentTrianglesSorted,
            GeometryTransparentTrianglesSorted_2,
            GeometryTransparentTrianglesSorted_3,
            GeometryTransparentTrianglesSorted_4,

            ForwardOpaque,
            ForwardOpaque_2,
            ForwardOpaque_3,
            ForwardOpaque_4,

            ForwardTransparent,
            ForwardTransparent_2,
            ForwardTransparent_3,
            ForwardTransparent_4,

            ForwardTransparentTrianglesSorted,
            ForwardTransparentTrianglesSorted_2,
            ForwardTransparentTrianglesSorted_3,
            ForwardTransparentTrianglesSorted_4,

            ForwardParticles,
            ForwardParticles_2,
            ForwardParticles_3,
            ForwardParticles_4,

            Decals,
            Decals_2,
            Decals_3,
            Decals_4,

            _TOTAL,
        };
        [[nodiscard]] const char* toString() const noexcept;
        BUILD_ENUM_CLASS_MEMBERS(RenderStage, Type)
};

#endif