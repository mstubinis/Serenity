#include "Engine.h"
#include "Texture.h"
#include "Mesh.h"
#include "Skybox.h"
#include "Engine_Window.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "ShaderProgram.h"
#include "FramebufferObject.h"
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
	friend class Engine::epriv::TextureLoader;
	friend class ::Texture;
    public:
        vector<uchar> m_Pixels;
        vector<string> m_Files; //if non cubemap, this is only 1 file
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

        void _init(uint w,uint h,float divisor,ImagePixelType::Type pxlType,GLuint type,Texture* super,string n,const sf::Image& i,ImageInternalFormat::Format internFormat,ImagePixelFormat::Format pxlFormat,bool genMipMaps){
            m_PixelFormat = pxlFormat;
            m_PixelType = pxlType;
            _baseInit(type,super,n,i,internFormat,genMipMaps);
            m_Width = uint(float(w) * divisor);
            m_Height = uint(float(h) * divisor);
            super->load();
        }
        void _init(GLuint type,Texture* super,string name,const sf::Image& i,ImageInternalFormat::Format format,bool genMipMaps){
			m_PixelType = ImagePixelType::UNSIGNED_BYTE;
            _baseInit(type,super,name,i,format,genMipMaps);
            m_Width = i.getSize().x;
            m_Height = i.getSize().y;
            super->load();
        }
        void _baseInit(GLuint type,Texture* super,string n,const sf::Image& i,ImageInternalFormat::Format internFormat,bool genMipMaps){
            vector_clear(m_Pixels);
            m_Mipmapped = false;
            m_IsToBeMipmapped = genMipMaps;
            m_MinFilter = GL_LINEAR;
            m_MipMapLevels = 0;
            m_Type = type;
            m_InternalFormat = internFormat;
            if(i.getSize().x > 0 && i.getSize().y > 0){
                const sf::Uint8* pxls = i.getPixelsPtr();
                vector<uchar> p(pxls,pxls+(i.getSize().x*i.getSize().y*4));
                m_Pixels = p;
            }
            super->setName(n);
        }
        void _load(Texture* super){
            if(m_TextureAddress.size() == 0)
                m_TextureAddress.push_back(0); //vector.at(0) will be the default address. at(1) is the colvoluted map address (for cubemap only)
            glGenTextures(1, &m_TextureAddress.at(0));
            glBindTexture(m_Type, m_TextureAddress.at(0));
            if(m_Files.size() == 1 && m_Files.at(0) != "FRAMEBUFFER" && m_Files.at(0) != "PIXELS"){//single file, NOT a framebuffer or pixel data texture
                sf::Image i;
				i.loadFromFile(m_Files.at(0).c_str());
                const sf::Uint8* pxls = i.getPixelsPtr();
                m_Width = i.getSize().x;
                m_Height = i.getSize().y;
                vector<uchar> p(pxls,pxls+(i.getSize().x*i.getSize().y*4));
                m_Pixels = p;
				epriv::TextureLoader::LoadTexture2DIntoOpenGL(super);
            }
            else if(m_Files.size() == 1 && m_Files.at(0) == "PIXELS"){//pixel data image
				epriv::TextureLoader::LoadTexture2DIntoOpenGL(super);
            }
            else if(m_Files.size() == 1 && m_Files.at(0) == "FRAMEBUFFER"){//Framebuffer
				epriv::TextureLoader::LoadTextureFramebufferIntoOpenGL(super);
            }
            else if(m_Files.size() > 1){//cubemap
				epriv::TextureLoader::LoadTextureCubemapIntoOpenGL(super);
            }
            else{//no files
            }
            if(m_IsToBeMipmapped){
                epriv::TextureLoader::GenerateMipmapsOpenGL(super);
            }
        }
        void _unload(){
            for(uint i = 0; i < m_TextureAddress.size(); ++i){
                glDeleteTextures(1,&m_TextureAddress.at(i));
            }
            glBindTexture(m_Type,0);
            m_MipMapLevels = 0;
            m_Mipmapped = false;
            vector_clear(m_Pixels);
            vector_clear(m_TextureAddress);
        }
        void _resize(epriv::FramebufferTexture* t,uint w, uint h){
            if(m_Files.size() == 0 || (m_Files.size() == 1 && m_Files.at(0) != "FRAMEBUFFER")){
                cout << "Error: Non-framebuffer texture cannot be resized. Returning..." << endl;
                return;
            }
            glBindTexture(m_Type, m_TextureAddress.at(0));
            m_Width = uint(float(w) * t->divisor()); 
            m_Height = uint(float(h) * t->divisor());
            glTexImage2D(m_Type,0,ImageInternalFormat::at(m_InternalFormat),m_Width,m_Height,0,ImagePixelFormat::at(m_PixelFormat),ImagePixelType::at(m_PixelType),NULL);
        }
};


