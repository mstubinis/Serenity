#include "Texture.h"
#include "Mesh.h"
#include "Skybox.h"
#include "Engine_Window.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "ShaderProgram.h"
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <iostream>

using namespace Engine;
using namespace std;

class Texture::impl final{
    public:
        vector<uchar> m_Pixels;
        vector<string> m_Files; //if non cubemap, this is only 1 file and a length of 1
        vector<GLuint> m_TextureAddress;
        GLuint m_Type;
        uint m_Width, m_Height;
        ImageInternalFormat::Format m_InternalFormat;
        ImagePixelFormat::Format m_PixelFormat;
        ImagePixelType::Type m_PixelType;
        ushort m_MipMapLevels;
        bool m_Mipmapped;
        bool m_IsToBeMipmapped;
        GLuint m_MinFilter; //used to determine filter type for mipmaps

        void _init(uint w,uint h,float divisor,ImagePixelType::Type pxlType,GLuint type,Texture* super,string n,sf::Image& i,ImageInternalFormat::Format internFormat,ImagePixelFormat::Format pxlFormat,bool genMipMaps){
            m_PixelFormat = pxlFormat;
            m_PixelType = pxlType;
            _baseInit(type,super,n,i,internFormat,genMipMaps);
            m_Width = uint(float(w) * divisor); m_Height = uint(float(h) * divisor);
            super->load();
        }
        void _init(GLuint type,Texture* super,string name,sf::Image& img,ImageInternalFormat::Format format,bool genMipMaps){
            _baseInit(type,super,name,img,format,genMipMaps);
            super->load();
        }
        void _baseInit(GLuint type,Texture* super,string n,sf::Image& i,ImageInternalFormat::Format internFormat,bool genMipMaps){
            vector_clear(m_Pixels);
            m_Width = m_Height = 0;
            m_Mipmapped = false;
            m_IsToBeMipmapped = genMipMaps;
            m_MinFilter = GL_LINEAR;
            m_MipMapLevels = 0;
            m_Type = type;
            m_InternalFormat = internFormat;
            if(i.getSize().x > 0 && i.getSize().y > 0){
                vector<uchar> p(i.getPixelsPtr(),i.getPixelsPtr()+(i.getSize().x*i.getSize().y*4));
                m_Pixels = p;
            }
            super->setName(Resources::Detail::ResourceManagement::_incrementName(Resources::Detail::ResourceManagement::m_Textures,n));
            Resources::Detail::ResourceManagement::_addToContainer(Resources::Detail::ResourceManagement::m_Textures,super->name(),boost::shared_ptr<Texture>(super));
        }
        void _load(Texture* super){
            if(m_TextureAddress.size() == 0)
                m_TextureAddress.push_back(0); //vector.at(0) will be the default address. at(1) is the colvoluted map address (for cubemap only)
            glGenTextures(1, &m_TextureAddress.at(0));
            glBindTexture(m_Type, m_TextureAddress.at(0));
            if(m_Files.size() == 1 && m_Files[0] != "FRAMEBUFFER" && m_Files[0] != "PIXELS"){//single file, NOT a framebuffer or pixel data texture
                sf::Image i;i.loadFromFile(m_Files[0].c_str());
                _generateFromImage(i,super);
                glBindTexture(m_Type,0);
            }
            else if(m_Files.size() == 1 && m_Files[0] == "PIXELS"){//pixel data image
                sf::Image i;i.loadFromMemory(&m_Pixels[0],m_Pixels.size());
                _generateFromImage(i,super);
                glBindTexture(m_Type,0);
                _getPixels();
            }
            else if(m_Files.size() == 1 && m_Files[0] == "FRAMEBUFFER"){//Framebuffer
				glBindTexture(m_Type,m_TextureAddress.at(0));
				_buildTexImage2D(m_Type,ImageInternalFormat::at(m_InternalFormat),GLsizei(m_Width),GLsizei(m_Height),ImagePixelFormat::at(m_PixelFormat),ImagePixelType::at(m_PixelType));
				super->setFilter(TextureFilter::Linear);
                super->setWrapping(TextureWrap::ClampToEdge);
                glBindTexture(m_Type,0);
            }
            else if(m_Files.size() > 1){//cubemap
                for(uint k = 0; k < m_Files.size(); k++){
                    sf::Image i;i.loadFromFile(m_Files[k].c_str());
                    _generateFromImage(i,super);
                    _buildTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+k,ImageInternalFormat::at(m_InternalFormat),i,ImagePixelFormat::at(m_PixelFormat),GL_UNSIGNED_BYTE);
                }
                super->setFilter(TextureFilter::Linear);
                super->setWrapping(TextureWrap::ClampToEdge);
                glBindTexture(m_Type,0);
            }
            else{//no files
            }
            if(m_IsToBeMipmapped){
                _generateMipmaps();
            }
        }
        void _buildTexImage2D(GLuint targetType,GLuint internal,sf::Image& img, GLuint pxlFormat,GLuint pxlType,float divisor=1.0f){
            glTexImage2D(targetType,0,internal,GLsizei(img.getSize().x*divisor),GLsizei(img.getSize().y*divisor),0,pxlFormat,pxlType,img.getPixelsPtr());
        }
        void _buildTexImage2D(GLuint targetType,GLuint internal,GLsizei w,GLsizei h, GLuint pxlFormat,GLuint pxlType){
            glTexImage2D(targetType,0,internal,w,h,0,pxlFormat,pxlType,NULL);
        }
        void _unload(){
            for(uint i = 0; i < m_TextureAddress.size(); i++){
                glDeleteTextures(1,&m_TextureAddress.at(i));
            }
            glBindTexture(m_Type,0);
            m_MipMapLevels = 0;
            m_Mipmapped = false;
            vector_clear(m_Pixels);
            vector_clear(m_TextureAddress);
        }
        void _generateFromImage(sf::Image& i,Texture* super){
            if(m_InternalFormat == ImageInternalFormat::RGBA8 || m_InternalFormat == ImageInternalFormat::SRGB8_ALPHA8){
                m_PixelFormat = ImagePixelFormat::RGBA;
            }
            else if(m_InternalFormat == ImageInternalFormat::RGB8 || m_InternalFormat == ImageInternalFormat::SRGB8){
                m_PixelFormat = ImagePixelFormat::RGB;
            }
            _buildTexImage2D(m_Type,ImageInternalFormat::at(m_InternalFormat),i,ImagePixelFormat::at(m_PixelFormat),GL_UNSIGNED_BYTE);
            super->setFilter(TextureFilter::Linear);
            m_Width = i.getSize().x;
            m_Height = i.getSize().y;
        }
        uchar* _getPixels(){
            if(m_Pixels.size() == 0){
                m_Pixels.resize(m_Width*m_Height*4);
                glBindTexture(m_Type,m_TextureAddress.at(0));
                glPixelStorei(GL_UNPACK_ALIGNMENT,1);
                glGetTexImage(m_Type,0,ImagePixelFormat::at(m_PixelFormat),GL_UNSIGNED_BYTE,&m_Pixels[0]);
                glBindTexture(m_Type,0);
            }
            return &m_Pixels[0];
        }
        static void _enumWrapToGL(uint& gl, TextureWrap::Wrap& wrap){
            if(wrap == TextureWrap::Repeat)              gl = GL_REPEAT;
            else if(wrap == TextureWrap::RepeatMirrored) gl = GL_MIRRORED_REPEAT;
            else if(wrap == TextureWrap::ClampToBorder)  gl = GL_CLAMP_TO_BORDER;
            else if(wrap == TextureWrap::ClampToEdge)    gl = GL_CLAMP_TO_EDGE;
        }
        static void _enumFilterToGL(uint& gl, TextureFilter::Filter& filter,bool min){
            if(min == true){
                if(filter == TextureFilter::Linear)                       gl = GL_LINEAR;
                else if(filter == TextureFilter::Nearest)                 gl = GL_NEAREST;
                else if(filter == TextureFilter::Linear_Mipmap_Linear)    gl = GL_LINEAR_MIPMAP_LINEAR;
                else if(filter == TextureFilter::Linear_Mipmap_Nearest)   gl = GL_LINEAR_MIPMAP_NEAREST;
                else if(filter == TextureFilter::Nearest_Mipmap_Linear)   gl = GL_NEAREST_MIPMAP_LINEAR;
                else if(filter == TextureFilter::Nearest_Mipmap_Nearest)  gl = GL_NEAREST_MIPMAP_NEAREST;
            }
            else{
                if(filter == TextureFilter::Linear)                       gl = GL_LINEAR;
                else if(filter == TextureFilter::Nearest)                 gl = GL_NEAREST;
                else if(filter == TextureFilter::Linear_Mipmap_Linear)    gl = GL_LINEAR;
                else if(filter == TextureFilter::Linear_Mipmap_Nearest)   gl = GL_LINEAR;
                else if(filter == TextureFilter::Nearest_Mipmap_Linear)   gl = GL_NEAREST;
                else if(filter == TextureFilter::Nearest_Mipmap_Nearest)  gl = GL_NEAREST;
            }
        }
        void _generateMipmaps(){
            if(m_Mipmapped == false){
                glBindTexture(m_Type, m_TextureAddress.at(0));
                glTexParameteri(m_Type, GL_TEXTURE_BASE_LEVEL, 0);
                if(m_MinFilter == GL_LINEAR){        m_MinFilter = GL_LINEAR_MIPMAP_LINEAR; }
                else if(m_MinFilter == GL_NEAREST){  m_MinFilter = GL_NEAREST_MIPMAP_NEAREST; }
                glTexParameteri(m_Type, GL_TEXTURE_MIN_FILTER, m_MinFilter);
                glGenerateMipmap(m_Type);
                m_Mipmapped = true;
                m_MipMapLevels = uint(glm::log2(glm::max(m_Width, m_Height)) + 1.0f);
                glBindTexture(m_Type, 0);
            }
      }
};
Texture::Texture(std::string n,uint w, uint h,ImageInternalFormat::Format internal,ImagePixelFormat::Format pxlFormat,ImagePixelType::Type pxlType,GLuint t,float divisor):m_i(new impl){ //framebuffer
    m_i->m_Files.push_back("FRAMEBUFFER");
    sf::Image i;
    m_i->_init(w,h,divisor,pxlType,t,this,n,i,internal,pxlFormat,false);
}
Texture::Texture(sf::Image& img,string n,GLuint t,bool genMipMaps,ImageInternalFormat::Format internalFormat):m_i(new impl){ //pixels
    m_i->m_Files.push_back("PIXELS");
    m_i->_init(t,this,n,img,internalFormat,genMipMaps);
}
Texture::Texture(string file,string n,GLuint t,bool genMipMaps,ImageInternalFormat::Format internalFormat):m_i(new impl){ //image file
    m_i->m_Files.push_back(file);
    sf::Image i;
    if(n == "") n = file;
    m_i->_init(t,this,n,i,internalFormat,genMipMaps);
}
Texture::Texture(string files[],string n,GLuint t,bool genMipMaps,ImageInternalFormat::Format internalFormat):m_i(new impl){ //cubemap images
    for(uint q = 0; q < 6; q++){ 
        m_i->m_Files.push_back(files[q]); 
    }
    sf::Image i;
    m_i->_init(t,this,n,i,internalFormat,genMipMaps);
}
Texture::~Texture(){
    unload();
}
void Texture::render(glm::vec2& pos, glm::vec4& color,float angle, glm::vec2& scl, float depth){
    if(m_i->m_Files.size() != 1)return;
    Engine::Renderer::Detail::RenderManagement::getTextureRenderQueue().push_back(TextureRenderInfo(name(),pos,color,scl,angle,depth));
}
void Texture::setXWrapping(TextureWrap::Wrap w){ Texture::setXWrapping(m_i->m_Type,w); }
void Texture::setYWrapping(TextureWrap::Wrap w){ Texture::setYWrapping(m_i->m_Type,w); }
void Texture::setZWrapping(TextureWrap::Wrap w){ Texture::setZWrapping(m_i->m_Type,w); }
void Texture::setWrapping(TextureWrap::Wrap w){ Texture::setWrapping(m_i->m_Type,w); }
void Texture::setMinFilter(TextureFilter::Filter f){ Texture::setMinFilter(m_i->m_Type,f); m_i->m_MinFilter = f; }
void Texture::setMaxFilter(TextureFilter::Filter f){ Texture::setMaxFilter(m_i->m_Type,f); }
void Texture::setFilter(TextureFilter::Filter f){ Texture::setFilter(m_i->m_Type,f); }
void Texture::setXWrapping(GLuint type,TextureWrap::Wrap w){ GLuint gl; Texture::impl::_enumWrapToGL(gl,w); glTexParameteri(type, GL_TEXTURE_WRAP_S, gl); }
void Texture::setYWrapping(GLuint type,TextureWrap::Wrap w){ GLuint gl; Texture::impl::_enumWrapToGL(gl,w); glTexParameteri(type, GL_TEXTURE_WRAP_T, gl); }
void Texture::setZWrapping(GLuint type,TextureWrap::Wrap w){
    if(type != GL_TEXTURE_CUBE_MAP)return;GLuint gl;Texture::impl::_enumWrapToGL(gl,w);glTexParameteri(type,GL_TEXTURE_WRAP_R,gl);
}
void Texture::setWrapping(GLuint type,TextureWrap::Wrap w){ Texture::setXWrapping(type,w); Texture::setYWrapping(type,w); Texture::setZWrapping(type,w); }
void Texture::setMinFilter(GLuint type,TextureFilter::Filter f){ GLuint g; Texture::impl::_enumFilterToGL(g,f,true); glTexParameteri(type,GL_TEXTURE_MIN_FILTER,g); }
void Texture::setMaxFilter(GLuint type,TextureFilter::Filter f){ GLuint g; Texture::impl::_enumFilterToGL(g,f,false); glTexParameteri(type,GL_TEXTURE_MAG_FILTER,g);  }
void Texture::setFilter(GLuint type,TextureFilter::Filter f){ Texture::setMinFilter(type,f); Texture::setMaxFilter(type,f); }

