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

class RenderbufferObject::impl{
    public:
        GLuint m_RBO;
        void _init(RenderbufferObject* super){
            glGenRenderbuffers(1, &m_RBO);
        }
        void _destruct(RenderbufferObject* super){
            glDeleteRenderbuffers(1, &m_RBO);
        }
};
RenderbufferObject::RenderbufferObject(){
    m_i->_init(this);
}
RenderbufferObject::~RenderbufferObject(){ m_i->_destruct(this); }

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
            glDeleteFramebuffers(1, &m_FBO);
        }
        void _resize(FramebufferObject* super,uint new_width,uint new_height){
            Renderer::bindFBO(m_FBO);
            Renderer::bindRBO(m_RBO);
            
            m_FramebufferWidth = new_width; m_FramebufferHeight = new_height;
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, new_width, new_height);
            
            Renderer::unbindFBO();
            Renderer::unbindRBO();
        }
        void _attatchTexture(FramebufferObject* super,Texture* texture,FramebufferAttatchment::Attatchment attatchment){
            if(attatchment != FramebufferAttatchment::Depth && attatchment != FramebufferAttatchment::Stencil && attatchment != FramebufferAttatchment::DepthAndStencil){
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attatchment,texture->type(),texture->address(), 0);
            }
            else{
                if(attatchment == FramebufferAttatchment::Depth)
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture->type(), texture->address(), 0);
                else if(attatchment == FramebufferAttatchment::Stencil)
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, texture->type(), texture->address(), 0);
                else if(attatchment == FramebufferAttatchment::DepthAndStencil)
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, texture->type(), texture->address(), 0);
            }
        }
        void _attatchRenderbuffer(FramebufferObject* super,RenderbufferObject* rbo,FramebufferAttatchment::Attatchment attatchment){
            Renderer::bindRBO(rbo);
            if(attatchment != FramebufferAttatchment::Depth && attatchment != FramebufferAttatchment::Stencil && attatchment != FramebufferAttatchment::DepthAndStencil){
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attatchment,GL_RENDERBUFFER,rbo->address());
            }
            else{
                if(attatchment == FramebufferAttatchment::Depth)
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,rbo->address());
                else if(attatchment == FramebufferAttatchment::Stencil)
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,GL_RENDERBUFFER,rbo->address());
                else if(attatchment == FramebufferAttatchment::DepthAndStencil)
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,GL_RENDERBUFFER,rbo->address());
            }
            Renderer::unbindRBO();
        }
};
FramebufferObjectDefaultBindFunctor FramebufferObject::impl::DEFAULT_BIND_FUNCTOR;
FramebufferObjectDefaultUnbindFunctor FramebufferObject::impl::DEFAULT_UNBIND_FUNCTOR;

FramebufferObject::FramebufferObject(std::string name,uint w,uint h):BindableResource(name),m_i(new impl){
    m_i->_init(this,w,h);
}
FramebufferObject::~FramebufferObject(){ m_i->_destruct(this); }
void FramebufferObject::resize(uint w,uint h){ m_i->_resize(this,w,h); }
void FramebufferObject::attatchTexture(Texture* t,FramebufferAttatchment::Attatchment a){ m_i->_attatchTexture(this,t,a); }
void FramebufferObject::attatchRenderBuffer(Renderbuffer* t,FramebufferAttatchment::Attatchment a){ m_i->_attatchRenderbuffer(this,t,a); }
