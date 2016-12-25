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

using namespace Engine;

class Texture::impl final{
    public:
        std::vector<uchar> m_Pixels;
        std::vector<std::string> m_Files; //if non cubemap, this is only 1 file and a length of 1
        GLuint m_TextureAddress;
        GLuint m_Type;
        uint m_Width, m_Height;

        void _init(GLuint type,Texture* super,std::string name,sf::Image& img){
            m_Pixels.clear();
            m_Width = m_Height = m_TextureAddress = 0;
            m_Type = type;

            if(img.getSize().x > 0 && img.getSize().y > 0){
                std::vector<uchar> p(img.getPixelsPtr(),img.getPixelsPtr() + (img.getSize().x * img.getSize().y * 4));
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
                sf::Image image; image.loadFromFile(m_Files[0].c_str());
                _generateFromImage(image);
				glBindTexture(m_Type,0);
            }
            else if(m_Files.size() == 1 && m_Files[0] == "PIXELS"){//pixel data image
                sf::Image i; i.loadFromMemory(&m_Pixels[0],m_Pixels.size());
                _generateFromImage(i);
				glBindTexture(m_Type,0);
                _getPixels();
            }
            else if(m_Files.size() > 1){//cubemap
                for(uint i = 0; i < m_Files.size(); i++){
                    sf::Image image; image.loadFromFile(m_Files[i].c_str());
                    _generateFromImage(image);
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA,image.getSize().x, image.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE,image.getPixelsPtr());
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
        }
        void _generateFromImage(sf::Image& image){
            glTexImage2D(m_Type, 0, GL_RGBA, image.getSize().x, image.getSize().y, 0, GL_RGBA,GL_UNSIGNED_BYTE, image.getPixelsPtr());
            //glTexParameteri(m_Type, GL_TEXTURE_WRAP_S, GL_REPEAT);
            //(m_Type, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(m_Type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(m_Type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glGenerateMipmap(m_Type);

            m_Width = image.getSize().x; m_Height = image.getSize().y;
        }
        uchar* _getPixels(){
            if(m_Pixels.size() == 0){
                m_Pixels.resize(m_Width*m_Height*4);glBindTexture(m_Type,m_TextureAddress);glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                glGetTexImage(m_Type,0,GL_RGBA,GL_UNSIGNED_BYTE,&m_Pixels[0]);
            }
            return &m_Pixels[0];
        }
};
Texture::Texture(std::string _name,uint w, uint h,GLuint type):m_i(new impl()){
    m_i->m_Files.push_back("FRAMEBUFFER");
    sf::Image i;
    m_i->_init(type,this,_name,i);
}
Texture::Texture(sf::Image& img,std::string _name,GLuint type):m_i(new impl()){
    m_i->m_Files.push_back("PIXELS");
    m_i->_init(type,this,_name,img);
}
Texture::Texture(std::string file,std::string _name,GLuint type):m_i(new impl()){
    m_i->m_Files.push_back(file);
    sf::Image i;
    if(_name == "") _name = file;
    m_i->_init(type,this,_name,i);
}
Texture::Texture(std::string files[],std::string _name,GLuint type):m_i(new impl()){
    for(uint i = 0; i < 6; i++){ m_i->m_Files.push_back(files[i]); }
    sf::Image i;
    m_i->_init(type,this,_name,i);
}
Texture::~Texture(){
    unload();
}
void Texture::render(glm::vec2& pos, glm::vec4& color,float angle, glm::vec2& scl, float depth){
    Engine::Renderer::Detail::RenderManagement::getTextureRenderQueue().push_back(TextureRenderInfo(name(),pos,color,scl,angle,depth));
}
void Texture::_constructAsFramebuffer(uint w,uint h,float scale,int intern,int format,int type,int attatchment,uint multisample){
    m_i->m_Width = w; m_i->m_Height = h;

	if(multisample == 0){
		glBindTexture(m_i->m_Type, m_i->m_TextureAddress);
		glTexImage2D(m_i->m_Type, 0, intern, (GLsizei)(w*scale), (GLsizei)(h*scale), 0, format, type, 0);
		//glTexParameteri(m_i->m_Type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameteri(m_i->m_Type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	    glTexParameterf(m_i->m_Type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameterf(m_i->m_Type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(m_i->m_Type, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, attatchment, m_i->m_Type, m_i->m_TextureAddress, 0);
	}
	else{
		m_i->m_Type = GL_TEXTURE_2D_MULTISAMPLE;
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_i->m_TextureAddress);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, multisample, intern, (GLsizei)(w*scale),(GLsizei)(h*scale),GL_TRUE);
		glTexParameterf(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, attatchment, GL_TEXTURE_2D_MULTISAMPLE, m_i->m_TextureAddress, 0);
	}
}
void Texture::load(){
    if(!isLoaded()){
        m_i->_load();
        EngineResource::load();
    }
}
void Texture::unload(){
    if(isLoaded()){
        m_i->_unload();
        EngineResource::unload();
    }
}
uchar* Texture::pixels(){ return m_i->_getPixels(); }
GLuint& Texture::address(){ return m_i->m_TextureAddress; }
GLuint Texture::type(){ return m_i->m_Type; }
uint Texture::width(){ return m_i->m_Width; }
uint Texture::height(){ return m_i->m_Height; }