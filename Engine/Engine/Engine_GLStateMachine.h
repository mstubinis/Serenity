#pragma once
#ifndef ENGINE_GL_STATE_MACHINE_H
#define ENGINE_GL_STATE_MACHINE_H

#include <unordered_map>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include <boost/function.hpp>
#include <boost/bind.hpp>

typedef unsigned int uint;
typedef boost::function<void()> BF;

struct GLStateT{
	bool enabled; 
	BF enableFunc, disableFunc;
	GLStateT(const bool& b, const BF& en, const BF& dis){
		enabled = b;
		enableFunc = en;
		disableFunc = dis;
	}
};
class GLState{
	public: enum State{
	    TEXTURE_1D,TEXTURE_2D,TEXTURE_3D,CULL_FACE,ALPHA_TEST,DEPTH_TEST,STENCIL_TEST,TEXTURE_CUBE_MAP_SEAMLESS,BLEND,DEPTH_MASK,

		_STATE_COUNT
	};
    public: static std::unordered_map<GLState::State,GLStateT> SM;
};

namespace Engine{
	namespace Renderer{
		inline void GLEnable(const GLState::State& s){
			GLStateT& t=GLState::SM.at(s);if(t.enabled) return;t.enableFunc();t.enabled=1;
		}
		inline void GLDisable(const GLState::State& s){ 
			GLStateT& t=GLState::SM.at(s);if(!t.enabled) return;t.disableFunc();t.enabled=0;
		}
		inline bool GLEnabled(const GLState::State& s){
			GLStateT& t=GLState::SM.at(s);return t.enabled;
		}
		inline bool GLDisabled(const GLState::State& s){
			GLStateT& t=GLState::SM.at(s);return !t.enabled;
		}
		inline void RestoreGLState(){
			for(auto a:GLState::SM){GLStateT& t=a.second;if(t.enabled)t.enableFunc();else t.disableFunc();}
		}
	};
};

#endif