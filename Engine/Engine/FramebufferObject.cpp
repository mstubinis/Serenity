#include "FramebufferObject.h"

using namespace Engine;

class FramebufferObject::impl{
    public:
        GLuint m_FBO;
        GLuint m_RBO;
        
};

FramebufferObject::FramebufferObject(){
}
FramebufferObject::~FramebufferObject(){
}
