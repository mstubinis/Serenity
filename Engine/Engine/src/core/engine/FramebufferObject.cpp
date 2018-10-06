#include "core/engine/FramebufferObject.h"
#include "core/engine/Engine_Renderer.h"
#include "core/engine/Engine_Resources.h"
#include "core/Texture.h"

#include <iostream>

using namespace Engine;
using namespace std;

#pragma region FramebufferObjectAttatchmentBaseClass

class epriv::FramebufferObjectAttatchment::impl{
    public:
        GLuint m_InternalFormat;
        FramebufferObject* m_FBO;
        GLuint m_GL_Attatchment;
        void _init(FramebufferObject& _fbo,FramebufferAttatchment::Attatchment a,ImageInternalFormat::Format i){
            m_FBO = &_fbo;
            m_GL_Attatchment = FramebufferAttatchment::at(a);
            m_InternalFormat = ImageInternalFormat::at(i);
        }
        void _init(FramebufferObject& _fbo,FramebufferAttatchment::Attatchment a,Texture* t){
            m_FBO = &_fbo;
            m_GL_Attatchment = FramebufferAttatchment::at(a);
            m_InternalFormat = ImageInternalFormat::at(t->internalFormat());
        }
};
epriv::FramebufferObjectAttatchment::FramebufferObjectAttatchment(FramebufferObject& _fbo,FramebufferAttatchment::Attatchment a,ImageInternalFormat::Format i):m_i(new impl){ m_i->_init(_fbo,a,i); }
epriv::FramebufferObjectAttatchment::FramebufferObjectAttatchment(FramebufferObject& _fbo,FramebufferAttatchment::Attatchment a,Texture* t):m_i(new impl){ m_i->_init(_fbo,a,t); }
epriv::FramebufferObjectAttatchment::~FramebufferObjectAttatchment(){ }
uint epriv::FramebufferObjectAttatchment::width(){ return m_i->m_FBO->width(); }
uint epriv::FramebufferObjectAttatchment::height(){ return m_i->m_FBO->height(); }
uint epriv::FramebufferObjectAttatchment::attatchment(){ return m_i->m_GL_Attatchment; }
void epriv::FramebufferObjectAttatchment::resize(FramebufferObject& _fbo,uint w, uint h){ }
GLuint epriv::FramebufferObjectAttatchment::address(){ return GLuint(0); }
void epriv::FramebufferObjectAttatchment::bind(){}
void epriv::FramebufferObjectAttatchment::unbind(){}
GLuint epriv::FramebufferObjectAttatchment::internalFormat(){ return m_i->m_InternalFormat; }

#pragma endregion

class epriv::FramebufferTexture::impl{
    public:
        Texture* m_Texture;
        GLuint m_PixelFormat;
        GLuint m_PixelType;
        void _init(Texture* t){
            m_Texture = t;
            m_PixelFormat = ImagePixelFormat::at(t->pixelFormat());
            m_PixelType = ImagePixelType::at(t->pixelType());
        }
        void _bindGPUDataToFBO(FramebufferTexture& super) {               //force as GL_TEXTURE_2D?                      //mipmap level
            glFramebufferTexture2D(GL_FRAMEBUFFER, super.attatchment(), super.texture()->type(), m_Texture->address(), 0);
        }
        void _destruct(){
            SAFE_DELETE(m_Texture);
        }
        void _resize(FramebufferObject& _fbo,uint w,uint h){
            m_Texture->resize(_fbo,w,h);
        }
};
epriv::FramebufferTexture::FramebufferTexture(FramebufferObject& _fbo,FramebufferAttatchment::Attatchment a,Texture* t):FramebufferObjectAttatchment(_fbo,a,t),m_i(new impl){
    m_i->_init(t);
}
epriv::FramebufferTexture::~FramebufferTexture(){ m_i->_destruct(); }
void epriv::FramebufferTexture::resize(FramebufferObject& _fbo,uint w, uint h){ m_i->_resize(_fbo,w,h); }
GLuint epriv::FramebufferTexture::address(){ return m_i->m_Texture->address(); }
Texture* epriv::FramebufferTexture::texture(){ return m_i->m_Texture; }
void epriv::FramebufferTexture::bind(){}
void epriv::FramebufferTexture::unbind(){}

