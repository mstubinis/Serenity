#pragma once
#ifndef ENGINE_GL_STATE_MACHINE_H
#define ENGINE_GL_STATE_MACHINE_H

#include <vector>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include <boost/function.hpp>
#include <boost/bind.hpp>

typedef unsigned int uint;
typedef boost::function<void()> BF;

struct GLStateT{
    bool enabled; 
    BF enableFunc, disableFunc;
    GLStateT(){}
    GLStateT(const bool& b, const BF& en, const BF& dis){
        enabled = b;
        enableFunc = en;
        disableFunc = dis;
    }
};
class GLState{
    public: enum State{
        TEXTURE_1D,TEXTURE_2D,TEXTURE_3D,CULL_FACE,ALPHA_TEST,DEPTH_TEST,DEPTH_CLAMP,STENCIL_TEST,TEXTURE_CUBE_MAP_SEAMLESS,BLEND,DEPTH_MASK,DITHER,
		SCISSOR_TEST,
    _TOTAL};
    public: static std::vector<GLStateT> SM;
};

namespace Engine{
    namespace Renderer{
        inline void GLEnable(const GLState::State s){ auto& t=GLState::SM.at(s);if(t.enabled)return;t.enableFunc();t.enabled=1; }
        inline void GLDisable(const GLState::State s){ auto& t=GLState::SM.at(s);if(!t.enabled)return;t.disableFunc();t.enabled=0; }
        inline bool GLEnabled(const GLState::State s){ auto& t=GLState::SM.at(s);return t.enabled; }
        inline bool GLDisabled(const GLState::State s){ auto& t=GLState::SM.at(s);return !t.enabled; }
        inline void RestoreGLState(){ for(auto t:GLState::SM){ t.enabled? t.enableFunc() : t.disableFunc(); } }
    };
};

#endif