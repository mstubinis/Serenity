#pragma once
#ifndef ENGINE_RENDERER_ENUMS_H
#define ENGINE_RENDERER_ENUMS_H

#include <gl/glew.h>
#include <SFML/OpenGL.hpp>

constexpr const glm::vec4 NO_SCISSOR = glm::vec4(-1.0f);

enum class SortingMode : uint32_t {
    None = 0,
    FrontToBack,
    BackToFront,
};
enum class DepthFunc : uint32_t {
    Never    = GL_NEVER,
    Less     = GL_LESS,
    Equal    = GL_EQUAL,
    LEqual   = GL_LEQUAL,
    Greater  = GL_GREATER,
    NotEqual = GL_NOTEQUAL,
    GEqual   = GL_GEQUAL,
    Always   = GL_ALWAYS,
};
enum class AntiAliasingAlgorithm : uint32_t {
    None = 0, 
    FXAA, 
    SMAA_LOW,
    SMAA_MED,
    SMAA_HIGH,
    SMAA_ULTRA,
};
enum class Alignment : uint32_t {
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
enum class RenderStage : uint32_t {
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

_TOTAL};

#endif