class epriv::RenderbufferObject::impl{
    public:
        GLuint m_RBO;
        uint m_Width, m_Height;
        void _init(){
            m_Width = m_Height = 0;
            glGenRenderbuffers(1, &m_RBO);
        }
        void _destruct(){
            m_Width = m_Height = 0;
            glDeleteRenderbuffers(1, &m_RBO);
        }
        void _bindGPUDataToFBO(RenderbufferObject& super, uint w, uint h) {
            m_Width = w;  m_Height = h;
            glRenderbufferStorage(GL_RENDERBUFFER, super.attatchment(), m_Width, m_Height);
        }
        void _resize(RenderbufferObject& super,uint w, uint h){
            Renderer::bindRBO(m_RBO);
            _bindGPUDataToFBO(super, w, h);
            Renderer::unbindRBO();
        }
};
epriv::RenderbufferObject::RenderbufferObject(FramebufferObject& f,FramebufferAttatchment::Attatchment a,ImageInternalFormat::Format i):FramebufferObjectAttatchment(f,a,i),m_i(new impl){
    m_i->_init();
}
epriv::RenderbufferObject::~RenderbufferObject(){ m_i->_destruct(); }
void epriv::RenderbufferObject::resize(FramebufferObject& _fbo,uint w, uint h){ m_i->_resize(*this,w,h); }
GLuint epriv::RenderbufferObject::address(){ return m_i->m_RBO; }

void epriv::RenderbufferObject::bind(){ Renderer::bindRBO(m_i->m_RBO); }
void epriv::RenderbufferObject::unbind(){ Renderer::unbindRBO(); }

class epriv::FramebufferObject::impl{
    friend struct ::Engine::epriv::FramebufferObjectDefaultBindFunctor;
    friend struct ::Engine::epriv::FramebufferObjectDefaultUnbindFunctor;
    public:
        uint m_CurrentFBOIndex;
        vector<GLuint> m_FBO;

        unordered_map<uint,FramebufferObjectAttatchment*> m_Attatchments;
        float m_Divisor;

        uint m_FramebufferWidth; uint m_FramebufferHeight;
        void _baseInit(uint width, uint height,float divisor,uint swapBufferCount){
            m_CurrentFBOIndex = 0;
            m_FBO.resize(swapBufferCount, GLuint(0));
            m_Divisor = divisor;
            m_FramebufferWidth = uint(float(width) * m_Divisor);
            m_FramebufferHeight = uint(float(height) * m_Divisor);
            for(uint i = 0; i < m_FBO.size(); ++i)
                glGenFramebuffers(1, &m_FBO[i]);
        }
        void _init(uint width, uint height,float divisor,uint swapBufferCount){
            _baseInit(width,height,divisor, swapBufferCount);
        }
        void _init(FramebufferObject& super,uint width, uint height,ImageInternalFormat::Format depthInternalFormat,float divisor,uint swapBufferCount){
            _baseInit(width,height,divisor, swapBufferCount);
            RenderbufferObject* rbo;
            if(depthInternalFormat == ImageInternalFormat::Depth24Stencil8 || depthInternalFormat == ImageInternalFormat::Depth32FStencil8)
                rbo = new RenderbufferObject(super,FramebufferAttatchment::DepthAndStencil,depthInternalFormat);
            else if(depthInternalFormat == ImageInternalFormat::StencilIndex8)
                rbo = new RenderbufferObject(super,FramebufferAttatchment::Stencil,depthInternalFormat);
            else
                rbo = new RenderbufferObject(super,FramebufferAttatchment::Depth,depthInternalFormat);
            _attatchRenderbuffer(super,*rbo);
        }
        void _destruct(){
            SAFE_DELETE_MAP(m_Attatchments);         
            for (uint i = 0; i < m_FBO.size(); ++i)
                glDeleteFramebuffers(1, &m_FBO[i]);
        }
        void _resize(FramebufferObject& super,uint new_width,uint new_height){
            m_FramebufferWidth = uint(float(new_width) * m_Divisor);
            m_FramebufferHeight = uint(float(new_height) * m_Divisor);
            Renderer::setViewport(0,0,m_FramebufferWidth,m_FramebufferHeight);
            for (uint i = 0; i < m_FBO.size(); ++i) {
                Renderer::bindFBO(m_FBO[i]);
                for (auto attatchment : m_Attatchments) {
                    attatchment.second->resize(super, new_width, new_height);
                }
            }
        }
        FramebufferTexture* _attatchTexture(FramebufferObject& super,Texture* _t,FramebufferAttatchment::Attatchment a){
            if(m_Attatchments.count(a)) return nullptr;
            FramebufferTexture* t = new FramebufferTexture(super, a, _t);
            for (uint i = 0; i < m_FBO.size(); ++i) {
                Renderer::bindFBO(m_FBO[i]);
                t->m_i->_bindGPUDataToFBO(*t);
            }    
            m_Attatchments.emplace(a,t);
            Renderer::unbindFBO();
            return t;
        }
        RenderbufferObject* _attatchRenderbuffer(FramebufferObject& super,RenderbufferObject& rbo){
            if(m_Attatchments.count(rbo.attatchment())){ return nullptr; }
            for (uint i = 0; i < m_FBO.size(); ++i) {
                Renderer::bindFBO(m_FBO[i]);
                Renderer::bindRBO(rbo);
                glRenderbufferStorage(GL_RENDERBUFFER, rbo.internalFormat(), super.width(), super.height());
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, rbo.internalFormat(), GL_RENDERBUFFER, rbo.address());
                Renderer::unbindRBO();
            }
            m_Attatchments.emplace(rbo.attatchment(),&rbo);
            Renderer::unbindRBO();
            Renderer::unbindFBO();
            return &rbo;
        }
        bool _check(){
            for (uint i = 0; i < m_FBO.size(); ++i) {
                Renderer::bindFBO(m_FBO[i]);
                GLenum err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
                if (err != GL_FRAMEBUFFER_COMPLETE) {
                    cout << "Framebuffer completeness in FramebufferObject::impl _check() (index " + to_string(i) + ") is incomplete!" << endl; cout << "Error is: " << err << std::endl;
                    return false;
                }
            }
            return true;
        }
};


