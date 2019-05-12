#include <core/engine/Engine_GLStateMachine.h>
#include <boost/bind.hpp>

using namespace Engine;
using namespace std;
namespace B = ::boost;

typedef unsigned int uint;

void epriv::GLStateMachineDataCustom::init() {
    current_bound_buffer_vbo = 0;
    current_bound_buffer_ebo = 0;

    current_bound_vao = 0;
    current_bound_read_fbo = 0;
    current_bound_draw_fbo = 0;
    current_bound_rbo = 0;
    current_bound_texture_1D = 0;
    current_bound_texture_2D = 0;
    current_bound_texture_3D = 0;
    current_bound_texture_cube_map = 0;

    current_bound_shader_program = nullptr;
    current_bound_material = nullptr;
    current_bound_mesh = nullptr;
}
epriv::GLStateMachineDataCustom::GLStateMachineDataCustom() { init(); }
epriv::GLStateMachineDataCustom::~GLStateMachineDataCustom() { init(); }

void gl_enable_blend() {
    Renderer::GLEnable(GLState::BLEND_0);
    Renderer::GLEnable(GLState::BLEND_1);
    Renderer::GLEnable(GLState::BLEND_2);
    Renderer::GLEnable(GLState::BLEND_3);
    Renderer::GLEnable(GLState::BLEND_4);
    Renderer::GLEnable(GLState::BLEND_5);
    Renderer::GLEnable(GLState::BLEND_6);
    Renderer::GLEnable(GLState::BLEND_7);
    Renderer::GLEnable(GLState::BLEND_8);
    Renderer::GLEnable(GLState::BLEND_9);
    Renderer::GLEnable(GLState::BLEND_10);
    Renderer::GLEnable(GLState::BLEND_11);
    Renderer::GLEnable(GLState::BLEND_12);
    Renderer::GLEnable(GLState::BLEND_13);
    Renderer::GLEnable(GLState::BLEND_14);
    Renderer::GLEnable(GLState::BLEND_15);
}
void gl_disable_blend() {
    Renderer::GLDisable(GLState::BLEND_0);
    Renderer::GLDisable(GLState::BLEND_1);
    Renderer::GLDisable(GLState::BLEND_2);
    Renderer::GLDisable(GLState::BLEND_3);
    Renderer::GLDisable(GLState::BLEND_4);
    Renderer::GLDisable(GLState::BLEND_5);
    Renderer::GLDisable(GLState::BLEND_6);
    Renderer::GLDisable(GLState::BLEND_7);
    Renderer::GLDisable(GLState::BLEND_8);
    Renderer::GLDisable(GLState::BLEND_9);
    Renderer::GLDisable(GLState::BLEND_10);
    Renderer::GLDisable(GLState::BLEND_11);
    Renderer::GLDisable(GLState::BLEND_12);
    Renderer::GLDisable(GLState::BLEND_13);
    Renderer::GLDisable(GLState::BLEND_14);
    Renderer::GLDisable(GLState::BLEND_15);
}
inline void gl_enable_0(const uint& _enum) { glEnablei(_enum, 0); }
inline void gl_enable_1(const uint& _enum) { glEnablei(_enum, 1); }
inline void gl_enable_2(const uint& _enum) { glEnablei(_enum, 2); }
inline void gl_enable_3(const uint& _enum) { glEnablei(_enum, 3); }
inline void gl_enable_4(const uint& _enum) { glEnablei(_enum, 4); }
inline void gl_enable_5(const uint& _enum) { glEnablei(_enum, 5); }
inline void gl_enable_6(const uint& _enum) { glEnablei(_enum, 6); }
inline void gl_enable_7(const uint& _enum) { glEnablei(_enum, 7); }
inline void gl_enable_8(const uint& _enum) { glEnablei(_enum, 8); }
inline void gl_enable_9(const uint& _enum) { glEnablei(_enum, 9); }
inline void gl_enable_10(const uint& _enum) { glEnablei(_enum, 10); }
inline void gl_enable_11(const uint& _enum) { glEnablei(_enum, 11); }
inline void gl_enable_12(const uint& _enum) { glEnablei(_enum, 12); }
inline void gl_enable_13(const uint& _enum) { glEnablei(_enum, 13); }
inline void gl_enable_14(const uint& _enum) { glEnablei(_enum, 14); }
inline void gl_enable_15(const uint& _enum) { glEnablei(_enum, 15); }

