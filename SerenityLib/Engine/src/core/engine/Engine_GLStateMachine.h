#pragma once
#ifndef ENGINE_GL_STATE_MACHINE_H
#define ENGINE_GL_STATE_MACHINE_H

#include <vector>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <boost/function.hpp>

typedef boost::function<void()> boost_func;

class Material;
class Mesh;
class ShaderP;

namespace Engine {
namespace epriv {
    struct GLStateMachineDataCustom final {
        GLuint              current_bound_buffer_vbo;
        GLuint              current_bound_buffer_ebo;
        GLuint              current_bound_vao;

        GLuint              current_bound_read_fbo;
        GLuint              current_bound_draw_fbo;
        GLuint              current_bound_rbo;
        GLuint              current_bound_texture_1D;
        GLuint              current_bound_texture_2D;
        GLuint              current_bound_texture_3D;
        GLuint              current_bound_texture_cube_map;

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
    struct GLStateData final {
        bool enabled;
        boost_func enableFunc, disableFunc;
        GLStateData() {}
        GLStateData(const bool& _enabled, const boost_func& _enableFunc, const boost_func& _disableFunc) {
            enabled = _enabled;
            enableFunc = _enableFunc;
            disableFunc = _disableFunc;
        }
    };
};
};
struct GLState final {
    enum State {
        TEXTURE_1D,
        TEXTURE_2D,
        TEXTURE_3D,
        CULL_FACE,
        ALPHA_TEST,
        DEPTH_TEST,
        DEPTH_CLAMP,
        STENCIL_TEST,
        TEXTURE_CUBE_MAP_SEAMLESS,

        BLEND,
        BLEND_0,
        BLEND_1,
        BLEND_2,
        BLEND_3,
        BLEND_4,
        BLEND_5,
        BLEND_6,
        BLEND_7,
        BLEND_8,
        BLEND_9,
        BLEND_10,
        BLEND_11,
        BLEND_12,
        BLEND_13,
        BLEND_14,
        BLEND_15,

        DEPTH_MASK, DITHER,
        SCISSOR_TEST,
        _TOTAL
    };
    static std::vector<Engine::epriv::GLStateData> SM;
};

namespace Engine{
namespace Renderer{
    inline       void GLEnable(const GLState::State s){ auto& t= GLState::SM[s];if(t.enabled)return;t.enableFunc();t.enabled=1; }
    inline       void GLDisable(const GLState::State s){ auto& t= GLState::SM[s];if(!t.enabled)return;t.disableFunc();t.enabled=0; }
    inline const bool GLEnabled(const GLState::State s){ auto& t= GLState::SM[s];return t.enabled; }
    inline const bool GLDisabled(const GLState::State s){ auto& t= GLState::SM[s];return !t.enabled; }
    inline       void RestoreGLState(){ for(auto& t: GLState::SM){ t.enabled? t.enableFunc() : t.disableFunc(); } }
};
};

#endif