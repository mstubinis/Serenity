#pragma once
#ifndef ENGINE_RENDERER_ENUMS_H
#define ENGINE_RENDERER_ENUMS_H

#include <gl/glew.h>
#include <SFML/OpenGL.hpp>

struct SortingMode final { enum Mode {
    None,
    FrontToBack,
    BackToFront,
};};

struct DepthFunc final { enum Func {
    Never = GL_NEVER,
    Less = GL_LESS,
    Equal = GL_EQUAL,
    LEqual = GL_LEQUAL,
    Greater = GL_GREATER,
    NotEqual = GL_NOTEQUAL,
    GEqual = GL_GEQUAL,
    Always = GL_ALWAYS,
};};
struct AntiAliasingAlgorithm final { enum Algorithm {
    None, FXAA, SMAA,
};};

struct Alignment final { enum Type {
    TopLeft,
    TopCenter,
    TopRight,
    Left,
    Center,
    Right,
    BottomLeft,
    BottomCenter,
    BottomRight,
_TOTAL};};

struct RenderStage { enum Stage {
    GeometryOpaque,
    GeometryTransparent,
    GeometryTransparentTrianglesSorted,
    ForwardOpaque,
    ForwardTransparent,
    ForwardTransparentTrianglesSorted,
    ForwardParticles,
_TOTAL};};

#endif