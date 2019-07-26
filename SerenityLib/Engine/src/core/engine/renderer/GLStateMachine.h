#pragma once
#ifndef ENGINE_GL_STATE_MACHINE_H
#define ENGINE_GL_STATE_MACHINE_H

#include <vector>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
class Material;
class Mesh;
class ShaderP;

namespace Engine {
    namespace epriv {
        struct GLStateMachineDataCustom final {
            GLuint              current_bound_buffer_vbo;
            GLuint              current_bound_buffer_ebo;

            GLuint              current_bound_read_fbo;
            GLuint              current_bound_draw_fbo;
            GLuint              current_bound_rbo;

            ShaderP*            current_bound_shader_program;
            Material*           current_bound_material;
            Mesh*               current_bound_mesh;

            GLStateMachineDataCustom();
            ~GLStateMachineDataCustom();
            void init();

            GLStateMachineDataCustom(const GLStateMachineDataCustom& other) = delete;
            GLStateMachineDataCustom& operator=(const GLStateMachineDataCustom& other) = delete;
            GLStateMachineDataCustom(GLStateMachineDataCustom&& other) noexcept = delete;
            GLStateMachineDataCustom& operator=(GLStateMachineDataCustom&& other) noexcept = delete;
        };
    };
};

#endif