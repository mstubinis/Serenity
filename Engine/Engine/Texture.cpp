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
#include <boost/filesystem.hpp>
#include <iostream>

using namespace Engine;
using namespace std;

#define FOURCC(a,b,c,d) ( (uint32) (((d)<<24) | ((c)<<16) | ((b)<<8) | (a)) )
string fourcc(uint32_t e){ char c[5]={'\0'};c[0]=e>>0 & 0xFF;c[1]=e>>8 & 0xFF;c[2]=e>>16 & 0xFF;c[3]=e>>24 & 0xFF;return c; }

struct epriv::ImageLoadedStructure final{
    uint                          width;
    uint                          height;
    uint                          compressedSize;
    uint                          pixelBufferSize;
    uchar*                        pixels;
    ImageInternalFormat::Format   internalFormat;
    ImagePixelFormat::Format      pixelFormat;
    ImagePixelType::Type          pixelType;
    ImageLoadedStructure(){
        width = 0; height = 0; pixels = 0; compressedSize = 0; pixelBufferSize = 0;
    }
    ~ImageLoadedStructure(){}
    ImageLoadedStructure(const sf::Image& i){
        width = i.getSize().x;
        height = i.getSize().y;
        pixels = const_cast<uchar*>(i.getPixelsPtr());
        compressedSize = 0;
        pixelBufferSize = width * height * 4;
    }
};


class Texture::impl final{
    friend class Engine::epriv::TextureLoader;
    friend class ::Texture;
    public:
        vector<uchar> m_Pixels;
        vector<string> m_Files; //if non cubemap, this is only 1 file
        vector<GLuint> m_TextureAddress;
        GLuint m_Type;
        uint m_CompressedSize;
        uint m_Width, m_Height;
        ImageInternalFormat::Format m_InternalFormat;
        ImagePixelFormat::Format m_PixelFormat;
        ImagePixelType::Type m_PixelType;
        ushort m_MipMapLevels;
        bool m_Mipmapped;
        bool m_IsToBeMipmapped;
        GLuint m_MinFilter; //used to determine filter type for mipmaps

        void _initFramebuffer(uint _w,uint _h,ImagePixelType::Type _pixelType,ImagePixelFormat::Format _pixelFormat,ImageInternalFormat::Format _internalFormat,float _divisor,Texture* _super){
            epriv::ImageLoadedStructure image;
            m_PixelFormat = _pixelFormat;
            m_PixelType = _pixelType;
            image.width = uint(float(_w) * _divisor);
            image.height = uint(float(_h) * _divisor);
            _baseInit(GL_TEXTURE_2D,"RenderTarget",image,_internalFormat,false,_super);
            _super->load();
        }
        void _initFromPixelsMemory(const sf::Image& _sfImage,string _name,GLuint _openglTextureType,bool _genMipMaps,ImageInternalFormat::Format _internalFormat,Texture* _super){
            epriv::ImageLoadedStructure image(_sfImage);
            m_PixelType = ImagePixelType::UNSIGNED_BYTE;
            _baseInit(_openglTextureType,_name,image,_internalFormat,false,_super);
            _super->load();
        }
        void _initFromImageFile(string _filename,GLuint _openglTextureType,bool _genMipMaps,ImageInternalFormat::Format _internalFormat,Texture* _super){	
            string extension = boost::filesystem::extension(_filename);
            if(extension != ".dds"){
                sf::Image _sfImage;
                _sfImage.loadFromFile(_filename);
                _initFromPixelsMemory(_sfImage,_filename,_openglTextureType,_genMipMaps,_internalFormat,_super);
            }
            else{
                vector<epriv::ImageLoadedStructure*> images;
                epriv::TextureLoader::LoadDDSFile(_super,_filename,images);
                m_PixelType = images.at(0)->pixelType;
                m_PixelFormat = images.at(0)->pixelFormat;
                _baseInit(_openglTextureType,_filename,*images.at(0),images.at(0)->internalFormat,false,_super);
                m_CompressedSize = images.at(0)->compressedSize;
                _super->load();

                //mipmaps
                if(images.size() > 1){
                    glBindTexture(m_Type, m_TextureAddress.at(0));
                    uint level = 0;		
                    for(auto image:images){
                        ++level;
                        glCompressedTexImage2D(GL_TEXTURE_2D,level,image->internalFormat,image->width,image->height,0,image->compressedSize,image->pixels);
                    }
					delete[] images.at(0)->pixels;
                    for(auto image:images){ 
						delete image;
                    }
                    glBindTexture(m_Type, 0);
                }
            }
        }
        void _initCubemap(string _name,ImageInternalFormat::Format _internalFormat,bool _genMipMaps,Texture* _super){
            epriv::ImageLoadedStructure image;
            m_PixelType = ImagePixelType::UNSIGNED_BYTE;
            _baseInit(GL_TEXTURE_CUBE_MAP,_name,image,_internalFormat,false,_super);
            _super->load();
        }
        void _baseInit(GLuint type,string n,epriv::ImageLoadedStructure& i,ImageInternalFormat::Format internFormat,bool genMipMaps,Texture* _super){
            vector_clear(m_Pixels);
            m_Mipmapped = false;
            m_IsToBeMipmapped = genMipMaps;
            m_MinFilter = GL_LINEAR;
            m_MipMapLevels = 0;
            m_CompressedSize = 0;
            m_Type = type;
            m_Width = i.width;
            m_Height = i.height;
            m_InternalFormat = internFormat;
            if(i.width > 0 && i.height > 0 && i.pixels){
                vector<uchar> p(i.pixels, i.pixels + i.pixelBufferSize);
                m_Pixels = p;
            }
            _super->setName(n);		
        }
        void _load(Texture* super){
            if(m_TextureAddress.size() == 0)
                m_TextureAddress.push_back(0); //vector.at(0) will be the default address. at(1) is the colvoluted map address (for cubemap only)
            glGenTextures(1, &m_TextureAddress.at(0));
            glBindTexture(m_Type, m_TextureAddress.at(0));
            
            if(m_Files.size() == 1 && m_Files.at(0) == "FB"){ epriv::TextureLoader::LoadTextureFramebufferIntoOpenGL(super); } //FRAMEBUFFER
            else if(m_Files.size() == 1 && m_Files.at(0) != "FB" && m_Files.at(0) != "PIXELS"){ epriv::TextureLoader::LoadTexture2DIntoOpenGL(super); } //IMAGE FILE
            else if(m_Files.size() == 1 && m_Files.at(0) == "PIXELS"){ epriv::TextureLoader::LoadTexture2DIntoOpenGL(super); } //PIXELS FROM MEMORY
            else if(m_Files.size() > 1){ epriv::TextureLoader::LoadTextureCubemapIntoOpenGL(super); } //CUBEMAP

            if(m_IsToBeMipmapped) epriv::TextureLoader::GenerateMipmapsOpenGL(super);
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
            if(m_Files.size() == 0 || (m_Files.size() == 1 && m_Files.at(0) != "FB")){
                cout << "Error: Non-framebuffer texture cannot be resized. Returning..." << endl;
                return;
            }
            glBindTexture(m_Type, m_TextureAddress.at(0));
            m_Width = uint(float(w) * t->divisor()); 
            m_Height = uint(float(h) * t->divisor());
            glTexImage2D(m_Type,0,ImageInternalFormat::at(m_InternalFormat),m_Width,m_Height,0,ImagePixelFormat::at(m_PixelFormat),ImagePixelType::at(m_PixelType),NULL);
        }
};

