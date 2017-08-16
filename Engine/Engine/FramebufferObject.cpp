#include "FramebufferObject.h"
#include "Engine_Renderer.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>

using namespace Engine;

struct FramebufferObjectDefaultBindFunctor{void operator()(BindableResource* r) const {
    Renderer::setViewport(0,0,m_FramebufferWidth,m_FramebufferHeight);
    Renderer::bindFBO(m_FBO);
    
    
    
}};
struct FramebufferObjectDefaultUnbindFunctor{void operator()(BindableResource* r) const {
    
    
    
    Renderer::unbindFBO();
    Renderer::setViewport(0,0,Resources::getWindowSize().x,Resources::getWindowSize().y);
}};


class FramebufferObject::impl{
    public:
        static FramebufferObjectDefaultBindFunctor DEFAULT_BIND_FUNCTOR;
        static FramebufferObjectDefaultUnbindFunctor DEFAULT_UNBIND_FUNCTOR;

        GLuint m_FBO;
        GLuint m_RBO;
    
        uint m_FramebufferWidth;
        uint m_FramebufferHeight;

        void _init(FramebufferObject* super,uint width, uint height){
            m_FramebufferWidth = width; m_FramebufferHeight = height;
            
            super->setCustomBindFunctor(FramebufferObject::impl::DEFAULT_BIND_FUNCTOR);
            super->setCustomUnbindFunctor(FramebufferObject::impl::DEFAULT_UNBIND_FUNCTOR);

            glGenFramebuffers(1, &m_FBO);
            glGenRenderbuffers(1, &m_RBO);
            Renderer::bindFBO(m_FBO);
            Renderer::bindRBO(m_RBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RBO); 

            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
                cout << "Framebuffer completeness in FramebufferObject::impl is incomplete!" << endl; return;
            }
        }
        void _destruct(FramebufferObject* super){
            glDeleteRenderbuffers(1, &m_RBO);
            glDeleteFramebuffers(1, &m_FBO);
        }

};
FramebufferObjectDefaultBindFunctor FramebufferObject::impl::DEFAULT_BIND_FUNCTOR;
FramebufferObjectDefaultUnbindFunctor FramebufferObject::impl::DEFAULT_UNBIND_FUNCTOR;

FramebufferObject::FramebufferObject(std::string name,uint w,uint h):BindableResource(name),m_i(new impl){
    m_i->_init(this,w,h);
}
FramebufferObject::~FramebufferObject(){
    m_i->_destruct(this);
}