inline void gl_disable_0(const uint& _enum) { glDisablei(_enum, 0); }
inline void gl_disable_1(const uint& _enum) { glDisablei(_enum, 1); }
inline void gl_disable_2(const uint& _enum) { glDisablei(_enum, 2); }
inline void gl_disable_3(const uint& _enum) { glDisablei(_enum, 3); }
inline void gl_disable_4(const uint& _enum) { glDisablei(_enum, 4); }
inline void gl_disable_5(const uint& _enum) { glDisablei(_enum, 5); }
inline void gl_disable_6(const uint& _enum) { glDisablei(_enum, 6); }
inline void gl_disable_7(const uint& _enum) { glDisablei(_enum, 7); }
inline void gl_disable_8(const uint& _enum) { glDisablei(_enum, 8); }
inline void gl_disable_9(const uint& _enum) { glDisablei(_enum, 9); }
inline void gl_disable_10(const uint& _enum) { glDisablei(_enum, 10); }
inline void gl_disable_11(const uint& _enum) { glDisablei(_enum, 11); }
inline void gl_disable_12(const uint& _enum) { glDisablei(_enum, 12); }
inline void gl_disable_13(const uint& _enum) { glDisablei(_enum, 13); }
inline void gl_disable_14(const uint& _enum) { glDisablei(_enum, 14); }
inline void gl_disable_15(const uint& _enum) { glDisablei(_enum, 15); }


void _add(vector<epriv::GLStateData>& v,uint key, const epriv::GLStateData& value){ v[key] = value; }

