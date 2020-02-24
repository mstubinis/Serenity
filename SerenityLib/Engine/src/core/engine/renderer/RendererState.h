#pragma once
#ifndef ENGINE_RENDERER_RENDERER_STATE_H
#define ENGINE_RENDERER_RENDERER_STATE_H

class  Mesh;
class  ShaderProgram;
class  Material;
struct RendererState final {
    Mesh*          current_bound_mesh           = nullptr;
    Material*      current_bound_material       = nullptr;
    ShaderProgram* current_bound_shader_program = nullptr;

    RendererState() = default;
};

#endif