namespace Engine {
    namespace epriv {
        struct FramebufferObjectDefaultBindFunctor {
            void operator()(BindableResource* r) const {
                FramebufferObject& fbo = *(FramebufferObject*)r;
                Renderer::setViewport(0, 0, fbo.width(), fbo.height());

                //swap buffers
                ++fbo.m_i->m_CurrentFBOIndex;
                if (fbo.m_i->m_CurrentFBOIndex >= fbo.m_i->m_FBO.size())
                    fbo.m_i->m_CurrentFBOIndex = 0;


                Renderer::bindFBO(fbo);
                for (auto attatchment : fbo.attatchments()) { attatchment.second->bind(); }
            }
        };
        struct FramebufferObjectDefaultUnbindFunctor {
            void operator()(BindableResource* r) const {
                FramebufferObject& fbo = *(FramebufferObject*)r;
                for (auto attatchment : fbo.attatchments()) { attatchment.second->unbind(); }
                Renderer::unbindFBO();
                glm::uvec2 winSize = Resources::getWindowSize();
                Renderer::setViewport(0, 0, winSize.x, winSize.y);
            }
        };
    };
};

epriv::FramebufferObjectDefaultBindFunctor DEFAULT_BIND_FUNCTOR;
epriv::FramebufferObjectDefaultUnbindFunctor DEFAULT_UNBIND_FUNCTOR;

epriv::FramebufferObject::FramebufferObject(string name,uint w,uint h,float divisor,uint swapBufferCount):BindableResource(name),m_i(new impl){
    m_i->_init(w,h,divisor, swapBufferCount);
    setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
    setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
}
epriv::FramebufferObject::FramebufferObject(string name,uint w,uint h,ImageInternalFormat::Format depthInternalFormat,float divisor, uint swapBufferCount):BindableResource(name),m_i(new impl){
    m_i->_init(*this,w,h,depthInternalFormat,divisor, swapBufferCount);
    setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
    setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
}
epriv::FramebufferObject::~FramebufferObject(){ m_i->_destruct(); }
void epriv::FramebufferObject::resize(uint w,uint h){ m_i->_resize(*this,w,h); }
epriv::FramebufferTexture* epriv::FramebufferObject::attatchTexture(Texture* t,FramebufferAttatchment::Attatchment a){ 
    return m_i->_attatchTexture(*this,t,a); 
}
epriv::RenderbufferObject* epriv::FramebufferObject::attatchRenderBuffer(epriv::RenderbufferObject* t){ 
    return m_i->_attatchRenderbuffer(*this,*t); 
}
uint epriv::FramebufferObject::width(){ return m_i->m_FramebufferWidth; }
uint epriv::FramebufferObject::height(){ return m_i->m_FramebufferHeight; }
const GLuint& epriv::FramebufferObject::address() const { return m_i->m_FBO[m_i->m_CurrentFBOIndex]; }
unordered_map<uint,epriv::FramebufferObjectAttatchment*>& epriv::FramebufferObject::attatchments(){ return m_i->m_Attatchments; }
bool epriv::FramebufferObject::check(){ return m_i->_check(); }
float epriv::FramebufferObject::divisor(){ return m_i->m_Divisor; }