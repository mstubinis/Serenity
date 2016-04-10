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

Texture::Texture(const unsigned char* pixels,unsigned int w, unsigned int h,std::string name,GLuint type){
    _init();
    m_Type = type;
    _loadFromPixels(pixels,w,h,type);
    m_Name = name;

    Resources::Detail::ResourceManagement::m_Textures[m_Name] = boost::shared_ptr<Texture>(this);
}
Texture::Texture(std::string file,std::string name,GLuint type){
    m_Directory = file;
    _init();
    m_Type = type;
    if(file != ""){
		_loadFromFile(file,type);
	}
    m_Name = name;
    if(name == ""){
        m_Name = file;
    }
    if(file != ""){
        Resources::Detail::ResourceManagement::m_Textures[m_Name] = boost::shared_ptr<Texture>(this);
    }
}
Texture::Texture(std::string files[],std::string name,GLuint type){
    m_Directory = "";
    _init();
    m_Type = type;
    _loadFromFilesCubemap(files,type);
    m_Name = name;
    if(name == "Cubemap "){
        unsigned int total = 0;
        for(auto texture:Resources::Detail::ResourceManagement::m_Textures){
            std::string lower = texture.second->getName();
            boost::to_lower(lower);
            if(boost::algorithm::contains(lower,"cubemap")){
                total++;
            }
        }
        m_Name = "Cubemap " + boost::lexical_cast<std::string>(total);
    }
    Resources::Detail::ResourceManagement::m_Textures[m_Name] = boost::shared_ptr<Texture>(this);
}
void Texture::_init(){
    m_Pixels.clear();
    m_TextureAddress = 0;
    m_Width = 0;
    m_Height = 0;
}
Texture::~Texture(){
    glDeleteTextures(1,&m_TextureAddress);
    _init();
}
void Texture::_loadFromPixels(const unsigned char* pixels, unsigned int w, unsigned int h,GLuint type){
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
void Texture::_loadFromFile(std::string file,GLuint type){
    std::string extention;
    for(unsigned int i = file.length() - 4; i < file.length(); i++) extention += tolower(file.at(i));

    sf::Image image;
    image.loadFromFile(file.c_str());
    _loadFromPixels(image.getPixelsPtr(), image.getSize().x, image.getSize().y, type);
}
void Texture::_loadFromFilesCubemap(std::string file[],GLuint type){
    if(type == GL_TEXTURE_CUBE_MAP){
        glGenTextures(1, &m_TextureAddress);
        glBindTexture(type, m_TextureAddress);
        for(unsigned int i = 0; i < 6; i++){
            std::string extention;
            for(unsigned int s = file[i].length() - 4; s < file[i].length(); s++) extention += tolower(file[i].at(s));

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

void Texture::render(glm::vec2 pos, glm::vec4 color,float angle, glm::vec2 scl, float depth){
    Engine::Renderer::Detail::RenderManagement::getTextureRenderQueue().push_back(TextureRenderInfo(m_Name,pos,color,scl,angle,depth));
}
const unsigned char* Texture::getPixels(){
    if(m_Pixels.size() == 0){
        m_Pixels.resize(m_Width*m_Height*4);
        glBindTexture(m_Type,m_TextureAddress);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glGetTexImage(m_Type,0,GL_RGBA,GL_UNSIGNED_BYTE,&m_Pixels[0]);
    }
    return &m_Pixels[0];
}