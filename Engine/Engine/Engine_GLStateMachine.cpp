#include "Engine_GLStateMachine.h"

using namespace std;
namespace B = ::boost;

//enabled? //enableGLFunc //disableGLFunc

std::unordered_map<GLState::State,GLStateT> _populateStateMachineMap(){
    std::unordered_map<GLState::State,GLStateT> m;
    
    m.emplace(GLState::TEXTURE_1D,                GLStateT(false,B::bind<void>(glEnable,GL_TEXTURE_1D),B::bind<void>(glDisable,GL_TEXTURE_1D) ));
    m.emplace(GLState::TEXTURE_2D,                GLStateT(false,B::bind<void>(glEnable,GL_TEXTURE_2D),B::bind<void>(glDisable,GL_TEXTURE_2D) ));
    m.emplace(GLState::TEXTURE_3D,                GLStateT(false,B::bind<void>(glEnable,GL_TEXTURE_3D),B::bind<void>(glDisable,GL_TEXTURE_3D) ));
    m.emplace(GLState::CULL_FACE,                 GLStateT(false,B::bind<void>(glEnable,GL_CULL_FACE),B::bind<void>(glDisable,GL_CULL_FACE) ));
    m.emplace(GLState::ALPHA_TEST,                GLStateT(false,B::bind<void>(glEnable,GL_ALPHA_TEST),B::bind<void>(glDisable,GL_ALPHA_TEST) ));
    m.emplace(GLState::DEPTH_TEST,                GLStateT(true,B::bind<void>(glEnable,GL_DEPTH_TEST),B::bind<void>(glDisable,GL_DEPTH_TEST) ));
    m.emplace(GLState::STENCIL_TEST,              GLStateT(false,B::bind<void>(glEnable,GL_STENCIL_TEST),B::bind<void>(glDisable,GL_STENCIL_TEST) ));
    m.emplace(GLState::TEXTURE_CUBE_MAP_SEAMLESS, GLStateT(false,B::bind<void>(glEnable,GL_TEXTURE_CUBE_MAP_SEAMLESS),B::bind<void>(glDisable,GL_TEXTURE_CUBE_MAP_SEAMLESS) ));
    m.emplace(GLState::BLEND,                     GLStateT(false,B::bind<void>(glEnable,GL_BLEND),B::bind<void>(glDisable,GL_BLEND) ));

    m.emplace(GLState::DEPTH_MASK,                GLStateT(true,B::bind<void>(glDepthMask,GL_TRUE),B::bind<void>(glDepthMask,GL_FALSE) ));

    return m;
}

std::unordered_map<GLState::State,GLStateT> GLState::SM = _populateStateMachineMap();