void epriv::TextureLoader::LoadDDSFile(Texture* _texture,string _filename,vector<epriv::ImageLoadedStructure*>& images){
    uchar header[124];
    epriv::ImageLoadedStructure* image = new epriv::ImageLoadedStructure();
    FILE* fileparser = fopen(_filename.c_str(), "rb");
    if (!fileparser) return;
    char filecode[4];
    fread(&filecode,1,4,fileparser);
	uint res = strncmp(filecode,"DDS ", 4);
	if (res != 0){
        fclose(fileparser);
        return;
    }
    fread(&header, 124, 1, fileparser);
    uint h = *(uint*)&(header[8 ]);
    uint w = *(uint*)&(header[12]);
    uint linearSize = *(uint*)&(header[16]);
    uint mipMapCount = *(uint*)&(header[24]);
    uint fourCC = *(uint*)&(header[80]);

    /* how big is it going to be including all mipmaps? */
    image->pixelBufferSize = mipMapCount > 1 ? linearSize * 2 : linearSize;
    image->pixels = new uchar[image->pixelBufferSize * sizeof(uchar)];
    fread(image->pixels, 1, image->pixelBufferSize, fileparser);
    fclose(fileparser);

	//std::cout << "FourCC: " << fourcc(fourCC) << std::endl;

    uint components = (fourCC == FOURCC('D','X','T','1')) ? 3 : 4;
    image->pixelFormat = ImagePixelFormat::RGBA;
    image->pixelType = ImagePixelType::UNSIGNED_BYTE;
    switch(fourCC){
        case FOURCC('D','X','T','1'):{ 
            image->internalFormat = ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
            break; 
        }
        case FOURCC('D','X','T','2'):{ 
            break; 
        }
        case FOURCC('D','X','T','3'):{ 
            image->internalFormat = ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
            break; 
        }
        case FOURCC('D','X','T','4'):{ 
            break; 
        }
        case FOURCC('D','X','T','5'):{
			//DXT5nm, A=X, G=Y, sometimes setting R & B to Y. Recalculate Z using z = sqrt( 1-x*x-y*y )
            image->internalFormat = ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
            break; 
        }
        case FOURCC('A','T','I','2'):{//aka ATI2n aka 3Dc aka LATC2 aka BC5
			image->internalFormat = ImageInternalFormat::COMPRESSED_RG_RGTC2;
            break; 
        }
        default:{ delete[] image->pixels; return; }
    }
    uint blockSize = (image->internalFormat == ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT ||
		image->internalFormat == ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
    uint offset = 0;
 
    //load single mipmap
    image->compressedSize = ((w + 3) / 4) * ((h + 3) / 4) * blockSize;
    image->width = w;
    image->height = h;
    offset += image->compressedSize;
    images.push_back(image);

    //load all mipmaps
	/*
	epriv::ImageLoadedStructure* imgIter = image;
    for (uint level = 0; level < mipMapCount && (w || h); ++level){
        if(level > 0)
            imgIter = images.at(level);
        if(w <= 2 || h <= 2) break;
        epriv::ImageLoadedStructure* img = new epriv::ImageLoadedStructure();
        img->internalFormat = image->internalFormat;
        img->pixelFormat = image->pixelFormat;
        img->pixelType = image->pixelType;
        img->width = w;
        img->height = h;
        img->compressedSize = ((w + 3) / 4) * ((h + 3) / 4) * blockSize;
        img->pixels = imgIter->pixels + offset;
        offset += img->compressedSize;
        w = Math::Max(uint(w / 2), 1);
        h = Math::Max(uint(h / 2), 1);
        images.push_back(img);
    }
	*/
}
void epriv::TextureLoader::LoadTexture2DIntoOpenGL(Texture* _texture){
    Texture::impl& i = *_texture->m_i;
    TextureLoader::ChoosePixelFormat(i.m_PixelFormat,i.m_InternalFormat);
    if(TextureLoader::IsCompressedType(i.m_InternalFormat) && i.m_CompressedSize != 0){
        glCompressedTexImage2D(i.m_Type,0,ImageInternalFormat::at(i.m_InternalFormat),i.m_Width,i.m_Height,0,i.m_CompressedSize,&i.m_Pixels[0]);
    }
    else{
        glTexImage2D(i.m_Type,0,ImageInternalFormat::at(i.m_InternalFormat),i.m_Width,i.m_Height,0,ImagePixelFormat::at(i.m_PixelFormat),ImagePixelType::at(i.m_PixelType),&i.m_Pixels[0]);
    }
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
bool epriv::TextureLoader::IsCompressedType(ImageInternalFormat::Format _format){
    switch(_format){
        case ImageInternalFormat::COMPRESSED_RED:{ return true; break; }
        case ImageInternalFormat::COMPRESSED_RED_RGTC1:{ return true; break; }
        case ImageInternalFormat::COMPRESSED_RG:{ return true; break; }
        case ImageInternalFormat::COMPRESSED_RGB:{ return true; break; }
        case ImageInternalFormat::COMPRESSED_RGBA:{ return true; break; }
        case ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT1_EXT:{ return true; break; }
        case ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT3_EXT:{ return true; break; }
        case ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT:{ return true; break; }
        case ImageInternalFormat::COMPRESSED_RGB_S3TC_DXT1_EXT:{ return true; break; }
        case ImageInternalFormat::COMPRESSED_RG_RGTC2:{ return true; break; }
        case ImageInternalFormat::COMPRESSED_SIGNED_RED_RGTC1:{ return true; break; }
        case ImageInternalFormat::COMPRESSED_SIGNED_RG_RGTC2:{ return true; break; }
        case ImageInternalFormat::COMPRESSED_SRGB:{ return true; break; }
        case ImageInternalFormat::COMPRESSED_SRGB_ALPHA:{ return true; break; }
        case ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:{ return true; break; }
        case ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:{ return true; break; }
        case ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:{ return true; break; }
        case ImageInternalFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT:{ return true; break; }
        default:{ return false; break; }
    }
    return false;
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

Texture::Texture(uint _w, uint _h,ImagePixelType::Type _pxlType,ImagePixelFormat::Format _pxlFormat,ImageInternalFormat::Format _internal,float _divisor):m_i(new impl){
    m_i->m_Files.push_back("FB");
    m_i->_initFramebuffer(_w,_h,_pxlType,_pxlFormat,_internal,_divisor,this);
}
Texture::Texture(const sf::Image& _sfImage,string _name,GLuint _openglTextureType,bool _genMipMaps,ImageInternalFormat::Format _internalFormat):m_i(new impl){
    m_i->m_Files.push_back("PIXELS");
    m_i->_initFromPixelsMemory(_sfImage,_name,_openglTextureType,_genMipMaps,_internalFormat,this);
}
Texture::Texture(string _filename,GLuint _openglTextureType,bool _genMipMaps,ImageInternalFormat::Format _internalFormat):m_i(new impl){
    m_i->m_Files.push_back(_filename);
    m_i->_initFromImageFile(_filename,_openglTextureType,_genMipMaps,_internalFormat,this);
}
//CubemapFrom6ImageFiles
Texture::Texture(string files[],string _name,bool _genMipMaps,ImageInternalFormat::Format _internalFormat):m_i(new impl){
    for(uint j = 0; j < 6; ++j){ 
        m_i->m_Files.push_back( files[j] ); 
    }
    m_i->_initCubemap(_name,_internalFormat,_genMipMaps,this);
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
