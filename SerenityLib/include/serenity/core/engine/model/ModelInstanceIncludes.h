#pragma once
#ifndef ENGINE_MODEL_INSTANCE_INCLUDES_H
#define ENGINE_MODEL_INSTANCE_INCLUDES_H

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

enum class ModelDrawingMode {
    Triangles              = GL_TRIANGLES,
    TriangleStrip          = GL_TRIANGLE_STRIP,
    TriangleFan            = GL_TRIANGLE_FAN,
    Points                 = GL_POINTS,
    LineStrip              = GL_LINE_STRIP,
    LineLoop               = GL_LINE_LOOP,
    Lines                  = GL_LINES,
    Patches                = GL_PATCHES,

    //GL_LINE_STRIP_ADJACENCY, GL_LINES_ADJACENCY, GL_TRIANGLE_STRIP_ADJACENCY and GL_TRIANGLES_ADJACENCY are available only if the GL version is 3.2 or greater.
    TriangleStripAdjacency = GL_TRIANGLE_STRIP_ADJACENCY,
    TrianglesAdjacency     = GL_TRIANGLES_ADJACENCY,
    LineStripAdjacency     = GL_LINE_STRIP_ADJACENCY,
    LinesAdjacency         = GL_LINES_ADJACENCY,
};

#endif