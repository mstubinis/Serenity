#include "FramebufferObject.h"
#include "Engine_Renderer.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <unordered_map>

using namespace Engine;
using namespace std;

unordered_map<uint,GLuint> _populateGLAttatchmentMap(){
    unordered_map<uint,GLuint> m;

    m.emplace(uint(Color_0), GL_COLOR_ATTACHMENT0);
    m.emplace(uint(Color_1), GL_COLOR_ATTACHMENT1);
    m.emplace(uint(Color_2), GL_COLOR_ATTACHMENT2);
    m.emplace(uint(Color_3), GL_COLOR_ATTACHMENT3);
    m.emplace(uint(Color_4), GL_COLOR_ATTACHMENT4);
    m.emplace(uint(Color_5), GL_COLOR_ATTACHMENT5);
    m.emplace(uint(Color_6), GL_COLOR_ATTACHMENT6);
    m.emplace(uint(Color_7), GL_COLOR_ATTACHMENT7);
    m.emplace(uint(Color_8), GL_COLOR_ATTACHMENT8);
    m.emplace(uint(Depth), GL_DEPTH_ATTACHMENT);
    m.emplace(uint(Stencil), GL_STENCIL_ATTACHMENT);
    m.emplace(uint(DepthAndStencil), GL_DEPTH_STENCIL_ATTACHMENT);

    return m;
}
unordered_map<uint,GLuint> GL_ATTATCHMENT_MAP = _populateGLAttatchmentMap();


struct FramebufferObjectDefaultBindFunctor{void operator()(BindableResource* r) const {
    Renderer::setViewport(0,0,m_FramebufferWidth,m_FramebufferHeight);
    Renderer::bindFBO(m_FBO);
    
    
    
}};
struct FramebufferObjectDefaultUnbindFunctor{void operator()(BindableResource* r) const {
    
    
    
    Renderer::unbindFBO();
    Renderer::setViewport(0,0,Resources::getWindowSize().x,Resources::getWindowSize().y);
}};

class FramebufferObjectAttatchment::impl{
    public:
        FramebufferObject* m_FBO;
        GLuint m_GL_Attatchment;
        uint m_AttatchmentWidth; uint m_AttatchmentHeight;
        void _init(FramebufferObjectAttatchment* super,FramebufferAttatchment::Attatchment a){
            m_GL_Attatchment = GL_ATTATCHMENT_MAP.at(uint(a));
        }
        void _destruct(FramebufferObjectAttatchment* super){

        }
};
FramebufferObjectAttatchment::FramebufferObjectAttatchment(FramebufferAttatchment::Attatchment a){
    m_i->_init(this,a);
}
FramebufferObjectAttatchment::~FramebufferObjectAttatchment(){ m_i->_destruct(this); }
uint FramebufferObjectAttatchment::width(){ m_i->m_FBO->width(); }
uint FramebufferObjectAttatchment::height(){ m_i->m_FBO->height(); }

class RenderbufferObject::impl{
    public:
        GLuint m_RBO;
        void _init(RenderbufferObject* super,uint width,uint height,GLuint internalFormat){
            glGenRenderbuffers(1, &m_RBO);
            Renderer::bindRBO(m_RBO);
            glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
            Renderer::unbindRBO();
        }
        void _destruct(RenderbufferObject* super){
            glDeleteRenderbuffers(1, &m_RBO);
        }
};
RenderbufferObject::RenderbufferObject(uint w,uint h,GLuint internalFormat){
    m_i->_init(this,w,h,internalFormat);
}
RenderbufferObject::~RenderbufferObject(){ m_i->_destruct(this); }

class FramebufferObject::impl{
    public:
        static FramebufferObjectDefaultBindFunctor DEFAULT_BIND_FUNCTOR;
        static FramebufferObjectDefaultUnbindFunctor DEFAULT_UNBIND_FUNCTOR;

        GLuint m_FBO;
    
        unordered_map<uint,FramebufferObjectAttatchment*> m_Attatchments;
    
        uint m_FramebufferWidth;
        uint m_FramebufferHeight;

        void _init(FramebufferObject* super,uint width, uint height){
            m_FramebufferWidth = width; m_FramebufferHeight = height;
            
            super->setCustomBindFunctor(FramebufferObject::impl::DEFAULT_BIND_FUNCTOR);
            super->setCustomUnbindFunctor(FramebufferObject::impl::DEFAULT_UNBIND_FUNCTOR);

            glGenFramebuffers(1, &m_FBO);
            //glGenRenderbuffers(1, &m_RBO);
            Renderer::bindFBO(m_FBO);
            //Renderer::bindRBO(m_RBO);
            //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
            //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RBO); 
        }
        void _destruct(FramebufferObject* super){
            for(auto attatchment:m_Attatchments){
                delete attatchment.second;
            }
            glDeleteFramebuffers(1, &m_FBO);
        }
        void _resize(FramebufferObject* super,uint new_width,uint new_height){
            Renderer::bindFBO(m_FBO);
            //Renderer::bindRBO(m_RBO);
            
            m_FramebufferWidth = new_width; m_FramebufferHeight = new_height;
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, new_width, new_height);
            
            //Renderer::unbindRBO();
            Renderer::unbindFBO();
        }
        void _attatchTexture(FramebufferObject* super,Texture* txtre,FramebufferAttatchment::Attatchment atchmnt){
            glFramebufferTexture2D(GL_FRAMEBUFFER,GL_ATTATCHMENT_MAP.at(uint(atchmnt)),txtre->type(),txtre->address(),0);
            FramebufferObjectAttatchment attatchmentObject = new FramebufferObjectAttatchment(atchmnt);
            m_Attatchments.emplace(atchmnt,attatchmentObject);
            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
                cout << "Framebuffer completeness in FramebufferObject::impl is incomplete!" << endl;
            }
        }
        void _attatchRenderbuffer(FramebufferObject* super,RenderbufferObject* rbo,FramebufferAttatchment::Attatchment atchmnt){
            Renderer::bindRBO(rbo);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_ATTATCHMENT_MAP.at(uint(atchmnt)),GL_RENDERBUFFER,rbo->address());
            Renderer::unbindRBO();
            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
                cout << "Framebuffer completeness in FramebufferObject::impl is incomplete!" << endl;
            }
        }
};
FramebufferObjectDefaultBindFunctor FramebufferObject::impl::DEFAULT_BIND_FUNCTOR;
FramebufferObjectDefaultUnbindFunctor FramebufferObject::impl::DEFAULT_UNBIND_FUNCTOR;

FramebufferObject::FramebufferObject(string name,uint w,uint h):BindableResource(name),m_i(new impl){
    m_i->_init(this,w,h);
}
FramebufferObject::~FramebufferObject(){ m_i->_destruct(this); }
void FramebufferObject::resize(uint w,uint h){ m_i->_resize(this,w,h); }
void FramebufferObject::attatchTexture(Texture* t,FramebufferAttatchment::Attatchment a){ m_i->_attatchTexture(this,t,a); }
void FramebufferObject::attatchRenderBuffer(Renderbuffer* t,FramebufferAttatchment::Attatchment a){ m_i->_attatchRenderbuffer(this,t,a); }
uint FramebufferObject::width(){ return m_i->m_FramebufferWidth; }
uint FramebufferObject::height(){ return m_i->m_FramebufferHeight; }

