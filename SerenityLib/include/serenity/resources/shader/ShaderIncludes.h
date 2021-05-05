#pragma once
#ifndef ENGINE_SHADER_INCLDUES_H
#define ENGINE_SHADER_INCLDUES_H

#include <cstdint>

enum class ShaderType : uint8_t {
    Vertex = 0,             // GL_VERTEX_SHADER
    Fragment,               // GL_FRAGMENT_SHADER
    Geometry,               // GL_GEOMETRY_SHADER
    Compute,                // GL_COMPUTE_SHADER          (requires GL 4.3 or ARB_compute_shader)
    TessellationControl,    // GL_TESS_CONTROL_SHADER     (requires GL 4.0 or ARB_tessellation_shader)
    TessellationEvaluation, // GL_TESS_EVALUATION_SHADER  (requires GL 4.0 or ARB_tessellation_shader)
    _TOTAL,
};

#endif