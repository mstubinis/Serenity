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

        void _init(GLuint type,Texture* super,string name,sf::Image& img,uint format){
            m_Pixels.clear();
            m_Width = m_Height = m_TextureAddress = 0;
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
        void _load(){
            glGenTextures(1, &m_TextureAddress);
            glBindTexture(m_Type, m_TextureAddress);

            if(m_Files.size() == 1 && m_Files[0] != "FRAMEBUFFER" && m_Files[0] != "PIXELS"){//single file, NOT a framebuffer or pixel data texture
                sf::Image image; 
                image.loadFromFile(m_Files[0].c_str());
                _generateFromImage(image);
                glBindTexture(m_Type,0);
            }
            else if(m_Files.size() == 1 && m_Files[0] == "PIXELS"){//pixel data image
                sf::Image i;
                i.loadFromMemory(&m_Pixels[0],m_Pixels.size());
                _generateFromImage(i);
                glBindTexture(m_Type,0);
                _getPixels();
            }
            else if(m_Files.size() > 1){//cubemap
                for(uint i = 0; i < m_Files.size(); i++){
                    sf::Image image;
                    image.loadFromFile(m_Files[i].c_str());
                    _generateFromImage(image);
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, m_Format ,image.getSize().x, image.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE,image.getPixelsPtr());
                }
                glTexParameteri(m_Type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(m_Type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(m_Type, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTexParameteri(m_Type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(m_Type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glGenerateMipmap(m_Type);
                glBindTexture(m_Type,0);
            }
            else{//no files
            }
        }
        void _unload(){
            glDeleteTextures(1,&m_TextureAddress);
            glBindTexture(m_Type,0);
        }
        void _generateFromImage(sf::Image& img){
            if(m_Format == GL_RGBA8 || m_Format == GL_SRGB8_ALPHA8){
                glTexImage2D(m_Type,0,m_Format,img.getSize().x,img.getSize().y,0,GL_RGBA,GL_UNSIGNED_BYTE,img.getPixelsPtr());
            }
            else if(m_Format == GL_RGB8 || m_Format == GL_SRGB8){
                glTexImage2D(m_Type,0,m_Format,img.getSize().x,img.getSize().y,0,GL_RGB,GL_UNSIGNED_BYTE,img.getPixelsPtr());
            }
            glTexParameteri(m_Type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(m_Type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glGenerateMipmap(m_Type);

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
            }
            return &m_Pixels[0];
        }
};
Texture::Texture(string n,uint w, uint h,GLuint t,uint format):m_i(new impl){ //framebuffer
    m_i->m_Files.push_back("FRAMEBUFFER");
    sf::Image i;
    m_i->_init(t,this,n,i,format);
}
Texture::Texture(sf::Image& img,string n,GLuint t,uint format):m_i(new impl){ //pixels
    m_i->m_Files.push_back("PIXELS");
    m_i->_init(t,this,n,img,format);
}
Texture::Texture(string file,string n,GLuint t,uint format):m_i(new impl){ //image file
    m_i->m_Files.push_back(file);
    sf::Image i;
    if(n == "") n = file;
    m_i->_init(t,this,n,i,format);
}
Texture::Texture(string files[],string n,GLuint t,uint format):m_i(new impl){ //cubemap images
    for(uint q = 0; q < 6; q++){ 
	m_i->m_Files.push_back(files[q]); 
    }
    sf::Image i;
    m_i->_init(t,this,n,i,format);
}
Texture::~Texture(){
    unload();
}
void Texture::render(glm::vec2& pos, glm::vec4& color,float angle, glm::vec2& scl, float depth){
    if(m_i->m_Files.size() != 1){ return; } //this is either not a valid texture or a cubemap, and cannot be rendered 2D.
    Engine::Renderer::Detail::RenderManagement::getTextureRenderQueue().push_back(TextureRenderInfo(name(),pos,color,scl,angle,depth));
}
void Texture::_constructAsFramebuffer(uint w,uint h,float scale,int intern,int format,int type,int attatchment){
    m_i->m_Width = w; m_i->m_Height = h;
    glBindTexture(m_i->m_Type, m_i->m_TextureAddress);
    glTexImage2D(m_i->m_Type, 0, intern, (GLsizei)(w*scale), (GLsizei)(h*scale), 0, format, type, 0);
    glTexParameterf(m_i->m_Type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(m_i->m_Type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(m_i->m_Type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(m_i->m_Type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(m_i->m_Type, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attatchment, m_i->m_Type, m_i->m_TextureAddress, 0);
}
void Texture::setXWrapping(TextureWrap::Wrap w){
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
}
void Texture::setYWrapping(TextureWrap::Wrap w){
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
void Texture::setZWrapping(TextureWrap::Wrap w){
    if(m_Type != GL_TEXTURE_CUBE_MAP){ return; } //this is not a cubemap
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
}
void Texture::setWrapping(TextureWrap::Wrap w){ Texture::setXWrapping(w); Texture::setYWrapping(w); Texture::setZWrapping(w); }

void Texture::setMinFilter(TextureFilter::Filter f){
}
void Texture::setMaxFilter(TextureFilter::Filter f){
}
void Texture::setFilter(TextureFilter::Filter f){ Texture::setMinFilter(f); Texture::setMaxFilter(f); }

void Texture::load(){
    if(!isLoaded()){
        m_i->_load();
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
uchar* Texture::pixels(){ return m_i->_getPixels(); }
GLuint& Texture::address(){ return m_i->m_TextureAddress; }
GLuint Texture::type(){ return m_i->m_Type; }
uint Texture::width(){ return m_i->m_Width; }
uint Texture::height(){ return m_i->m_Height; }