void Texture::load(){
    if(!isLoaded()){
        m_i->_load(this);
        cout << "(Texture) ";
        EngineResource::load();
    }
}
void Texture::unload(){
    if(isLoaded() && useCount() == 0){
        m_i->_unload();
        cout << "(Texture) ";
        EngineResource::unload();
    }
}
void Texture::genPBREnvMapData(uint convoludeTextureSize,uint preEnvFilterSize){
    if(m_i->m_Type != GL_TEXTURE_CUBE_MAP){
        cout << "(Texture) : Only cubemaps can be precomputed for IBL. Ignoring genPBREnvMapData() call..." << endl; return;
    }
    uint size = convoludeTextureSize;
    //cleanup previous convolute operation
    if(m_i->m_TextureAddress.size() >= 2){
        glDeleteTextures(1,&m_i->m_TextureAddress.at(1));
    }
    else if(m_i->m_TextureAddress.size() == 1){
        m_i->m_TextureAddress.push_back(0); // this should be element 2 (.at(1)) now
    }
    Renderer::unbindFBO();
    GLuint captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);
    Renderer::bindFBO(captureFBO);
    Renderer::bindRBO(captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, size, size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO); 

    glGenTextures(1, &m_i->m_TextureAddress.at(1));
    glBindTexture(m_i->m_Type, m_i->m_TextureAddress.at(1));
    for (uint i = 0; i < 6; ++i){
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, NULL);
    }

    this->setWrapping(TextureWrap::ClampToEdge);
    this->setFilter(TextureFilter::Linear);
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 3000.0f);
    glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };
    ShaderP* p = Resources::getShaderProgram("Cubemap_Convolude"); p->bind();
    Renderer::bindTexture("cubemap",address(),0,m_i->m_Type);
    Renderer::setViewport(0,0,size,size);
    for (uint i = 0; i < 6; ++i){
        glm::mat4 vp = captureProjection * captureViews[i];
        Renderer::sendUniformMatrix4f("VP", vp);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_i->m_TextureAddress.at(1), 0);
        Renderer::Settings::clear(true,true,false);
        Skybox::bindMesh();
    }
    cout << "---- " + this->name() + " (Cubemap): convolution done ----" << endl;
    Resources::getWindow()->display(); //prevent opengl & windows timeout
    p->unbind();

    //now gen EnvPrefilterMap for specular IBL. cleanup previous EnvPrefilterMap operation
    if(m_i->m_TextureAddress.size() >= 3){
        glDeleteTextures(1,&m_i->m_TextureAddress.at(2));
    }
    else if(m_i->m_TextureAddress.size() == 2){
        m_i->m_TextureAddress.push_back(0);
    }
    size = preEnvFilterSize;
    glGenTextures(1, &m_i->m_TextureAddress.at(2));
    glBindTexture(m_i->m_Type, m_i->m_TextureAddress.at(2));
    for (uint i = 0; i < 6; ++i){
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, NULL);
    }
    this->setWrapping(TextureWrap::ClampToEdge);
    this->setMinFilter(TextureFilter::Linear_Mipmap_Linear);
    this->setMaxFilter(TextureFilter::Linear);
    glGenerateMipmap(m_i->m_Type);
    p = Resources::getShaderProgram("Cubemap_Prefilter_Env"); p->bind();
    Renderer::bindTexture("cubemap",address(),0,m_i->m_Type);
    Renderer::sendUniform1f("PiFourDividedByResSquaredTimesSix",12.56637f / float((this->width() * this->width())*6));
    Renderer::sendUniform1i("NUM_SAMPLES",32);
    uint maxMipLevels = 5;
    for (uint m = 0; m < maxMipLevels; ++m){
        uint mipSize  = uint(size * glm::pow(0.5, m)); // reisze framebuffer according to mip-level size.
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, mipSize, mipSize);
        Renderer::setViewport(0,0,mipSize,mipSize);
        float roughness = (float)m/(float)(maxMipLevels-1);
        Renderer::sendUniform1f("roughness",roughness);
        float a = roughness * roughness;
        Renderer::sendUniform1f("a2",a*a);
        for (uint i = 0; i < 6; ++i){
            glm::mat4 vp = captureProjection * captureViews[i];
            Renderer::sendUniformMatrix4f("VP", vp);
            glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,m_i->m_TextureAddress.at(2),m);
            Renderer::Settings::clear(true,true,false);
            Skybox::bindMesh();
        }
    }
    cout << "---- " + this->name() + " (Cubemap): prefilter done ----" << endl;
    Resources::getWindow()->display(); //prevent opengl & windows timeout

    glDeleteRenderbuffers(1, &captureRBO);
    glDeleteFramebuffers(1, &captureFBO);
    Renderer::unbindReadFBO();
    Renderer::unbindDrawFBO();
    Renderer::setViewport(0,0,Resources::getWindowSize().x,Resources::getWindowSize().y);
}
bool Texture::mipmapped(){ return m_i->m_Mipmapped; }
ushort Texture::mipmapLevels(){ return m_i->m_MipMapLevels; }
uchar* Texture::pixels(){ return m_i->_getPixels(); }
GLuint& Texture::address(){ return m_i->m_TextureAddress.at(0); }
GLuint& Texture::address(uint index){ return m_i->m_TextureAddress.at(index); }
uint Texture::numAddresses(){ return m_i->m_TextureAddress.size(); }
GLuint Texture::type(){ return m_i->m_Type; }
uint Texture::width(){ return m_i->m_Width; }
uint Texture::height(){ return m_i->m_Height; }
ImageInternalFormat::Format Texture::internalFormat(){ return m_i->m_InternalFormat; }
ImagePixelFormat::Format Texture::pixelFormat(){ return m_i->m_PixelFormat; }
ImagePixelType::Type Texture::pixelType(){ return m_i->m_PixelType; }
