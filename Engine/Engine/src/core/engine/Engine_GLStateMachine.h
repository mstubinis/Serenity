#pragma once
#ifndef ENGINE_GL_STATE_MACHINE_H
#define ENGINE_GL_STATE_MACHINE_H

#include <vector>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <boost/function.hpp>

typedef boost::function<void()> boost_func;

struct GLStateT{
    bool enabled; 
    boost_func enableFunc, disableFunc;
    GLStateT(){}
    GLStateT(const bool& _enabled, const boost_func& _enableFunc, const boost_func& _disableFunc){
        enabled = _enabled;
        enableFunc = _enableFunc;
        disableFunc = _disableFunc;
    }
};
struct GLState final{
    enum State{
        TEXTURE_1D,TEXTURE_2D,TEXTURE_3D,CULL_FACE,ALPHA_TEST,DEPTH_TEST,DEPTH_CLAMP,STENCIL_TEST,TEXTURE_CUBE_MAP_SEAMLESS,BLEND,DEPTH_MASK,DITHER,
        SCISSOR_TEST,
    _TOTAL};
    static std::vector<GLStateT> SM;
};

namespace Engine{
    namespace Renderer{
        inline       void GLEnable(const GLState::State s){ auto& t=GLState::SM[s];if(t.enabled)return;t.enableFunc();t.enabled=1; }
        inline       void GLDisable(const GLState::State s){ auto& t=GLState::SM[s];if(!t.enabled)return;t.disableFunc();t.enabled=0; }
        inline const bool GLEnabled(const GLState::State s){ auto& t=GLState::SM[s];return t.enabled; }
        inline const bool GLDisabled(const GLState::State s){ auto& t=GLState::SM[s];return !t.enabled; }
        inline       void RestoreGLState(){ for(auto& t:GLState::SM){ t.enabled? t.enableFunc() : t.disableFunc(); } }
    };
};

#endif