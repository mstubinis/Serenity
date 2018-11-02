#include "core/engine/Engine_GLStateMachine.h"
#include <boost/bind.hpp>

using namespace Engine;
using namespace std;
namespace B = ::boost;

typedef unsigned int uint;

void epriv::GLStateMachineDataCustom::init() {
    current_bound_vbo = 0;
    current_bound_ebo = 0;
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
    _add(m,GLState::BLEND, epriv::GLStateData(false,B::bind<void>(glEnable,GL_BLEND),B::bind<void>(glDisable,GL_BLEND)));
    _add(m,GLState::DEPTH_MASK, epriv::GLStateData(true,B::bind<void>(glDepthMask,GL_TRUE),B::bind<void>(glDepthMask,GL_FALSE)));
    _add(m,GLState::DITHER, epriv::GLStateData(false,B::bind<void>(glEnable,GL_DITHER),B::bind<void>(glDisable,GL_DITHER)));
    _add(m,GLState::SCISSOR_TEST, epriv::GLStateData(false,B::bind<void>(glEnable,GL_SCISSOR_TEST),B::bind<void>(glDisable,GL_SCISSOR_TEST)));

    return m;
}();