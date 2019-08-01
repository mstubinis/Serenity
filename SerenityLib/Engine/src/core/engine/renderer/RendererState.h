#pragma once
#ifndef ENGINE_RENDERER_RENDERER_STATE_H
#define ENGINE_RENDERER_RENDERER_STATE_H

class  Mesh;
class  ShaderProgram;
class  Material;
struct RendererState final {
    Mesh* current_bound_mesh;
    Material* current_bound_material;
    ShaderProgram* current_bound_shader_program;
    RendererState() {
        current_bound_mesh = nullptr;
        current_bound_material = nullptr;
        current_bound_shader_program = nullptr;
    }
};

#endif