#include "FramebufferObject.h"
#include "Engine_Renderer.h"
#include "Engine_Resources.h"
#include "Texture.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <unordered_map>

using namespace Engine;
using namespace std;

#pragma region FramebufferObjectAttatchmentBaseClass

class FramebufferObjectAttatchment::impl{
    public:
        GLuint m_InternalFormat;
        FramebufferObject* m_FBO;
        GLuint m_GL_Attatchment;
        void _init(FramebufferObject* _fbo,FramebufferObjectAttatchment* super,FramebufferAttatchment::Attatchment a,ImageInternalFormat::Format i){
            m_FBO = _fbo;
            m_GL_Attatchment = FramebufferAttatchment::at(a);
            m_InternalFormat = ImageInternalFormat::at(i);
        }
        void _init(FramebufferObject* _fbo,FramebufferObjectAttatchment* super,FramebufferAttatchment::Attatchment a,Texture* t){
            m_FBO = _fbo;
            m_GL_Attatchment = FramebufferAttatchment::at(a);
            m_InternalFormat = ImageInternalFormat::at(t->internalFormat());
        }
        void _destruct(FramebufferObjectAttatchment* super){
        }
        void _resize(FramebufferObjectAttatchment* super,uint w,uint h){
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
void FramebufferObjectAttatchment::resize(uint w, uint h){ m_i->_resize(this,w,h); }
GLuint FramebufferObjectAttatchment::address(){ return GLuint(0); }
void FramebufferObjectAttatchment::bind(){}
void FramebufferObjectAttatchment::unbind(){}
GLuint FramebufferObjectAttatchment::internalFormat(){ return m_i->m_InternalFormat; }

#pragma endregion

class FramebufferTexture::impl{
    public:
        Texture* m_Texture;
        GLuint m_PixelFormat;
        GLuint m_PixelType;
		float m_Divisor;
        void _init(FramebufferTexture* super, FramebufferObject* _fbo,FramebufferAttatchment::Attatchment a,Texture* t){
            m_Texture = t;
            m_PixelFormat = ImagePixelFormat::at(t->pixelFormat());
            m_PixelType = ImagePixelType::at(t->pixelType());
			_resize(super,_fbo->width(),_fbo->height());
			m_Divisor = 1.0f;
        }
        void _init(FramebufferTexture* super, FramebufferObject* _fbo,FramebufferAttatchment::Attatchment a,Texture* t,float divisor){
            m_Texture = t;
            m_PixelFormat = ImagePixelFormat::at(t->pixelFormat());
            m_PixelType = ImagePixelType::at(t->pixelType());
            _resize(super,_fbo->width(),_fbo->height());
			m_Divisor = divisor;
        }
        void _destruct(FramebufferTexture* super){
        }
        void _resize(FramebufferTexture* super,uint w,uint h){
            glBindTexture(m_Texture->type(),m_Texture->address());
            glTexImage2D(m_Texture->type(),0,super->internalFormat(),w,h,0,m_PixelFormat,m_PixelType,NULL);
            glFramebufferTexture2D(GL_FRAMEBUFFER,super->attatchment(),GL_TEXTURE_2D,m_Texture->address(),0);
            glBindTexture(m_Texture->type(),0);
        }
};
FramebufferTexture::FramebufferTexture(FramebufferObject* _fbo,FramebufferAttatchment::Attatchment a,Texture* t,float d):FramebufferObjectAttatchment(_fbo,a,t),m_i(new impl){
    m_i->_init(this,_fbo,a,t,d);
}
FramebufferTexture::FramebufferTexture(FramebufferObject* _fbo,FramebufferAttatchment::Attatchment a,Texture* t):FramebufferObjectAttatchment(_fbo,a,t),m_i(new impl){
    m_i->_init(this,_fbo,a,t);
}
FramebufferTexture::~FramebufferTexture(){ m_i->_destruct(this); }
void FramebufferTexture::resize(uint w, uint h){ m_i->_resize(this,w,h); }
GLuint FramebufferTexture::address(){ return m_i->m_Texture->address(); }
Texture* FramebufferTexture::texture(){ return m_i->m_Texture; }
void FramebufferTexture::bind(){}
void FramebufferTexture::unbind(){}
float FramebufferTexture::divisor(){ return m_i->m_Divisor; }

class RenderbufferObject::impl{
    public:
        GLuint m_RBO;
        void _init(RenderbufferObject* super,FramebufferObject* _fbo,ImageInternalFormat::Format internalFormat,FramebufferAttatchment::Attatchment a){
            glGenRenderbuffers(1, &m_RBO);
        }
        void _destruct(RenderbufferObject* super){
            glDeleteRenderbuffers(1, &m_RBO);
        }
        void _resize(RenderbufferObject* super,uint w, uint h){
            Renderer::bindRBO(m_RBO);
            glRenderbufferStorage(GL_RENDERBUFFER, super->attatchment(), w, h);
            Renderer::unbindRBO();
        }
};
RenderbufferObject::RenderbufferObject(FramebufferObject* f,FramebufferAttatchment::Attatchment a,ImageInternalFormat::Format i):FramebufferObjectAttatchment(f,a,i),m_i(new impl){
    m_i->_init(this,f,i,a);
}
RenderbufferObject::~RenderbufferObject(){ m_i->_destruct(this); }
void RenderbufferObject::resize(uint w, uint h){ m_i->_resize(this,w,h); }
GLuint RenderbufferObject::address(){ return m_i->m_RBO; }

void RenderbufferObject::bind(){ Renderer::bindRBO(m_i->m_RBO); }
void RenderbufferObject::unbind(){ Renderer::unbindRBO(); }


struct FramebufferObjectDefaultBindFunctor{void operator()(BindableResource* r) const {
    FramebufferObject* fbo = static_cast<FramebufferObject*>(r);
    Renderer::setViewport(0,0,fbo->width(),fbo->height());
    Renderer::bindFBO(fbo);
    for(auto attatchment:fbo->attatchments()){
        attatchment.second->bind();
    }

}};
struct FramebufferObjectDefaultUnbindFunctor{void operator()(BindableResource* r) const {
    FramebufferObject* fbo = static_cast<FramebufferObject*>(r);
    for(auto attatchment:fbo->attatchments()){
        attatchment.second->unbind();
    }
    Renderer::unbindFBO();
    Renderer::setViewport(0,0,Resources::getWindowSize().x,Resources::getWindowSize().y);
}};
class FramebufferObject::impl{
    public:
        static FramebufferObjectDefaultBindFunctor DEFAULT_BIND_FUNCTOR;
        static FramebufferObjectDefaultUnbindFunctor DEFAULT_UNBIND_FUNCTOR;

        GLuint m_FBO;

        unordered_map<uint,FramebufferObjectAttatchment*> m_Attatchments;

        uint m_FramebufferWidth;
        uint m_FramebufferHeight;

        void _baseInit(FramebufferObject* super,uint width, uint height){
            m_FramebufferWidth = width; m_FramebufferHeight = height;

            super->setCustomBindFunctor(FramebufferObject::impl::DEFAULT_BIND_FUNCTOR);
            super->setCustomUnbindFunctor(FramebufferObject::impl::DEFAULT_UNBIND_FUNCTOR);

            glGenFramebuffers(1, &m_FBO);
        }
        void _init(FramebufferObject* super,uint width, uint height){
            _baseInit(super,width,height);
        }
        void _init(FramebufferObject* super,uint width, uint height,ImageInternalFormat::Format depthInternalFormat){
            _baseInit(super,width,height);
            RenderbufferObject* rbo = new RenderbufferObject(super,FramebufferAttatchment::Depth,depthInternalFormat);
            _attatchRenderbuffer(super,rbo);
        }
        void _destruct(FramebufferObject* super){
            for(auto attatchment:m_Attatchments){
                delete attatchment.second;
            }
            glDeleteFramebuffers(1, &m_FBO);
        }
        void _resize(FramebufferObject* super,uint new_width,uint new_height){
			Renderer::setViewport(0,0,new_width,new_height);
            Renderer::bindFBO(m_FBO);
            m_FramebufferWidth = new_width; m_FramebufferHeight = new_height;
            for(auto attatchment:m_Attatchments){
                attatchment.second->resize(new_width,new_height);
            }
        }
		FramebufferTexture* _attatchTexture(FramebufferObject* super,Texture* _t,FramebufferAttatchment::Attatchment a){
            if(m_Attatchments.count(a)) return nullptr;
            Renderer::bindFBO(m_FBO);
            glFramebufferTexture2D(GL_FRAMEBUFFER,FramebufferAttatchment::FRAMEBUFFER_ATTATCHMENT_FORMAT_MAP.at(uint(a)),_t->type(),_t->address(),0);
            FramebufferTexture* t = new FramebufferTexture(super,a,_t);

            m_Attatchments.emplace(a,t);
            Renderer::unbindFBO();
			return t;
        }
		FramebufferTexture* _attatchTexture(FramebufferObject* super,Texture* _t,FramebufferAttatchment::Attatchment a,float divisor){
            if(m_Attatchments.count(a)) return nullptr;
            Renderer::bindFBO(m_FBO);
            glFramebufferTexture2D(GL_FRAMEBUFFER,FramebufferAttatchment::FRAMEBUFFER_ATTATCHMENT_FORMAT_MAP.at(uint(a)),_t->type(),_t->address(),0);
            FramebufferTexture* t = new FramebufferTexture(super,a,_t,divisor);
            m_Attatchments.emplace(a,t);
            Renderer::unbindFBO();
			return t;
        }
        RenderbufferObject* _attatchRenderbuffer(FramebufferObject* super,RenderbufferObject* rbo){
            if(m_Attatchments.count(rbo->attatchment())){ return nullptr; }

            Renderer::bindFBO(m_FBO);
            Renderer::bindRBO(rbo);

            glRenderbufferStorage(GL_RENDERBUFFER,rbo->internalFormat(), super->width(), super->height());
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,rbo->attatchment(),GL_RENDERBUFFER,rbo->address());

            m_Attatchments.emplace(rbo->attatchment(),rbo);

            Renderer::unbindRBO();
            Renderer::unbindFBO();
			return rbo;
        }
        void _check(FramebufferObject* super){
            super->bind();
            GLenum err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if(err != GL_FRAMEBUFFER_COMPLETE){
                cout << "Framebuffer completeness in FramebufferObject::impl _check() is incomplete!" << endl;
                cout << "Error is: " << err << std::endl;
            }
        }
};
FramebufferObjectDefaultBindFunctor FramebufferObject::impl::DEFAULT_BIND_FUNCTOR;
FramebufferObjectDefaultUnbindFunctor FramebufferObject::impl::DEFAULT_UNBIND_FUNCTOR;

FramebufferObject::FramebufferObject(string name,uint w,uint h):BindableResource(name),m_i(new impl){
    m_i->_init(this,w,h);
}
FramebufferObject::FramebufferObject(string name,uint w,uint h,ImageInternalFormat::Format depthInternalFormat):BindableResource(name),m_i(new impl){
    m_i->_init(this,w,h,depthInternalFormat);
}
FramebufferObject::~FramebufferObject(){ m_i->_destruct(this); }
void FramebufferObject::resize(uint w,uint h){ m_i->_resize(this,w,h); }
FramebufferTexture* FramebufferObject::attatchTexture(Texture* t,FramebufferAttatchment::Attatchment a){ return m_i->_attatchTexture(this,t,a); }
RenderbufferObject* FramebufferObject::attatchRenderBuffer(RenderbufferObject* t){ return m_i->_attatchRenderbuffer(this,t); }
uint FramebufferObject::width(){ return m_i->m_FramebufferWidth; }
uint FramebufferObject::height(){ return m_i->m_FramebufferHeight; }
GLuint FramebufferObject::address(){ return m_i->m_FBO; }
unordered_map<uint,FramebufferObjectAttatchment*>& FramebufferObject::attatchments(){ return m_i->m_Attatchments; }
void FramebufferObject::check(){ m_i->_check(this); }
