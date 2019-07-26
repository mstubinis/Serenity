#include <core/engine/renderer/GLStateMachine.h>

using namespace Engine;
using namespace std;

typedef unsigned int uint;

void epriv::GLStateMachineDataCustom::init() {
    current_bound_buffer_vbo = 0;
    current_bound_buffer_ebo = 0;

    current_bound_read_fbo = 0;
    current_bound_draw_fbo = 0;
    current_bound_rbo = 0;

    current_bound_shader_program = nullptr;
    current_bound_material = nullptr;
    current_bound_mesh = nullptr;
}
epriv::GLStateMachineDataCustom::GLStateMachineDataCustom() { 
    init(); 
}
epriv::GLStateMachineDataCustom::~GLStateMachineDataCustom() { 
    init(); 
}
