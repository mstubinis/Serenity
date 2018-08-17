#include "Engine_GLStateMachine.h"

using namespace std;
namespace B = ::boost;

void _add(vector<GLStateT>& v,unsigned int key,GLStateT& value){ v.at(key) = value; }

//enabled? //enableGLFunc //disableGLFunc

vector<GLStateT> GLState::SM = [](){
    vector<GLStateT> m; m.resize(GLState::_TOTAL);
    
    _add(m,GLState::TEXTURE_1D,GLStateT(false,B::bind<void>(glEnable,GL_TEXTURE_1D),B::bind<void>(glDisable,GL_TEXTURE_1D)));
    _add(m,GLState::TEXTURE_1D,GLStateT(false,B::bind<void>(glEnable,GL_TEXTURE_1D),B::bind<void>(glDisable,GL_TEXTURE_1D)));
    _add(m,GLState::TEXTURE_2D,GLStateT(false,B::bind<void>(glEnable,GL_TEXTURE_2D),B::bind<void>(glDisable,GL_TEXTURE_2D)));
    _add(m,GLState::TEXTURE_3D,GLStateT(false,B::bind<void>(glEnable,GL_TEXTURE_3D),B::bind<void>(glDisable,GL_TEXTURE_3D)));
    _add(m,GLState::CULL_FACE,GLStateT(false,B::bind<void>(glEnable,GL_CULL_FACE),B::bind<void>(glDisable,GL_CULL_FACE)));
    _add(m,GLState::ALPHA_TEST,GLStateT(false,B::bind<void>(glEnable,GL_ALPHA_TEST),B::bind<void>(glDisable,GL_ALPHA_TEST)));
    _add(m,GLState::DEPTH_TEST,GLStateT(true,B::bind<void>(glEnable,GL_DEPTH_TEST),B::bind<void>(glDisable,GL_DEPTH_TEST)));
    _add(m,GLState::DEPTH_CLAMP,GLStateT(true,B::bind<void>(glEnable,GL_DEPTH_CLAMP),B::bind<void>(glDisable,GL_DEPTH_CLAMP)));
    _add(m,GLState::STENCIL_TEST,GLStateT(false,B::bind<void>(glEnable,GL_STENCIL_TEST),B::bind<void>(glDisable,GL_STENCIL_TEST)));
    _add(m,GLState::TEXTURE_CUBE_MAP_SEAMLESS,GLStateT(false,B::bind<void>(glEnable,GL_TEXTURE_CUBE_MAP_SEAMLESS),B::bind<void>(glDisable,GL_TEXTURE_CUBE_MAP_SEAMLESS) ));
    _add(m,GLState::BLEND,GLStateT(false,B::bind<void>(glEnable,GL_BLEND),B::bind<void>(glDisable,GL_BLEND)));
    _add(m,GLState::DEPTH_MASK,GLStateT(true,B::bind<void>(glDepthMask,GL_TRUE),B::bind<void>(glDepthMask,GL_FALSE)));

    return m;
}();