void epriv::TextureLoader::LoadTexture2DIntoOpenGL(Texture* _texture){
	Texture::impl& i = *_texture->m_i;
    sf::Image img;
    img.create(i.m_Width,i.m_Height,&i.m_Pixels[0]);
    TextureLoader::ChoosePixelFormat(i.m_PixelFormat,i.m_InternalFormat);
	glTexImage2D(i.m_Type,0,ImageInternalFormat::at(i.m_InternalFormat),i.m_Width,i.m_Height,0,ImagePixelFormat::at(i.m_PixelFormat),ImagePixelType::at(i.m_PixelType),img.getPixelsPtr());
	_texture->setFilter(TextureFilter::Linear);
    glBindTexture(i.m_Type,0);
    epriv::TextureLoader::WithdrawPixelsFromOpenGLMemory(_texture);
}
void epriv::TextureLoader::LoadTextureFramebufferIntoOpenGL(Texture* _texture){
	Texture::impl& i = *_texture->m_i;
    glBindTexture(i.m_Type,i.m_TextureAddress.at(0));
	glTexImage2D(i.m_Type,0,ImageInternalFormat::at(i.m_InternalFormat),i.m_Width,i.m_Height,0,ImagePixelFormat::at(i.m_PixelFormat),ImagePixelType::at(i.m_PixelType),NULL);
    _texture->setFilter(TextureFilter::Linear);
    _texture->setWrapping(TextureWrap::ClampToEdge);
    glBindTexture(i.m_Type,0);
}
void epriv::TextureLoader::LoadTextureCubemapIntoOpenGL(Texture* _texture){
	Texture::impl& i = *_texture->m_i;
	for(uint k = 0; k < i.m_Files.size(); ++k){
        sf::Image img;
		img.loadFromFile(i.m_Files[k].c_str());
        i.m_Width = img.getSize().x;
        i.m_Height = img.getSize().y;
		i.m_Type = GL_TEXTURE_CUBE_MAP_POSITIVE_X + k;
        TextureLoader::ChoosePixelFormat(i.m_PixelFormat,i.m_InternalFormat);
		glTexImage2D(i.m_Type,0,ImageInternalFormat::at(i.m_InternalFormat),i.m_Width,i.m_Height,0,ImagePixelFormat::at(i.m_PixelFormat),GL_UNSIGNED_BYTE,img.getPixelsPtr());
	}
	i.m_Type = GL_TEXTURE_CUBE_MAP;
	_texture->setFilter(TextureFilter::Linear);
    _texture->setWrapping(TextureWrap::ClampToEdge);
    glBindTexture(i.m_Type,0);
	epriv::TextureLoader::WithdrawPixelsFromOpenGLMemory(_texture);
}
void epriv::TextureLoader::WithdrawPixelsFromOpenGLMemory(Texture* _texture){
	Texture::impl& i = *_texture->m_i;
    if(i.m_Pixels.size() == 0){
        i.m_Pixels.resize(i.m_Width * i.m_Height * 4);
        glBindTexture(i.m_Type,i.m_TextureAddress.at(0));
		glGetTexImage(i.m_Type,0,ImagePixelFormat::at(i.m_PixelFormat),ImagePixelType::at(i.m_PixelType),&i.m_Pixels[0]);
        glBindTexture(i.m_Type,0);
    }
}
void epriv::TextureLoader::ChoosePixelFormat(ImagePixelFormat::Format& _out,ImageInternalFormat::Format& _in){
	switch(_in){
		case ImageInternalFormat::COMPRESSED_RED:{ _out = ImagePixelFormat::RED;break; }
		case ImageInternalFormat::COMPRESSED_RED_RGTC1:{ _out = ImagePixelFormat::RED;break; }//recheck this
		case ImageInternalFormat::COMPRESSED_RG:{ _out = ImagePixelFormat::RG;break; }
		case ImageInternalFormat::COMPRESSED_RGB:{ _out = ImagePixelFormat::RGB;break; }
		case ImageInternalFormat::COMPRESSED_RGBA:{ _out = ImagePixelFormat::RGBA;break; }
		case ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT1_EXT:{ _out = ImagePixelFormat::RGBA;break; }
		case ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT3_EXT:{ _out = ImagePixelFormat::RGBA;break; }
		case ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT:{ _out = ImagePixelFormat::RGBA;break; }
		case ImageInternalFormat::COMPRESSED_RGB_S3TC_DXT1_EXT:{ _out = ImagePixelFormat::RGB;break; }
		case ImageInternalFormat::COMPRESSED_RG_RGTC2:{ _out = ImagePixelFormat::RG;break; }//recheck this
		case ImageInternalFormat::COMPRESSED_SIGNED_RED_RGTC1:{ _out = ImagePixelFormat::RED;break; }//recheck this
		case ImageInternalFormat::COMPRESSED_SIGNED_RG_RGTC2:{ _out = ImagePixelFormat::RG;break; }//recheck this
		case ImageInternalFormat::COMPRESSED_SRGB:{ _out = ImagePixelFormat::RGB;break; }
		case ImageInternalFormat::COMPRESSED_SRGB_ALPHA:{ _out = ImagePixelFormat::RGBA;break; }
		case ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:{ _out = ImagePixelFormat::RGBA;break; }
		case ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:{ _out = ImagePixelFormat::RGBA;break; }
		case ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:{ _out = ImagePixelFormat::RGBA;break; }
		case ImageInternalFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT:{ _out = ImagePixelFormat::RGB;break; }
		case ImageInternalFormat::Depth16:{ _out = ImagePixelFormat::DEPTH_COMPONENT;break; }
		case ImageInternalFormat::Depth24:{ _out = ImagePixelFormat::DEPTH_COMPONENT;break; }
		case ImageInternalFormat::Depth32:{ _out = ImagePixelFormat::DEPTH_COMPONENT;break; }
		case ImageInternalFormat::Depth32F:{ _out = ImagePixelFormat::DEPTH_COMPONENT;break; }
		case ImageInternalFormat::Depth24Stencil8:{ _out = ImagePixelFormat::DEPTH_STENCIL;break; }
		case ImageInternalFormat::Depth32FStencil8:{ _out = ImagePixelFormat::DEPTH_STENCIL;break; }
		case ImageInternalFormat::DEPTH_COMPONENT:{ _out = ImagePixelFormat::DEPTH_COMPONENT;break; }
		case ImageInternalFormat::DEPTH_STENCIL:{ _out = ImagePixelFormat::DEPTH_STENCIL;break; }
		case ImageInternalFormat::R11F_G11F_B10F:{ _out = ImagePixelFormat::RGB;break; }
		case ImageInternalFormat::R16:{ _out = ImagePixelFormat::RED;break; }
		case ImageInternalFormat::R16F:{ _out = ImagePixelFormat::RED;break; }
		case ImageInternalFormat::R16I:{ _out = ImagePixelFormat::RED_INTEGER;break; }
		case ImageInternalFormat::R16UI:{ _out = ImagePixelFormat::RED_INTEGER;break; }
		case ImageInternalFormat::R16_SNORM:{ _out = ImagePixelFormat::RED;break; }
		case ImageInternalFormat::R32F:{ _out = ImagePixelFormat::RED;break; }
		case ImageInternalFormat::R32I:{ _out = ImagePixelFormat::RED_INTEGER;break; }
		case ImageInternalFormat::R32UI:{ _out = ImagePixelFormat::RED_INTEGER;break; }
		case ImageInternalFormat::R3_G3_B2:{ _out = ImagePixelFormat::RGB;break; }
		case ImageInternalFormat::R8:{ _out = ImagePixelFormat::RED;break; }
		case ImageInternalFormat::R8I:{ _out = ImagePixelFormat::RED_INTEGER;break; }
		case ImageInternalFormat::R8UI:{ _out = ImagePixelFormat::RED_INTEGER;break; }
		case ImageInternalFormat::R8_SNORM:{ _out = ImagePixelFormat::RED;break; }
		case ImageInternalFormat::RED:{ _out = ImagePixelFormat::RED;break; }
		case ImageInternalFormat::RG:{ _out = ImagePixelFormat::RG;break; }
		case ImageInternalFormat::RG16:{ _out = ImagePixelFormat::RG;break; }
		case ImageInternalFormat::RG16F:{ _out = ImagePixelFormat::RG;break; }
		case ImageInternalFormat::RG16I:{ _out = ImagePixelFormat::RG_INTEGER;break; }
		case ImageInternalFormat::RG16UI:{ _out = ImagePixelFormat::RG_INTEGER;break; }
		case ImageInternalFormat::RG16_SNORM:{ _out = ImagePixelFormat::RG;break; }
		case ImageInternalFormat::RG32F:{ _out = ImagePixelFormat::RG;break; }
		case ImageInternalFormat::RG32I:{ _out = ImagePixelFormat::RG_INTEGER;break; }
		case ImageInternalFormat::RG32UI:{ _out = ImagePixelFormat::RG_INTEGER;break; }
		case ImageInternalFormat::RG8:{ _out = ImagePixelFormat::RG;break; }
		case ImageInternalFormat::RG8I:{ _out = ImagePixelFormat::RG_INTEGER;break; }
		case ImageInternalFormat::RG8UI:{ _out = ImagePixelFormat::RG_INTEGER;break; }
		case ImageInternalFormat::RG8_SNORM:{ _out = ImagePixelFormat::RG;break; }
		case ImageInternalFormat::RGB:{ _out = ImagePixelFormat::RGB;break; }
		case ImageInternalFormat::RGB10:{ _out = ImagePixelFormat::RGB;break; }
		case ImageInternalFormat::RGB10_A2:{ _out = ImagePixelFormat::RGBA;break; }
		case ImageInternalFormat::RGB12:{ _out = ImagePixelFormat::RGB;break; }
		case ImageInternalFormat::RGB16F:{ _out = ImagePixelFormat::RGB;break; }
		case ImageInternalFormat::RGB16I:{ _out = ImagePixelFormat::RGB_INTEGER;break; }
		case ImageInternalFormat::RGB16UI:{ _out = ImagePixelFormat::RGB_INTEGER;break; }
		case ImageInternalFormat::RGB16_SNORM:{ _out = ImagePixelFormat::RGB;break; }
		case ImageInternalFormat::RGB32F:{ _out = ImagePixelFormat::RGB;break; }
		case ImageInternalFormat::RGB32I:{ _out = ImagePixelFormat::RGB_INTEGER;break; }
		case ImageInternalFormat::RGB32UI:{ _out = ImagePixelFormat::RGB_INTEGER;break; }
		case ImageInternalFormat::RGB4:{ _out = ImagePixelFormat::RGB;break; }
		case ImageInternalFormat::RGB5:{ _out = ImagePixelFormat::RGB;break; }
		case ImageInternalFormat::RGB5_A1:{ _out = ImagePixelFormat::RGBA;break; }
		case ImageInternalFormat::RGB8:{ _out = ImagePixelFormat::RGB;break; }
		case ImageInternalFormat::RGB8I:{ _out = ImagePixelFormat::RGB_INTEGER;break; }
		case ImageInternalFormat::RGB8UI:{ _out = ImagePixelFormat::RGB_INTEGER;break; }
		case ImageInternalFormat::RGB8_SNORM:{ _out = ImagePixelFormat::RGB;break; }
		case ImageInternalFormat::RGB9_E5:{ _out = ImagePixelFormat::RGB;break; }//recheck this
		case ImageInternalFormat::RGBA:{ _out = ImagePixelFormat::RGBA;break; }
		case ImageInternalFormat::RGBA12:{ _out = ImagePixelFormat::RGBA;break; }
		case ImageInternalFormat::RGBA16:{ _out = ImagePixelFormat::RGBA;break; }
		case ImageInternalFormat::RGBA16F:{ _out = ImagePixelFormat::RGBA;break; }
		case ImageInternalFormat::RGBA16I:{ _out = ImagePixelFormat::RGBA_INTEGER;break; }
		case ImageInternalFormat::RGBA16UI:{ _out = ImagePixelFormat::RGBA_INTEGER;break; }
		case ImageInternalFormat::RGBA2:{ _out = ImagePixelFormat::RGBA;break; }
		case ImageInternalFormat::RGBA32F:{ _out = ImagePixelFormat::RGBA;break; }
		case ImageInternalFormat::RGBA32I:{ _out = ImagePixelFormat::RGBA_INTEGER;break; }
		case ImageInternalFormat::RGBA32UI:{ _out = ImagePixelFormat::RGBA_INTEGER;break; }
		case ImageInternalFormat::RGBA4:{ _out = ImagePixelFormat::RGBA;break; }
		case ImageInternalFormat::RGBA8:{ _out = ImagePixelFormat::RGBA;break; }
		case ImageInternalFormat::RGBA8I:{ _out = ImagePixelFormat::RGBA_INTEGER;break; }
		case ImageInternalFormat::RGBA8UI:{ _out = ImagePixelFormat::RGBA_INTEGER;break; }
		case ImageInternalFormat::RGBA8_SNORM:{ _out = ImagePixelFormat::RGBA;break; }
		case ImageInternalFormat::SRGB8:{ _out = ImagePixelFormat::RGB;break; }
		case ImageInternalFormat::SRGB8_ALPHA8:{ _out = ImagePixelFormat::RGBA;break; }
		case ImageInternalFormat::StencilIndex8:{ _out = ImagePixelFormat::STENCIL_INDEX;break; }
		case ImageInternalFormat::STENCIL_INDEX:{ _out = ImagePixelFormat::STENCIL_INDEX;break; }
		default:{ _out = ImagePixelFormat::RGBA;break; }
	}
}
void epriv::TextureLoader::GenerateMipmapsOpenGL(Texture* _texture){
	Texture::impl& i = *_texture->m_i; if(i.m_Mipmapped) return;

    glBindTexture(i.m_Type, i.m_TextureAddress.at(0));
    glTexParameteri(i.m_Type, GL_TEXTURE_BASE_LEVEL, 0);
    if(i.m_MinFilter == GL_LINEAR){        
		i.m_MinFilter = GL_LINEAR_MIPMAP_LINEAR; 
	}
    else if(i.m_MinFilter == GL_NEAREST){  
		i.m_MinFilter = GL_NEAREST_MIPMAP_NEAREST; 
	}
    glTexParameteri(i.m_Type, GL_TEXTURE_MIN_FILTER, i.m_MinFilter);
    glGenerateMipmap(i.m_Type);
    i.m_Mipmapped = true;
    i.m_MipMapLevels = uint(glm::log2(glm::max(i.m_Width,i.m_Height)) + 1.0f);
    glBindTexture(i.m_Type, 0);
}
void epriv::TextureLoader::EnumWrapToGL(uint& gl, TextureWrap::Wrap& wrap){
    if(wrap == TextureWrap::Repeat)              gl = GL_REPEAT;
    else if(wrap == TextureWrap::RepeatMirrored) gl = GL_MIRRORED_REPEAT;
    else if(wrap == TextureWrap::ClampToBorder)  gl = GL_CLAMP_TO_BORDER;
    else if(wrap == TextureWrap::ClampToEdge)    gl = GL_CLAMP_TO_EDGE;
}
void epriv::TextureLoader::EnumFilterToGL(uint& gl, TextureFilter::Filter& filter,bool min){
    if(min){
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

Texture::Texture(string n,uint w, uint h,ImageInternalFormat::Format internal,ImagePixelFormat::Format pxlFormat,ImagePixelType::Type pxlType,GLuint t,float divisor):m_i(new impl){ //framebuffer
    m_i->m_Files.push_back("FRAMEBUFFER");
    const sf::Image i;
    m_i->_init(w,h,divisor,pxlType,t,this,n,i,internal,pxlFormat,false);
}
Texture::Texture(const sf::Image& i,string n,GLuint t,bool genMipMaps,ImageInternalFormat::Format internalFormat):m_i(new impl){ //pixels
    m_i->m_Files.push_back("PIXELS");
    m_i->_init(t,this,n,i,internalFormat,genMipMaps);
}
Texture::Texture(string file,GLuint t,bool genMipMaps,ImageInternalFormat::Format internalFormat):m_i(new impl){ //image file
    m_i->m_Files.push_back(file);
    const sf::Image i;
    m_i->_init(t,this,file,i,internalFormat,genMipMaps);
}
Texture::Texture(string files[],string n,GLuint t,bool genMipMaps,ImageInternalFormat::Format internalFormat):m_i(new impl){ //cubemap images
    for(uint j = 0; j < 6; ++j){ 
        m_i->m_Files.push_back(files[j]); 
    }
    const sf::Image i;
    m_i->_init(t,this,n,i,internalFormat,genMipMaps);
}
Texture::~Texture(){
    unload();
}
void Texture::render(glm::vec2& pos, glm::vec4& color,float angle, glm::vec2& scl, float depth){
    if(m_i->m_Files.size() != 1) return;
    epriv::Core::m_Engine->m_RenderManager->_renderTexture(this,pos,color,scl,angle,depth);
}
void Texture::setXWrapping(TextureWrap::Wrap w){ Texture::setXWrapping(m_i->m_Type,w); }
void Texture::setYWrapping(TextureWrap::Wrap w){ Texture::setYWrapping(m_i->m_Type,w); }
void Texture::setZWrapping(TextureWrap::Wrap w){ Texture::setZWrapping(m_i->m_Type,w); }
void Texture::setWrapping(TextureWrap::Wrap w){ Texture::setWrapping(m_i->m_Type,w); }
void Texture::setMinFilter(TextureFilter::Filter f){ Texture::setMinFilter(m_i->m_Type,f); m_i->m_MinFilter = f; }
void Texture::setMaxFilter(TextureFilter::Filter f){ Texture::setMaxFilter(m_i->m_Type,f); }
void Texture::setFilter(TextureFilter::Filter f){ Texture::setFilter(m_i->m_Type,f); }
void Texture::setXWrapping(GLuint type,TextureWrap::Wrap w){ 
    GLuint gl;
	epriv::TextureLoader::EnumWrapToGL(gl,w);
    glTexParameteri(type, GL_TEXTURE_WRAP_S, gl);
}
void Texture::setYWrapping(GLuint type,TextureWrap::Wrap w){ 
    GLuint gl; 
    epriv::TextureLoader::EnumWrapToGL(gl,w);
    glTexParameteri(type, GL_TEXTURE_WRAP_T, gl);
}
void Texture::setZWrapping(GLuint type,TextureWrap::Wrap w){
    if(type != GL_TEXTURE_CUBE_MAP) return;
    GLuint gl;
    epriv::TextureLoader::EnumWrapToGL(gl,w);
    glTexParameteri(type,GL_TEXTURE_WRAP_R,gl);
}
void Texture::setWrapping(GLuint type,TextureWrap::Wrap w){ 
    Texture::setXWrapping(type,w);
    Texture::setYWrapping(type,w);
    Texture::setZWrapping(type,w);
}
void Texture::setMinFilter(GLuint type,TextureFilter::Filter filter){ 
    GLuint gl;
    epriv::TextureLoader::EnumFilterToGL(gl,filter,true);
    glTexParameteri(type,GL_TEXTURE_MIN_FILTER,gl); 
}
void Texture::setMaxFilter(GLuint type,TextureFilter::Filter filter){ 
    GLuint gl;
    epriv::TextureLoader::EnumFilterToGL(gl,filter,false);
    glTexParameteri(type,GL_TEXTURE_MAG_FILTER,gl);  
}
void Texture::setFilter(GLuint type,TextureFilter::Filter f){
    Texture::setMinFilter(type,f);
    Texture::setMaxFilter(type,f);
}
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
    if(m_i->m_TextureAddress.size() == 1){
        m_i->m_TextureAddress.push_back(0);
        glGenTextures(1, &m_i->m_TextureAddress.at(1));
        glBindTexture(m_i->m_Type, m_i->m_TextureAddress.at(1));
        for (uint i = 0; i < 6; ++i){
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,0,GL_RGB16F,convoludeTextureSize,convoludeTextureSize,0,GL_RGB,GL_FLOAT,NULL);
        }
        setWrapping(TextureWrap::ClampToEdge);
        setFilter(TextureFilter::Linear);
    }
    if(m_i->m_TextureAddress.size() == 2){
        glBindTexture(m_i->m_Type,0);
        m_i->m_TextureAddress.push_back(0);
        glGenTextures(1, &m_i->m_TextureAddress.at(2));
        glBindTexture(m_i->m_Type, m_i->m_TextureAddress.at(2));
        for (uint i = 0; i < 6; ++i){
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,0,GL_RGB16F,preEnvFilterSize,preEnvFilterSize,0,GL_RGB,GL_FLOAT,NULL);
        }
        setWrapping(TextureWrap::ClampToEdge);
        setMinFilter(TextureFilter::Linear_Mipmap_Linear);
        setMaxFilter(TextureFilter::Linear);
        glGenerateMipmap(m_i->m_Type);
    }
    epriv::Core::m_Engine->m_RenderManager->_genPBREnvMapData(this,convoludeTextureSize,preEnvFilterSize);
}
void Texture::resize(epriv::FramebufferTexture* t,uint w, uint h){ m_i->_resize(t,w,h); }
bool Texture::mipmapped(){ return m_i->m_Mipmapped; }
ushort Texture::mipmapLevels(){ return m_i->m_MipMapLevels; }
uchar* Texture::pixels(){ epriv::TextureLoader::WithdrawPixelsFromOpenGLMemory(this); return &m_i->m_Pixels[0]; }
GLuint& Texture::address(){ return m_i->m_TextureAddress.at(0); }
GLuint& Texture::address(uint index){ return m_i->m_TextureAddress.at(index); }
uint Texture::numAddresses(){ return m_i->m_TextureAddress.size(); }
GLuint Texture::type(){ return m_i->m_Type; }
uint Texture::width(){ return m_i->m_Width; }
uint Texture::height(){ return m_i->m_Height; }
ImageInternalFormat::Format Texture::internalFormat(){ return m_i->m_InternalFormat; }
ImagePixelFormat::Format Texture::pixelFormat(){ return m_i->m_PixelFormat; }
ImagePixelType::Type Texture::pixelType(){ return m_i->m_PixelType; }
