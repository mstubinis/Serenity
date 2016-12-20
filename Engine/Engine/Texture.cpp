#include "Texture.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "ShaderProgram.h"
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>



using namespace Engine;

class Texture::impl final{
    private:
        std::vector<unsigned char> m_Pixels;
        std::string m_Directory;
        GLuint m_TextureAddress;
        GLuint m_Type;
        unsigned int m_Width, m_Height;
        void _loadFromPixels(const unsigned char* pixels,unsigned int w, unsigned int h,GLuint type){
			m_Type = type;
            if(type == GL_TEXTURE_2D){
                glGenTextures(1, &m_TextureAddress);

                glBindTexture(type, m_TextureAddress);
                glTexImage2D(type, 0, GL_RGBA, w, h, 0, GL_RGBA,GL_UNSIGNED_BYTE, pixels);
                glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glGenerateMipmap(type);

                m_Width = w;
                m_Height = h;
            }
        }
        void _loadFromFile(std::string file,GLuint type){
			m_Type = type;
            sf::Image image;
            image.loadFromFile(file.c_str());
            _loadFromPixels(image.getPixelsPtr(), image.getSize().x, image.getSize().y, type);
        }
        void _loadFromFilesCubemap(std::string file[],GLuint type){
			m_Type = type;
            if(type == GL_TEXTURE_CUBE_MAP){
                glGenTextures(1, &m_TextureAddress);
                glBindTexture(type, m_TextureAddress);
                for(unsigned int i = 0; i < 6; i++){
                    sf::Image image;
                    image.loadFromFile(file[i].c_str());
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA,image.getSize().x, image.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE,image.getPixelsPtr());
                }
                glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
            }
        }
    public:
        void _construct(const unsigned char* pixels,uint w, uint h,std::string name,GLuint type,Texture* super){
            _init();
            _loadFromPixels(pixels,w,h,type);
			super->setName(Resources::Detail::ResourceManagement::_incrementName(Resources::Detail::ResourceManagement::m_Textures,name));
        }
        void _construct(std::string file,std::string name,GLuint type,Texture* super){
			_init();
            m_Directory = file;
            if(file != ""){
                _loadFromFile(file,type);
            }
			super->setName(name);
            if(name == ""){
				super->setName(file);
            }
			super->setName(Resources::Detail::ResourceManagement::_incrementName(Resources::Detail::ResourceManagement::m_Textures,super->name()));
        }
        void _construct(std::string files[],std::string name,GLuint type,Texture* super){
			_init();
            _loadFromFilesCubemap(files,type);
			super->setName(Resources::Detail::ResourceManagement::_incrementName(Resources::Detail::ResourceManagement::m_Textures,name));
        }
        void _init(){
			m_Directory = "";
            m_Pixels.clear();
            m_Width = m_Height = m_TextureAddress = 0;
        }
        void _destruct(){
            glDeleteTextures(1,&m_TextureAddress);
            _init();
        }
        unsigned char* _getPixels(){
            if(m_Pixels.size() == 0){
                m_Pixels.resize(m_Width*m_Height*4);
                glBindTexture(m_Type,m_TextureAddress);
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                glGetTexImage(m_Type,0,GL_RGBA,GL_UNSIGNED_BYTE,&m_Pixels[0]);
            }
            return &m_Pixels[0];
        }
        const GLuint _address() const { return m_TextureAddress; }
        const GLuint _type() const { return m_Type; }
        const uint _width() const { return m_Width; }
        const uint _height() const { return m_Height; }
};


Texture::Texture(const unsigned char* pixels,uint w, uint h,std::string _name,GLuint type):m_i(new impl()){
	m_i->_construct(pixels,w,h,_name,type,this);
	Resources::Detail::ResourceManagement::m_Textures[this->name()] = boost::shared_ptr<Texture>(this);
}

Texture::Texture(std::string file,std::string _name,GLuint type):m_i(new impl()){
    m_i->_construct(file,_name,type,this);
    if(file != ""){
        Resources::Detail::ResourceManagement::m_Textures[this->name()] = boost::shared_ptr<Texture>(this);
    }
}
Texture::Texture(std::string files[],std::string _name,GLuint type):m_i(new impl()){
    m_i->_construct(files,_name,type,this);
    Resources::Detail::ResourceManagement::m_Textures[this->name()] = boost::shared_ptr<Texture>(this);
}
Texture::~Texture(){
    m_i->_destruct();
}
void Texture::render(glm::vec2 pos, glm::vec4 color,float angle, glm::vec2 scl, float depth){
    Engine::Renderer::Detail::RenderManagement::getTextureRenderQueue().push_back(TextureRenderInfo(this->name(),pos,color,scl,angle,depth));
}
unsigned char* Texture::pixels(){ return m_i->_getPixels(); }
GLuint Texture::address() { return m_i->_address(); }
GLuint Texture::type() { return m_i->_type(); }
uint Texture::width() { return m_i->_width(); }
uint Texture::height() { return m_i->_height(); }