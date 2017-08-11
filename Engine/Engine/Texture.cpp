#include "Texture.h"
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
        GLuint m_TextureAddress;
        GLuint m_Type;
        uint m_Width, m_Height;
        uint m_Format;
        ushort m_MipMapLevels;
        bool m_Mipmapped;
        GLuint m_MinFilter; //used to determine filter type for mipmaps

        void _init(GLuint type,Texture* super,string name,sf::Image& img,uint format){
            vector_clear(m_Pixels);
            m_Width = m_Height = m_TextureAddress = 0;
            m_Mipmapped = false;
            m_MinFilter = GL_Linear;
            m_MipMapLevels = 0;
            m_Type = type;
            m_Format = format;
            if(img.getSize().x > 0 && img.getSize().y > 0){
                vector<uchar> p(img.getPixelsPtr(),img.getPixelsPtr() + (img.getSize().x * img.getSize().y * 4));
                m_Pixels = p;
            }
            super->setName(Resources::Detail::ResourceManagement::_incrementName(Resources::Detail::ResourceManagement::m_Textures,name));
            Resources::Detail::ResourceManagement::_addToContainer(Resources::Detail::ResourceManagement::m_Textures,super->name(),boost::shared_ptr<Texture>(super));
            super->load();
        }
        void _load(Texture* super){
            glGenTextures(1, &m_TextureAddress);
            glBindTexture(m_Type, m_TextureAddress);
            if(m_Files.size() == 1 && m_Files[0] != "FRAMEBUFFER" && m_Files[0] != "PIXELS"){//single file, NOT a framebuffer or pixel data texture
                sf::Image image; 
                image.loadFromFile(m_Files[0].c_str());
                _generateFromImage(image,super);
                glBindTexture(m_Type,0);
            }
            else if(m_Files.size() == 1 && m_Files[0] == "PIXELS"){//pixel data image
                sf::Image i;
                i.loadFromMemory(&m_Pixels[0],m_Pixels.size());
                _generateFromImage(i,super);
                glBindTexture(m_Type,0);
                _getPixels();
            }
            else if(m_Files.size() > 1){//cubemap
                for(uint i = 0; i < m_Files.size(); i++){
                    sf::Image img;
                    img.loadFromFile(m_Files[i].c_str());
                    _generateFromImage(img,super);
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, m_Format ,img.getSize().x, img.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE,img.getPixelsPtr());
                }
                super->setFilter(TextureFilter::Linear);
                super->setWrapping(TextureWrap::ClampToEdge);
                glBindTexture(m_Type,0);
            }
            else{//no files
            }
        }
        void _unload(){
            glDeleteTextures(1,&m_TextureAddress);
            glBindTexture(m_Type,0);
            vector_clear(m_Pixels);
        }
        void _generateFromImage(sf::Image& img,Texture* super){
            if(m_Format == GL_RGBA8 || m_Format == GL_SRGB8_ALPHA8){
                glTexImage2D(m_Type,0,m_Format,img.getSize().x,img.getSize().y,0,GL_RGBA,GL_UNSIGNED_BYTE,img.getPixelsPtr());
            }
            else if(m_Format == GL_RGB8 || m_Format == GL_SRGB8){
                glTexImage2D(m_Type,0,m_Format,img.getSize().x,img.getSize().y,0,GL_RGB,GL_UNSIGNED_BYTE,img.getPixelsPtr());
            }
            super->setFilter(TextureFilter::Linear);
            m_Width = img.getSize().x;
            m_Height = img.getSize().y;
        }
        uchar* _getPixels(){
            if(m_Pixels.size() == 0){
                m_Pixels.resize(m_Width*m_Height*4);
                glBindTexture(m_Type,m_TextureAddress);
                glPixelStorei(GL_UNPACK_ALIGNMENT,1);
                if(m_Format == GL_RGBA8 || m_Format == GL_SRGB8_ALPHA8){
                    glGetTexImage(m_Type,0,GL_RGBA,GL_UNSIGNED_BYTE,&m_Pixels[0]);
                }
                else{
                    glGetTexImage(m_Type,0,GL_RGB,GL_UNSIGNED_BYTE,&m_Pixels[0]);
                }
                glBindTexture(m_Type,0);
            }
            return &m_Pixels[0];
        }
        void _enumWrapToGL(uint& gl, TextureWrap::Wrap& wrap){
            if(wrap == TextureWrap::Repeat)              gl = GL_REPEAT;
            else if(wrap == TextureWrap::RepeatMirrored) gl = GL_MIRRORED_REPEAT;
            else if(wrap == TextureWrap::ClampToBorder)  gl = GL_CLAMP_TO_BORDER;
            else if(wrap == TextureWrap::ClampToEdge)    gl = GL_CLAMP_TO_EDGE;
        }
        void _enumFilterToGL(uint& gl, TextureFilter::Filter& filter,bool min){
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
                glBindTexture(m_Type, m_TextureAddress);
                glTexParameteri(m_Type, GL_TEXTURE_BASE_LEVEL, 0);
                glTexParameteri(m_Type, GL_TEXTURE_MAX_LEVEL, 20);

                if(m_MinFilter == GL_LINEAR){        m_MinFilter = GL_LINEAR_MIPMAP_LINEAR; }
                else if(m_MinFilter == GL_NEAREST){  m_MinFilter = GL_NEAREST_MIPMAP_NEAREST; }

                glTexParameteri(m_Type, GL_TEXTURE_MIN_FILTER, m_MinFilter);
                glGenerateMipmap(m_Type);
                m_Mipmapped = true;
                m_MipMapLevels =  glm::log2(glm::max(m_Width, m_Height)) + 1;
                glBindTexture(m_Type, 0);
            }
	    }
};
Texture::Texture(string n,uint w, uint h,GLuint t,uint format):m_i(new impl){ //framebuffer
    m_i->m_Files.push_back("FRAMEBUFFER");
    sf::Image i;
    m_i->_init(t,this,n,i,format);
}
Texture::Texture(sf::Image& img,string n,GLuint t,bool genMipMaps,uint format):m_i(new impl){ //pixels
    m_i->m_Files.push_back("PIXELS");
    m_i->_init(t,this,n,img,format);
    if(genMipMaps){
        m_i->_generateMipmaps();
    }
}
Texture::Texture(string file,string n,GLuint t,bool genMipMaps,uint format):m_i(new impl){ //image file
    m_i->m_Files.push_back(file);
    sf::Image i;
    if(n == "") n = file;
    m_i->_init(t,this,n,i,format);
    if(genMipMaps){
        m_i->_generateMipmaps();
    }
}
Texture::Texture(string files[],string n,GLuint t,bool genMipMaps,uint format):m_i(new impl){ //cubemap images
    for(uint q = 0; q < 6; q++){ 
        m_i->m_Files.push_back(files[q]); 
    }
    sf::Image i;
    m_i->_init(t,this,n,i,format);
    if(genMipMaps){
        m_i->_generateMipmaps();
    }
}
Texture::~Texture(){
    unload();
}
void Texture::render(glm::vec2& pos, glm::vec4& color,float angle, glm::vec2& scl, float depth){
    if(m_i->m_Files.size() != 1){ return; } //this is either not a valid texture or a cubemap, and cannot be rendered 2D.
    Engine::Renderer::Detail::RenderManagement::getTextureRenderQueue().push_back(TextureRenderInfo(name(),pos,color,scl,angle,depth));
}
void Texture::_constructAsFramebuffer(uint w,uint h,float s,int intern,int format,int type,int attatchment){
    m_i->m_Width = w; m_i->m_Height = h;
    glBindTexture(m_i->m_Type, m_i->m_TextureAddress);
    glTexImage2D(m_i->m_Type, 0, intern, (GLsizei)(w*s), (GLsizei)(h*s), 0, format, type, NULL);
    this->setFilter(TextureFilter::Linear);
    this->setWrapping(TextureWrap::ClampToEdge);
    glBindTexture(m_i->m_Type, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attatchment, m_i->m_Type, m_i->m_TextureAddress, 0);
}
void Texture::setXWrapping(TextureWrap::Wrap w){ GLuint gl; m_i->_enumWrapToGL(gl,w); glTexParameteri(m_i->m_Type, GL_TEXTURE_WRAP_S, gl); }
void Texture::setYWrapping(TextureWrap::Wrap w){ GLuint gl; m_i->_enumWrapToGL(gl,w); glTexParameteri(m_i->m_Type, GL_TEXTURE_WRAP_T, gl); }
void Texture::setZWrapping(TextureWrap::Wrap w){
    if(m_i->m_Type != GL_TEXTURE_CUBE_MAP){ return; } //this is not a cubemap
    GLuint gl; m_i->_enumWrapToGL(gl,w); glTexParameteri(m_i->m_Type, GL_TEXTURE_WRAP_R, gl);
}
void Texture::setWrapping(TextureWrap::Wrap w){ Texture::setXWrapping(w); Texture::setYWrapping(w); Texture::setZWrapping(w); }
void Texture::setMinFilter(TextureFilter::Filter f){ 
    GLuint g; m_i->_enumFilterToGL(g,f,true); glTexParameteri(m_i->m_Type,GL_TEXTURE_MIN_FILTER,g); m_MinFilter = g;
}
void Texture::setMaxFilter(TextureFilter::Filter f){ 
    GLuint g; m_i->_enumFilterToGL(g,f,false); glTexParameteri(m_i->m_Type,GL_TEXTURE_MAG_FILTER,g); 
}
void Texture::setFilter(TextureFilter::Filter f){ Texture::setMinFilter(f); Texture::setMaxFilter(f); }

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
ushort Texture::mipmapLevels(){ return m_i->m_MipMapLevels; }
uchar* Texture::pixels(){ return m_i->_getPixels(); }
GLuint& Texture::address(){ return m_i->m_TextureAddress; }
GLuint Texture::type(){ return m_i->m_Type; }
uint Texture::width(){ return m_i->m_Width; }
uint Texture::height(){ return m_i->m_Height; }