vector<epriv::GLStateData> GLState::SM = [](){
    vector<epriv::GLStateData> m; m.resize(GLState::_TOTAL);
                                        //enabled?              //enableGLFunc                      //disableGLFunc
    _add(m,GLState::TEXTURE_1D, epriv::GLStateData(false,B::bind<void>(glEnable,GL_TEXTURE_1D),B::bind<void>(glDisable,GL_TEXTURE_1D)));
    _add(m,GLState::TEXTURE_1D, epriv::GLStateData(false,B::bind<void>(glEnable,GL_TEXTURE_1D),B::bind<void>(glDisable,GL_TEXTURE_1D)));
    _add(m,GLState::TEXTURE_2D, epriv::GLStateData(false,B::bind<void>(glEnable,GL_TEXTURE_2D),B::bind<void>(glDisable,GL_TEXTURE_2D)));
    _add(m,GLState::TEXTURE_3D, epriv::GLStateData(false,B::bind<void>(glEnable,GL_TEXTURE_3D),B::bind<void>(glDisable,GL_TEXTURE_3D)));
    _add(m,GLState::CULL_FACE, epriv::GLStateData(false,B::bind<void>(glEnable,GL_CULL_FACE),B::bind<void>(glDisable,GL_CULL_FACE)));
    _add(m,GLState::ALPHA_TEST, epriv::GLStateData(false,B::bind<void>(glEnable,GL_ALPHA_TEST),B::bind<void>(glDisable,GL_ALPHA_TEST)));
    _add(m,GLState::DEPTH_TEST, epriv::GLStateData(true,B::bind<void>(glEnable,GL_DEPTH_TEST),B::bind<void>(glDisable,GL_DEPTH_TEST)));
    _add(m,GLState::DEPTH_CLAMP, epriv::GLStateData(true,B::bind<void>(glEnable,GL_DEPTH_CLAMP),B::bind<void>(glDisable,GL_DEPTH_CLAMP)));
    _add(m,GLState::STENCIL_TEST, epriv::GLStateData(false,B::bind<void>(glEnable,GL_STENCIL_TEST),B::bind<void>(glDisable,GL_STENCIL_TEST)));
    _add(m,GLState::TEXTURE_CUBE_MAP_SEAMLESS, epriv::GLStateData(false,B::bind<void>(glEnable,GL_TEXTURE_CUBE_MAP_SEAMLESS),B::bind<void>(glDisable,GL_TEXTURE_CUBE_MAP_SEAMLESS) ));
    _add(m,GLState::DEPTH_MASK, epriv::GLStateData(true,B::bind<void>(glDepthMask,GL_TRUE),B::bind<void>(glDepthMask,GL_FALSE)));
    _add(m,GLState::DITHER, epriv::GLStateData(false,B::bind<void>(glEnable,GL_DITHER),B::bind<void>(glDisable,GL_DITHER)));
    _add(m,GLState::SCISSOR_TEST, epriv::GLStateData(false,B::bind<void>(glEnable,GL_SCISSOR_TEST),B::bind<void>(glDisable,GL_SCISSOR_TEST)));

    _add(m, GLState::BLEND, epriv::GLStateData(false, B::bind<void>(gl_enable_blend), B::bind<void>(gl_disable_blend)));
    _add(m, GLState::BLEND_0, epriv::GLStateData(false, B::bind<void>(gl_enable_0, GL_BLEND), B::bind<void>(gl_disable_0, GL_BLEND)));
    _add(m, GLState::BLEND_1, epriv::GLStateData(false, B::bind<void>(gl_enable_1, GL_BLEND), B::bind<void>(gl_disable_1, GL_BLEND)));
    _add(m, GLState::BLEND_2, epriv::GLStateData(false, B::bind<void>(gl_enable_2, GL_BLEND), B::bind<void>(gl_disable_2, GL_BLEND)));
    _add(m, GLState::BLEND_3, epriv::GLStateData(false, B::bind<void>(gl_enable_3, GL_BLEND), B::bind<void>(gl_disable_3, GL_BLEND)));
    _add(m, GLState::BLEND_4, epriv::GLStateData(false, B::bind<void>(gl_enable_4, GL_BLEND), B::bind<void>(gl_disable_4, GL_BLEND)));
    _add(m, GLState::BLEND_5, epriv::GLStateData(false, B::bind<void>(gl_enable_5, GL_BLEND), B::bind<void>(gl_disable_5, GL_BLEND)));
    _add(m, GLState::BLEND_6, epriv::GLStateData(false, B::bind<void>(gl_enable_6, GL_BLEND), B::bind<void>(gl_disable_6, GL_BLEND)));
    _add(m, GLState::BLEND_7, epriv::GLStateData(false, B::bind<void>(gl_enable_7, GL_BLEND), B::bind<void>(gl_disable_7, GL_BLEND)));
    _add(m, GLState::BLEND_8, epriv::GLStateData(false, B::bind<void>(gl_enable_8, GL_BLEND), B::bind<void>(gl_disable_8, GL_BLEND)));
    _add(m, GLState::BLEND_9, epriv::GLStateData(false, B::bind<void>(gl_enable_9, GL_BLEND), B::bind<void>(gl_disable_9, GL_BLEND)));
    _add(m, GLState::BLEND_10, epriv::GLStateData(false, B::bind<void>(gl_enable_10, GL_BLEND), B::bind<void>(gl_disable_10, GL_BLEND)));
    _add(m, GLState::BLEND_11, epriv::GLStateData(false, B::bind<void>(gl_enable_11, GL_BLEND), B::bind<void>(gl_disable_11, GL_BLEND)));
    _add(m, GLState::BLEND_12, epriv::GLStateData(false, B::bind<void>(gl_enable_12, GL_BLEND), B::bind<void>(gl_disable_12, GL_BLEND)));
    _add(m, GLState::BLEND_13, epriv::GLStateData(false, B::bind<void>(gl_enable_13, GL_BLEND), B::bind<void>(gl_disable_13, GL_BLEND)));
    _add(m, GLState::BLEND_14, epriv::GLStateData(false, B::bind<void>(gl_enable_14, GL_BLEND), B::bind<void>(gl_disable_14, GL_BLEND)));
    _add(m, GLState::BLEND_15, epriv::GLStateData(false, B::bind<void>(gl_enable_15, GL_BLEND), B::bind<void>(gl_disable_15, GL_BLEND)));

    return m;
}();