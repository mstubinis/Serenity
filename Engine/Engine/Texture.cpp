#include "Texture.h"
#include "Mesh.h"
#include "Skybox.h"
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
        uint m_Format;
        ushort m_MipMapLevels;
        bool m_Mipmapped;
        bool m_IsToBeMipmapped;
        GLuint m_MinFilter; //used to determine filter type for mipmaps

        void _init(GLuint type,Texture* super,string name,sf::Image& img,uint format,bool genMipMaps){
            vector_clear(m_Pixels);
            m_Width = m_Height = 0;
            m_Mipmapped = false;
            m_IsToBeMipmapped = genMipMaps;
            m_MinFilter = GL_LINEAR;
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
            if(m_TextureAddress.size() == 0)
                m_TextureAddress.push_back(0); //vector.at(0) will be the default address. at(1) is the colvoluted map address (for cubemap only)
            
            glGenTextures(1, &m_TextureAddress.at(0));
            glBindTexture(m_Type, m_TextureAddress.at(0));
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
            if(m_IsToBeMipmapped){
                _generateMipmaps();
            }
        }
        void _unload(){
            glDeleteTextures(1,&m_TextureAddress.at(0));
            if(m_TextureAddress.size() == 2)
                glDeleteTextures(1,&m_TextureAddress.at(1));
            glBindTexture(m_Type,0);
            m_Width = m_Height = 0;
            m_MipMapLevels = 0;
            m_Mipmapped = false;
            vector_clear(m_Pixels);
            vector_clear(m_TextureAddress);
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
                glBindTexture(m_Type,m_TextureAddress.at(0));
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
Texture::Texture(string n,uint w, uint h,GLuint t,uint format):m_i(new impl){ //framebuffer
    m_i->m_Files.push_back("FRAMEBUFFER");
    sf::Image i;
    m_i->_init(t,this,n,i,format,false);
}
Texture::Texture(sf::Image& img,string n,GLuint t,bool genMipMaps,uint format):m_i(new impl){ //pixels
    m_i->m_Files.push_back("PIXELS");
    m_i->_init(t,this,n,img,format,genMipMaps);
}
Texture::Texture(string file,string n,GLuint t,bool genMipMaps,uint format):m_i(new impl){ //image file
    m_i->m_Files.push_back(file);
    sf::Image i;
    if(n == "") n = file;
    m_i->_init(t,this,n,i,format,genMipMaps);
}
Texture::Texture(string files[],string n,GLuint t,bool genMipMaps,uint format):m_i(new impl){ //cubemap images
    for(uint q = 0; q < 6; q++){ 
        m_i->m_Files.push_back(files[q]); 
    }
    sf::Image i;
    m_i->_init(t,this,n,i,format,genMipMaps);
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
    glBindTexture(m_i->m_Type, m_i->m_TextureAddress.at(0));
    glTexImage2D(m_i->m_Type, 0, intern, (GLsizei)(w*s), (GLsizei)(h*s), 0, format, type, NULL);
    this->setFilter(TextureFilter::Linear);
    this->setWrapping(TextureWrap::ClampToEdge);
    glBindTexture(m_i->m_Type, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attatchment, m_i->m_Type, m_i->m_TextureAddress.at(0), 0);
}
void Texture::setXWrapping(TextureWrap::Wrap w){ GLuint gl; m_i->_enumWrapToGL(gl,w); glTexParameteri(m_i->m_Type, GL_TEXTURE_WRAP_S, gl); }
void Texture::setYWrapping(TextureWrap::Wrap w){ GLuint gl; m_i->_enumWrapToGL(gl,w); glTexParameteri(m_i->m_Type, GL_TEXTURE_WRAP_T, gl); }
void Texture::setZWrapping(TextureWrap::Wrap w){
    if(m_i->m_Type != GL_TEXTURE_CUBE_MAP){ return; } //this is not a cubemap
    GLuint gl; m_i->_enumWrapToGL(gl,w); glTexParameteri(m_i->m_Type, GL_TEXTURE_WRAP_R, gl);
}
void Texture::setWrapping(TextureWrap::Wrap w){ Texture::setXWrapping(w); Texture::setYWrapping(w); Texture::setZWrapping(w); }
void Texture::setMinFilter(TextureFilter::Filter f){ 
    GLuint g; m_i->_enumFilterToGL(g,f,true); glTexParameteri(m_i->m_Type,GL_TEXTURE_MIN_FILTER,g); m_i->m_MinFilter = g;
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
void Texture::genPBREnvMapData(){
    if(m_i->m_Type != GL_TEXTURE_CUBE_MAP){
        cout << "(Texture) : Only cubemaps can be convoluted. Ignoring convolute() call..." << endl;
        return;
    }
    uint& prevReadBuffer = Renderer::Detail::RendererInfo::GeneralInfo::current_bound_read_fbo;
    uint& prevDrawBuffer = Renderer::Detail::RendererInfo::GeneralInfo::current_bound_draw_fbo;
    //or just use plain 32...
    uint width = 32; /*glm::max(32, m_Width / 32);*/
    uint height = 32; /*glm::max(32, m_Height / 32);*/
    //cleanup previous convolute operation
    if(m_i->m_TextureAddress.size() >= 2){
        glDeleteTextures(1,&m_i->m_TextureAddress.at(1));
        glBindTexture(m_i->m_Type,0);
    }
    else if(m_i->m_TextureAddress.size() == 1){
        m_i->m_TextureAddress.push_back(0); // this should be element 2 (.at(1)) now
    }
    Renderer::bindFBO(0);

    GLuint captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);
    
    Renderer::bindFBO(captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);  
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO); 
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        return;
    }

    glGenTextures(1, &m_i->m_TextureAddress.at(1));
    glBindTexture(m_i->m_Type, m_i->m_TextureAddress.at(1));
    for (uint i = 0; i < 6; ++i){
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(m_i->m_Type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(m_i->m_Type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(m_i->m_Type, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(m_i->m_Type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(m_i->m_Type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, 1000.0f);
    glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };
    ShaderP* p = Resources::getShaderProgram("Cubemap_Convolude"); p->bind();
	Renderer::bindTexture("cubemap",address(),0,m_i->m_Type);
    
    glViewport(0, 0, width, height); // don't forget to configure the viewport to the capture dimensions.
    for (uint i = 0; i < 6; ++i){
        glm::mat4 vp = captureProjection * captureViews[i];
        Renderer::sendUniformMatrix4f("VP", vp);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_i->m_TextureAddress.at(1), 0);
        Renderer::Settings::clear(true,true,false);
		Skybox::bindMesh();
    }
	Renderer::bindFBO(0);
    p->unbind();
    

	//now gen EnvPrefilterMap for specular IBL
    //cleanup previous EnvPrefilterMap operation
    if(m_i->m_TextureAddress.size() >= 3){
        glDeleteTextures(1,&m_i->m_TextureAddress.at(2));
        glBindTexture(m_i->m_Type,0);
    }
    else if(m_i->m_TextureAddress.size() == 2){
        m_i->m_TextureAddress.push_back(0); // this should be element 3 (.at(2)) now
    }
    width = 128;
    height = 128;
    glGenTextures(1, &m_i->m_TextureAddress.at(2));
    glBindTexture(m_i->m_Type, m_i->m_TextureAddress.at(2));
    for (uint i = 0; i < 6; ++i){
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(m_i->m_Type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(m_i->m_Type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(m_i->m_Type, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(m_i->m_Type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(m_i->m_Type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(m_i->m_Type);

	p = Resources::getShaderProgram("Cubemap_Prefilter_Env"); p->bind();
	Renderer::bindTexture("cubemap",address(),0,m_i->m_Type);

    Renderer::bindFBO(captureFBO);
    uint maxMipLevels = 10;
    for (uint mip = 0; mip < maxMipLevels; ++mip){
        uint mipWidth  = width * glm::pow(0.5, mip); // reisze framebuffer according to mip-level size.
        uint mipHeight = height * glm::pow(0.5, mip);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);
        float roughness = (float)mip / (float)(maxMipLevels - 1);
		Renderer::sendUniform1f("roughness",roughness);
        for (uint i = 0; i < 6; ++i){
			glm::mat4 vp = captureProjection * captureViews[i];
			Renderer::sendUniformMatrix4f("VP", vp);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_i->m_TextureAddress.at(2), mip);
            Renderer::Settings::clear(true,true,false);
            Skybox::bindMesh();
        }
    }
    Renderer::bindFBO(0);


	//now generate the BDRF LUT -- should probably just make this a global variable
	//
    //cleanup previous BDRF LUT operation
	width = 512; //might have to be 512
	height = 512; //might have to be 512
    if(m_i->m_TextureAddress.size() >= 4){
        glDeleteTextures(1,&m_i->m_TextureAddress.at(3));
        glBindTexture(m_i->m_Type,0);
    }
    else if(m_i->m_TextureAddress.size() == 3){
        m_i->m_TextureAddress.push_back(0); // this should be element 4 (.at(3)) now
    }
	glGenTextures(1, &m_i->m_TextureAddress.at(3));
    glBindTexture(GL_TEXTURE_2D, m_i->m_TextureAddress.at(3));
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height, 0, GL_RG, GL_FLOAT, 0);
    // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
    Renderer::bindFBO(captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_i->m_TextureAddress.at(3), 0);

    glViewport(0, 0, width, height);
	p = Resources::getShaderProgram("BRDF_Precompute"); p->bind();
    Renderer::Settings::clear(true,true,false);
	glColorMask(GL_TRUE,GL_TRUE,GL_FALSE,GL_FALSE);
	Renderer::Detail::renderFullscreenQuad(width,height); //this might have to be winsize x and winsize y

	p->unbind();
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    Renderer::bindFBO(0);

    //cleanup... might have to comment this out if this bugs it out
    glDeleteRenderbuffers(1, &captureRBO);
    glDeleteFramebuffers(1, &captureFBO);
    ////

    Renderer::bindReadFBO(prevReadBuffer);
    Renderer::bindDrawFBO(prevDrawBuffer);
    glViewport(0,0,Resources::getWindowSize().x,Resources::getWindowSize().y);
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
