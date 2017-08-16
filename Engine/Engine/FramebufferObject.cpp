#include "FramebufferObject.h"
#include "Engine_Renderer.h"
#include "Engine_Resources.h"
#include "Texture.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <unordered_map>

using namespace Engine;
using namespace std;


struct FramebufferObjectDefaultBindFunctor{void operator()(BindableResource* r) const {
	FramebufferObject* fbo = static_cast<FramebufferObject*>(r);
	Renderer::setViewport(0,0,fbo->width(),fbo->height());
    Renderer::bindFBO(fbo);
    
    
    
}};
struct FramebufferObjectDefaultUnbindFunctor{void operator()(BindableResource* r) const {
    
    
    
    Renderer::unbindFBO();
    Renderer::setViewport(0,0,Resources::getWindowSize().x,Resources::getWindowSize().y);
}};

class FramebufferObjectAttatchment::impl{
    public:
        ImageInternalFormat::Format m_InternalFormat;
        FramebufferObject* m_FBO;
        GLuint m_GL_Attatchment;
        void _init(FramebufferObject* _fbo,FramebufferObjectAttatchment* super,FramebufferAttatchment::Attatchment a,ImageInternalFormat::Format i){
            m_FBO = _fbo;
			m_GL_Attatchment = FramebufferAttatchment::FRAMEBUFFER_ATTATCHMENT_FORMAT_MAP.at(uint(a));
			m_InternalFormat = i;
        }
        void _init(FramebufferObject* _fbo,FramebufferObjectAttatchment* super,FramebufferAttatchment::Attatchment a,Texture* t){
            m_FBO = _fbo;
            m_GL_Attatchment = FramebufferAttatchment::FRAMEBUFFER_ATTATCHMENT_FORMAT_MAP.at(uint(a));
			m_InternalFormat = t->internalFormat();
        }
        void _destruct(FramebufferObjectAttatchment* super){

        }
		void _resize(){
		}
};
FramebufferObjectAttatchment::FramebufferObjectAttatchment(FramebufferObject* _fbo,FramebufferAttatchment::Attatchment a,ImageInternalFormat::Format i):m_i(new impl){
    m_i->_init(_fbo,this,a,i);
}
FramebufferObjectAttatchment::FramebufferObjectAttatchment(FramebufferObject* _fbo,FramebufferAttatchment::Attatchment a,Texture* t):m_i(new impl){
    m_i->_init(_fbo,this,a,t);
}
FramebufferObjectAttatchment::~FramebufferObjectAttatchment(){ m_i->_destruct(this); }
uint FramebufferObjectAttatchment::width(){ return m_i->m_FBO->width(); }
uint FramebufferObjectAttatchment::height(){ return m_i->m_FBO->height(); }
FramebufferObject* FramebufferObjectAttatchment::fbo(){ return m_i->m_FBO; }
uint FramebufferObjectAttatchment::attatchment(){ return m_i->m_GL_Attatchment; }
void FramebufferObjectAttatchment::resize(){ m_i->_resize(); }

class RenderbufferObject::impl{
    public:
        GLuint m_RBO;
        void _init(RenderbufferObject* super,FramebufferObject* _fbo,ImageInternalFormat::Format internalFormat){
            glGenRenderbuffers(1, &m_RBO);
            Renderer::bindRBO(m_RBO);
			glRenderbufferStorage(GL_RENDERBUFFER, ImageInternalFormat::IMAGE_INTERNAL_FORMAT_MAP.at(uint(internalFormat)), super->fbo()->width(), super->fbo()->height());
            Renderer::unbindRBO();
        }
        void _destruct(RenderbufferObject* super){
            glDeleteRenderbuffers(1, &m_RBO);
        }
        void _resize(RenderbufferObject* super){
            Renderer::bindRBO(m_RBO);
            glRenderbufferStorage(GL_RENDERBUFFER, super->attatchment(), super->fbo()->width(), super->fbo()->height());
            Renderer::unbindRBO();
        }
};
RenderbufferObject::RenderbufferObject(FramebufferObject* f,FramebufferAttatchment::Attatchment a,ImageInternalFormat::Format i):FramebufferObjectAttatchment(f,a,i),m_i(new impl){
    m_i->_init(this,f,i);
}
RenderbufferObject::~RenderbufferObject(){ m_i->_destruct(this); }
void RenderbufferObject::resize(){ m_i->_resize(this); }
GLuint RenderbufferObject::address(){ return m_i->m_RBO; }

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

            for(auto attatchment:m_Attatchments){
                attatchment.second->resize();
            }
            
            //Renderer::unbindRBO();
            Renderer::unbindFBO();
        }
        void _attatchTexture(FramebufferObject* super,Texture* txtre,FramebufferAttatchment::Attatchment a){
            if(m_Attatchments.count(a)) return;
            
            glFramebufferTexture2D(GL_FRAMEBUFFER,FramebufferAttatchment::FRAMEBUFFER_ATTATCHMENT_FORMAT_MAP.at(uint(a)),txtre->type(),txtre->address(),0);
            FramebufferObjectAttatchment* attatchmentObject = new FramebufferObjectAttatchment(super,a,txtre);
            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
                cout << "Framebuffer completeness in FramebufferObject::impl is incomplete!" << endl;
            }
            m_Attatchments.emplace(a,attatchmentObject);
        }
        void _attatchRenderbuffer(FramebufferObject* super,RenderbufferObject* rbo){
            if(m_Attatchments.count(rbo->attatchment())){ return; }
            
            Renderer::bindRBO(rbo);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,rbo->attatchment(),GL_RENDERBUFFER,rbo->address());
            Renderer::unbindRBO();
            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
                cout << "Framebuffer completeness in FramebufferObject::impl is incomplete!" << endl;
            }
            m_Attatchments.emplace(rbo->attatchment(),rbo);
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
void FramebufferObject::attatchRenderBuffer(RenderbufferObject* t){ m_i->_attatchRenderbuffer(this,t); }
uint FramebufferObject::width(){ return m_i->m_FramebufferWidth; }
uint FramebufferObject::height(){ return m_i->m_FramebufferHeight; }
GLuint FramebufferObject::address(){ return m_i->m_FBO; }