#include "Engine_GLStateMachine.h"

using namespace std;

//enabled? //enableGLFunc //disableGLFunc


std::unordered_map<GLState::State,GLStateT> _populateStateMachineMap(){
	std::unordered_map<GLState::State,GLStateT> m; //TODO: use std::bind instead of lambdas? (or the boost equivalent)
	
	m.emplace(GLState::TEXTURE_1D,                GLStateT(false,[](){glEnable(GL_TEXTURE_1D);},[](){glDisable(GL_TEXTURE_1D);} ));
	m.emplace(GLState::TEXTURE_2D,                GLStateT(false,[](){glEnable(GL_TEXTURE_2D);},[](){glDisable(GL_TEXTURE_2D);} ));
	m.emplace(GLState::TEXTURE_3D,                GLStateT(false,[](){glEnable(GL_TEXTURE_3D);},[](){glDisable(GL_TEXTURE_3D);} ));
	m.emplace(GLState::CULL_FACE,                 GLStateT(false,[](){glEnable(GL_CULL_FACE);},[](){glDisable(GL_CULL_FACE);} ));
	m.emplace(GLState::ALPHA_TEST,                GLStateT(false,[](){glEnable(GL_ALPHA_TEST);},[](){glDisable(GL_ALPHA_TEST);} ));
	m.emplace(GLState::DEPTH_TEST,                GLStateT(true,[](){glEnable(GL_DEPTH_TEST);},[](){glDisable(GL_DEPTH_TEST);} ));
	m.emplace(GLState::STENCIL_TEST,              GLStateT(false,[](){glEnable(GL_STENCIL_TEST);},[](){glDisable(GL_STENCIL_TEST);} ));
	m.emplace(GLState::TEXTURE_CUBE_MAP_SEAMLESS, GLStateT(false,[](){glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);},[](){glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);} ));
	m.emplace(GLState::BLEND,                     GLStateT(false,[](){glEnable(GL_BLEND);},[](){glDisable(GL_BLEND);} ));

	m.emplace(GLState::DEPTH_MASK,                GLStateT(true,[](){glDepthMask(GL_TRUE);},[](){glDepthMask(GL_FALSE);} ));

	return m;
}

std::unordered_map<GLState::State,GLStateT> GLState::SM = _populateStateMachineMap();