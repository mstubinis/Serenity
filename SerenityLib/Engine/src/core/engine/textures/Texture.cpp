#include <core/engine/Engine.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/textures/DDS.h>
#include <core/engine/Engine_Window.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/renderer/FramebufferObject.h>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>
#include <fstream>

using namespace Engine;
using namespace Engine::epriv;
using namespace Engine::epriv::textures;
using namespace std;

Texture* Texture::White    = nullptr;
Texture* Texture::Black    = nullptr;
Texture* Texture::Checkers = nullptr;

class Texture::impl final{
    friend struct epriv::TextureLoader;
    friend class ::Texture;
    public:
        vector<ImageLoadedStructure*>   m_ImagesDatas;
        vector<GLuint>                  m_TextureAddress;
        GLuint                          m_Type;
        TextureType::Type               m_TextureType;
        bool                            m_Mipmapped;
        bool                            m_IsToBeMipmapped;
        GLuint                          m_MinFilter; //used to determine filter type for mipmaps

        void _initCommon(GLuint _openglTextureType,bool _toBeMipmapped){
            m_Mipmapped = false;
            m_IsToBeMipmapped = _toBeMipmapped;
            m_MinFilter = GL_LINEAR;
            m_Type = _openglTextureType;
        }
        void _initFramebuffer(uint _w,uint _h,ImagePixelType::Type _pixelType,ImagePixelFormat::Format _pixelFormat,ImageInternalFormat::Format _internalFormat,float _divisor,Texture& super){
            uint _width(uint(float(_w)*_divisor));
            uint _height(uint(float(_h)*_divisor));
            ImageLoadedStructure* image = new ImageLoadedStructure(_width,_height,_pixelType,_pixelFormat,_internalFormat);

            _initCommon(GL_TEXTURE_2D,false);

            m_ImagesDatas.push_back(image);
            super.setName("RenderTarget");
            super.load();
        }
        void _initFromPixelsMemory(const sf::Image& _sfImage,string _name,GLuint _openglTextureType,bool _genMipMaps,ImageInternalFormat::Format _internalFormat,Texture& super){
            ImageLoadedStructure* image = new ImageLoadedStructure(_sfImage,_name);
            image->pixelType = ImagePixelType::UNSIGNED_BYTE;
            image->internalFormat = _internalFormat;

            _initCommon(_openglTextureType,_genMipMaps);
   
            TextureLoader::ChoosePixelFormat(image->pixelFormat,image->internalFormat);
            m_ImagesDatas.push_back(image);
            super.setName(_name);
            super.load();
        }
        void _initFromImageFile(string _filename,GLuint _openglTextureType,bool _genMipMaps,ImageInternalFormat::Format _internalFormat,Texture& super){	
            ImageLoadedStructure* image = new ImageLoadedStructure();
            image->filename = _filename;
            const string& extension = boost::filesystem::extension(_filename);
            _initCommon(_openglTextureType,_genMipMaps);      
            if(extension == ".dds"){
                TextureLoader::LoadDDSFile(super,_filename,*image);
            }else{
                image->pixelType = ImagePixelType::UNSIGNED_BYTE;
                image->internalFormat = _internalFormat;
            }
            TextureLoader::ChoosePixelFormat(image->pixelFormat,image->internalFormat);
            m_ImagesDatas.insert(m_ImagesDatas.begin(),image);

            super.setName(_filename);
            super.load();
        }
        void _initCubemap(string _name,ImageInternalFormat::Format _internalFormat,bool _genMipMaps,Texture& super){
            for(auto& sideImage:m_ImagesDatas){
                m_Type = GL_TEXTURE_CUBE_MAP;
                sideImage->pixelType = ImagePixelType::UNSIGNED_BYTE;
                sideImage->internalFormat = _internalFormat;
                TextureLoader::ChoosePixelFormat(sideImage->pixelFormat,sideImage->internalFormat);	
            }
            super.setName(_name);
            super.load();
        }
        void _load_CPU(Texture& super){
            for(auto& image:m_ImagesDatas){
                if(image->filename != ""){
                    bool _do = false;
                    if(image->mipmaps.size() == 0){ _do = true; }
                    for(auto& mip:image->mipmaps){
                        if(mip.pixels.size() == 0){ _do = true; }
                    }
                    if(_do){
                        const string& extension = boost::filesystem::extension(image->filename);
                        if(extension == ".dds"){
                            TextureLoader::LoadDDSFile(super,image->filename,*image);
                        }else{
                            sf::Image _sfImage;
                            _sfImage.loadFromFile(image->filename);
                            image->load(_sfImage,image->filename);
                        }
                    }
                }
            }
        }
        void _load_GPU(Texture& super){
            _unload_GPU(super);
            if(m_TextureAddress.size() == 0)
                m_TextureAddress.push_back(0);
            Renderer::genAndBindTexture(m_Type,m_TextureAddress[0]);
            switch(m_TextureType){
                case TextureType::RenderTarget:{
                    TextureLoader::LoadTextureFramebufferIntoOpenGL(super);
                    break;
                }
                case TextureType::Texture1D:{ 
                    break;
                }
                case TextureType::Texture2D:{
                    TextureLoader::LoadTexture2DIntoOpenGL(super); 
                    break;
                }
                case TextureType::Texture3D:{ 
                    break; 
                }
                case TextureType::CubeMap:{
                    TextureLoader::LoadTextureCubemapIntoOpenGL(super); 
                    break;
                }
                default:{ break; }
            }
            if (m_IsToBeMipmapped) {
                TextureLoader::GenerateMipmapsOpenGL(super);
            }
        }

        void _unload_GPU(Texture& super){
            for(uint i = 0; i < m_TextureAddress.size(); ++i){
                glDeleteTextures(1,&m_TextureAddress[i]);
            }
            vector_clear(m_TextureAddress);
        }
        void _unload_CPU(Texture& super){
            for(auto& image:m_ImagesDatas){
                if(image->filename != ""){
                    for(auto& mipmap:image->mipmaps){
                        if(mipmap.pixels.size() == 0){
                            vector_clear(mipmap.pixels);
                        }
                    }	
                }
            }  
            m_Mipmapped = false;
        }
        void _resize(epriv::FramebufferObject& fbo,const uint& w, const uint& h){
            if(m_TextureType != TextureType::RenderTarget){
                cout << "Error: Non-framebuffer texture cannot be resized. Returning..." << endl;
                return;
            }
            const float _divisor = fbo.divisor();
            Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
            const uint _w = static_cast<uint>(static_cast<float>(w) * _divisor);
            const uint _h = static_cast<uint>(static_cast<float>(h) * _divisor);
            auto& imageData = *m_ImagesDatas[0];
            imageData.mipmaps[0].width = _w;
            imageData.mipmaps[0].height = _h;
            glTexImage2D(m_Type,0,imageData.internalFormat,_w,_h,0,imageData.pixelFormat,imageData.pixelType,NULL);
        }
        void _importIntoOpenGL(const ImageMipmap& mipmap,const GLuint& openGLType){
            auto& imageData = *m_ImagesDatas[0];
            if(TextureLoader::IsCompressedType(imageData.internalFormat) && mipmap.compressedSize != 0)
                glCompressedTexImage2D(openGLType,mipmap.level,imageData.internalFormat,mipmap.width,mipmap.height,0,mipmap.compressedSize,&mipmap.pixels[0]);
            else
                glTexImage2D(openGLType,mipmap.level,imageData.internalFormat,mipmap.width,mipmap.height,0,imageData.pixelFormat,imageData.pixelType,&mipmap.pixels[0]);
        }
};

void epriv::TextureLoader::LoadDDSFile(Texture& texture, const string& filename,ImageLoadedStructure& image){
    auto& i = *texture.m_i;

    ifstream stream(filename.c_str(), ios::binary);
    if (!stream) return;

    uchar header_buffer[128];
    stream.read((char*)header_buffer, sizeof(header_buffer));

    DDS::DDS_Header head(header_buffer);
    if (head.magic != 0x20534444) { stream.close(); return; } //check if this is "DDS "
    //DX10 header here
    DDS::DDS_Header_DX10 headDX10;
    if( (head.header_flags & DDS::DDPF_FOURCC) && head.format.fourCC == FourCC_DX10 ){
        uchar header_buffer_DX10[20];
        stream.read((char*)header_buffer_DX10, sizeof(header_buffer_DX10));
        headDX10.fill(header_buffer_DX10);
    }
    uint32_t factor, blockSize, offset = 0;
    switch(head.format.fourCC){
        case FourCC_DXT1:{ 
            factor = 2;
            blockSize = 8;
            image.internalFormat = ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
            break; 
        }
        case FourCC_DXT2:{ 
            factor = 4;
            blockSize = 16;
            break;
        }
        case FourCC_DXT3:{ 
            factor = 4;
            blockSize = 16;
            image.internalFormat = ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
            break; 
        }
        case FourCC_DXT4:{ 
            factor = 4;
            blockSize = 16;
            break;
        }
        case FourCC_DXT5:{
            factor = 4;
            blockSize = 16;
            image.internalFormat = ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
            break; 
        }
        case FourCC_DX10:{ 
            break; 
        }
        case FourCC_ATI1:{ //useful for 1 channel textures (greyscales, glow / specular / ao / smoothness / metalness etc)
            factor = 2;
            blockSize = 8;
            image.internalFormat = ImageInternalFormat::COMPRESSED_RED_RGTC1;
            break;
        }
        case FourCC_ATI2:{//aka ATI2n aka 3Dc aka LATC2 aka BC5 - used for normal maps (store x,y recalc z) z = sqrt( 1-x*x-y*y )
            blockSize = 16;
            image.internalFormat = ImageInternalFormat::COMPRESSED_RG_RGTC2;
            break; 
        }
        case FourCC_RXGB:{ //By its design, it is just a DXT5 image with reading it in the shader differently
            //As I recall, the most you would have to do in the shader is something like:
            //vec3 normal;
            //normal.xy = texture2D(RXGBnormalmap, texcoord).ag;
            //normal.z = sqrt(1.0 - normal.x * normal.x - normal.y * normal.y);
            factor = 4;
            blockSize = 16;
            image.internalFormat = ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT;
            break;
        }
        case FourCC_$:{ 
            break; 
        }
        case FourCC_o:{ 
            break; 
        }
        case FourCC_p:{ 
            break; 
        }
        case FourCC_q:{ 
            break; 
        }
        case FourCC_r:{ 
            break; 
        }
        case FourCC_s:{ 
            break; 
        }
        case FourCC_t:{ 
            break; 
        }
        case FourCC_BC4U:{ 
            break; 
        }
        case FourCC_BC4S:{ 
            break; 
        }
        case FourCC_BC5U:{ 
            break;
        }
        case FourCC_BC5S:{ 
            break; 
        }
        case FourCC_RGBG:{ 
            break;
        }
        case FourCC_GRGB:{
            break;
        }
        case FourCC_YUY2:{
            break;
        }
        default:{ 
            return;
        }
    }

    uint numberOfMainImages = 1;
    if(head.caps & DDS::DDS_CAPS_COMPLEX){
        if(head.caps2 & DDS::DDS_CAPS2_CUBEMAP){//cubemap
            //note: in skybox dds files, especially in gimp, layer order is as follows:
            //right,left,top,bottom,front,back
            if(head.caps2 & DDS::DDS_CAPS2_CUBEMAP_POSITIVEX){ ++numberOfMainImages; }//right
            if(head.caps2 & DDS::DDS_CAPS2_CUBEMAP_NEGATIVEX){ ++numberOfMainImages; }//left
            if(head.caps2 & DDS::DDS_CAPS2_CUBEMAP_POSITIVEY){ ++numberOfMainImages; }//top
            if(head.caps2 & DDS::DDS_CAPS2_CUBEMAP_NEGATIVEY){ ++numberOfMainImages; }//bottom
            if(head.caps2 & DDS::DDS_CAPS2_CUBEMAP_POSITIVEZ){ ++numberOfMainImages; }//front
            if(head.caps2 & DDS::DDS_CAPS2_CUBEMAP_NEGATIVEZ){ ++numberOfMainImages; }//back
            i.m_Type = GL_TEXTURE_CUBE_MAP;
            i.m_TextureType = TextureType::CubeMap;
            i.m_IsToBeMipmapped = false;
            --numberOfMainImages;
        }
    }
    
    const uint& bufferSize = (head.mipMapCount >= 2 ? head.pitchOrlinearSize * factor : head.pitchOrlinearSize);
    uchar* pxls = (uchar*)malloc(bufferSize * numberOfMainImages);
    stream.read((char*)pxls, bufferSize * numberOfMainImages);
    stream.close();

    image.pixelFormat = ImagePixelFormat::RGBA;
    image.pixelType = ImagePixelType::UNSIGNED_BYTE;	

    uint _width = head.w;
    uint _height = head.h;
    for(uint mainImageLevel = 0; mainImageLevel < numberOfMainImages; ++mainImageLevel){

        ImageLoadedStructure* imgPtr = nullptr;
        if(mainImageLevel == 0){ 
            imgPtr = &image; 
        }else if(i.m_ImagesDatas.size() < mainImageLevel){                    
            imgPtr = new ImageLoadedStructure();
            imgPtr->pixelFormat = image.pixelFormat;
            imgPtr->pixelType = image.pixelType;
            imgPtr->internalFormat = image.internalFormat;
            i.m_ImagesDatas.push_back(imgPtr);
        }else{
            imgPtr = i.m_ImagesDatas[mainImageLevel];
        }

        _width = head.w;
        _height = head.h;
        for (uint level = 0; level < head.mipMapCount && (_width || _height); ++level){
            if(level > 0 && (_width < 64 || _height < 64)) break;
            ImageMipmap* mipmap    = nullptr;
            ImageMipmap  mipMapCon = ImageMipmap();
            if(level >= 1){ mipmap = &mipMapCon; }
            else{           mipmap = &imgPtr->mipmaps[0]; }
            mipmap->level = level;
            mipmap->width = _width;
            mipmap->height = _height;
            mipmap->compressedSize = ((_width + 3) / 4) * ((_height + 3) / 4) * blockSize;

            mipmap->pixels.resize(mipmap->compressedSize);
            for(uint t = 0; t < mipmap->pixels.size(); ++t){
                const uint& _index = offset + t;
                mipmap->pixels[t] = pxls[_index];
            }
            _width = Math::Max(uint(_width / 2), 1);
            _height = Math::Max(uint(_height / 2), 1);
            offset += mipmap->compressedSize;
            if(level >= 1){ imgPtr->mipmaps.push_back(*mipmap); }
        }
    }
    free(pxls);
}
void epriv::TextureLoader::LoadTexture2DIntoOpenGL(Texture& _texture){
    auto& i = *_texture.m_i;
    Renderer::bindTextureForModification(i.m_Type,i.m_TextureAddress[0]);
    for(auto& mipmap:i.m_ImagesDatas[0]->mipmaps){
        i._importIntoOpenGL(mipmap,i.m_Type);
        //TextureLoader::WithdrawPixelsFromOpenGLMemory(_texture,0,mipmap.level);
    }
    _texture.setFilter(TextureFilter::Linear);
    _texture.setWrapping(TextureWrap::Repeat);
}
void epriv::TextureLoader::LoadTextureFramebufferIntoOpenGL(Texture& _texture){
    const auto& i = *_texture.m_i;
    Renderer::bindTextureForModification(i.m_Type,i.m_TextureAddress[0]);
    const auto& image = *i.m_ImagesDatas[0];
    const uint& _w = image.mipmaps[0].width;
    const uint& _h = image.mipmaps[0].height;
    glTexImage2D(i.m_Type,0,image.internalFormat,_w,_h,0,image.pixelFormat,image.pixelType,NULL);
    _texture.setFilter(TextureFilter::Linear);
    _texture.setWrapping(TextureWrap::ClampToEdge);
}
void epriv::TextureLoader::LoadTextureCubemapIntoOpenGL(Texture& _texture){
    auto& i = *_texture.m_i;
    Renderer::bindTextureForModification(i.m_Type,i.m_TextureAddress[0]);
    uint imageIndex = 0;
    for(auto& image:i.m_ImagesDatas){
        for(auto& mipmap:image->mipmaps){
            i._importIntoOpenGL(mipmap,GL_TEXTURE_CUBE_MAP_POSITIVE_X + imageIndex);
            //TextureLoader::WithdrawPixelsFromOpenGLMemory(_texture,imageIndex,mipmap.level);
        }
        ++imageIndex;
    }
    _texture.setFilter(TextureFilter::Linear);
    _texture.setWrapping(TextureWrap::ClampToEdge);

}
void epriv::TextureLoader::WithdrawPixelsFromOpenGLMemory(Texture& texture, const uint& imageIndex, const uint& mipmapLevel){
    const auto& i = *texture.m_i;
    auto& image = *i.m_ImagesDatas[imageIndex];
    auto& pxls = image.mipmaps[mipmapLevel].pixels;
    if(pxls.size() != 0) 
        return;
    const uint& _w = image.mipmaps[mipmapLevel].width;
    const uint& _h = image.mipmaps[mipmapLevel].height;
    pxls.resize(_w * _h * 4);
    Renderer::bindTextureForModification(i.m_Type,i.m_TextureAddress[0]);
    glGetTexImage(i.m_Type,0,image.pixelFormat,image.pixelType,&pxls[0]);
}
void epriv::TextureLoader::ChoosePixelFormat(ImagePixelFormat::Format& out, const ImageInternalFormat::Format& in){
    switch(in){
        case ImageInternalFormat::COMPRESSED_RED:{ out = ImagePixelFormat::RED;break; }
        case ImageInternalFormat::COMPRESSED_RED_RGTC1:{ out = ImagePixelFormat::RED;break; }//recheck this
        case ImageInternalFormat::COMPRESSED_RG:{ out = ImagePixelFormat::RG;break; }
        case ImageInternalFormat::COMPRESSED_RGB:{ out = ImagePixelFormat::RGB;break; }
        case ImageInternalFormat::COMPRESSED_RGBA:{ out = ImagePixelFormat::RGBA;break; }
        case ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT1_EXT:{ out = ImagePixelFormat::RGBA;break; }
        case ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT3_EXT:{ out = ImagePixelFormat::RGBA;break; }
        case ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT:{ out = ImagePixelFormat::RGBA;break; }
        case ImageInternalFormat::COMPRESSED_RGB_S3TC_DXT1_EXT:{ out = ImagePixelFormat::RGB;break; }
        case ImageInternalFormat::COMPRESSED_RG_RGTC2:{ out = ImagePixelFormat::RG;break; }//recheck this
        case ImageInternalFormat::COMPRESSED_SIGNED_RED_RGTC1:{ out = ImagePixelFormat::RED;break; }//recheck this
        case ImageInternalFormat::COMPRESSED_SIGNED_RG_RGTC2:{ out = ImagePixelFormat::RG;break; }//recheck this
        case ImageInternalFormat::COMPRESSED_SRGB:{ out = ImagePixelFormat::RGB;break; }
        case ImageInternalFormat::COMPRESSED_SRGB_ALPHA:{ out = ImagePixelFormat::RGBA;break; }
        case ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:{ out = ImagePixelFormat::RGBA;break; }
        case ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:{ out = ImagePixelFormat::RGBA;break; }
        case ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:{ out = ImagePixelFormat::RGBA;break; }
        case ImageInternalFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT:{ out = ImagePixelFormat::RGB;break; }
        case ImageInternalFormat::Depth16:{ out = ImagePixelFormat::DEPTH_COMPONENT;break; }
        case ImageInternalFormat::Depth24:{ out = ImagePixelFormat::DEPTH_COMPONENT;break; }
        case ImageInternalFormat::Depth32:{ out = ImagePixelFormat::DEPTH_COMPONENT;break; }
        case ImageInternalFormat::Depth32F:{ out = ImagePixelFormat::DEPTH_COMPONENT;break; }
        case ImageInternalFormat::Depth24Stencil8:{ out = ImagePixelFormat::DEPTH_STENCIL;break; }
        case ImageInternalFormat::Depth32FStencil8:{ out = ImagePixelFormat::DEPTH_STENCIL;break; }
        case ImageInternalFormat::DEPTH_COMPONENT:{ out = ImagePixelFormat::DEPTH_COMPONENT;break; }
        case ImageInternalFormat::DEPTH_STENCIL:{ out = ImagePixelFormat::DEPTH_STENCIL;break; }
        case ImageInternalFormat::R11F_G11F_B10F:{ out = ImagePixelFormat::RGB;break; }
        case ImageInternalFormat::R16:{ out = ImagePixelFormat::RED;break; }
        case ImageInternalFormat::R16F:{ out = ImagePixelFormat::RED;break; }
        case ImageInternalFormat::R16I:{ out = ImagePixelFormat::RED_INTEGER;break; }
        case ImageInternalFormat::R16UI:{ out = ImagePixelFormat::RED_INTEGER;break; }
        case ImageInternalFormat::R16_SNORM:{ out = ImagePixelFormat::RED;break; }
        case ImageInternalFormat::R32F:{ out = ImagePixelFormat::RED;break; }
        case ImageInternalFormat::R32I:{ out = ImagePixelFormat::RED_INTEGER;break; }
        case ImageInternalFormat::R32UI:{ out = ImagePixelFormat::RED_INTEGER;break; }
        case ImageInternalFormat::R3_G3_B2:{ out = ImagePixelFormat::RGB;break; }
        case ImageInternalFormat::R8:{ out = ImagePixelFormat::RED;break; }
        case ImageInternalFormat::R8I:{ out = ImagePixelFormat::RED_INTEGER;break; }
        case ImageInternalFormat::R8UI:{ out = ImagePixelFormat::RED_INTEGER;break; }
        case ImageInternalFormat::R8_SNORM:{ out = ImagePixelFormat::RED;break; }
        case ImageInternalFormat::RED:{ out = ImagePixelFormat::RED;break; }
        case ImageInternalFormat::RG:{ out = ImagePixelFormat::RG;break; }
        case ImageInternalFormat::RG16:{ out = ImagePixelFormat::RG;break; }
        case ImageInternalFormat::RG16F:{ out = ImagePixelFormat::RG;break; }
        case ImageInternalFormat::RG16I:{ out = ImagePixelFormat::RG_INTEGER;break; }
        case ImageInternalFormat::RG16UI:{ out = ImagePixelFormat::RG_INTEGER;break; }
        case ImageInternalFormat::RG16_SNORM:{ out = ImagePixelFormat::RG;break; }
        case ImageInternalFormat::RG32F:{ out = ImagePixelFormat::RG;break; }
        case ImageInternalFormat::RG32I:{ out = ImagePixelFormat::RG_INTEGER;break; }
        case ImageInternalFormat::RG32UI:{ out = ImagePixelFormat::RG_INTEGER;break; }
        case ImageInternalFormat::RG8:{ out = ImagePixelFormat::RG;break; }
        case ImageInternalFormat::RG8I:{ out = ImagePixelFormat::RG_INTEGER;break; }
        case ImageInternalFormat::RG8UI:{ out = ImagePixelFormat::RG_INTEGER;break; }
        case ImageInternalFormat::RG8_SNORM:{ out = ImagePixelFormat::RG;break; }
        case ImageInternalFormat::RGB:{ out = ImagePixelFormat::RGB;break; }
        case ImageInternalFormat::RGB10:{ out = ImagePixelFormat::RGB;break; }
        case ImageInternalFormat::RGB10_A2:{ out = ImagePixelFormat::RGBA;break; }
        case ImageInternalFormat::RGB12:{ out = ImagePixelFormat::RGB;break; }
        case ImageInternalFormat::RGB16F:{ out = ImagePixelFormat::RGB;break; }
        case ImageInternalFormat::RGB16I:{ out = ImagePixelFormat::RGB_INTEGER;break; }
        case ImageInternalFormat::RGB16UI:{ out = ImagePixelFormat::RGB_INTEGER;break; }
        case ImageInternalFormat::RGB16_SNORM:{ out = ImagePixelFormat::RGB;break; }
        case ImageInternalFormat::RGB32F:{ out = ImagePixelFormat::RGB;break; }
        case ImageInternalFormat::RGB32I:{ out = ImagePixelFormat::RGB_INTEGER;break; }
        case ImageInternalFormat::RGB32UI:{ out = ImagePixelFormat::RGB_INTEGER;break; }
        case ImageInternalFormat::RGB4:{ out = ImagePixelFormat::RGB;break; }
        case ImageInternalFormat::RGB5:{ out = ImagePixelFormat::RGB;break; }
        case ImageInternalFormat::RGB5_A1:{ out = ImagePixelFormat::RGBA;break; }
        case ImageInternalFormat::RGB8:{ out = ImagePixelFormat::RGB;break; }
        case ImageInternalFormat::RGB8I:{ out = ImagePixelFormat::RGB_INTEGER;break; }
        case ImageInternalFormat::RGB8UI:{ out = ImagePixelFormat::RGB_INTEGER;break; }
        case ImageInternalFormat::RGB8_SNORM:{ out = ImagePixelFormat::RGB;break; }
        case ImageInternalFormat::RGB9_E5:{ out = ImagePixelFormat::RGB;break; }//recheck this
        case ImageInternalFormat::RGBA:{ out = ImagePixelFormat::RGBA;break; }
        case ImageInternalFormat::RGBA12:{ out = ImagePixelFormat::RGBA;break; }
        case ImageInternalFormat::RGBA16:{ out = ImagePixelFormat::RGBA;break; }
        case ImageInternalFormat::RGBA16F:{ out = ImagePixelFormat::RGBA;break; }
        case ImageInternalFormat::RGBA16I:{ out = ImagePixelFormat::RGBA_INTEGER;break; }
        case ImageInternalFormat::RGBA16UI:{ out = ImagePixelFormat::RGBA_INTEGER;break; }
        case ImageInternalFormat::RGBA2:{ out = ImagePixelFormat::RGBA;break; }
        case ImageInternalFormat::RGBA32F:{ out = ImagePixelFormat::RGBA;break; }
        case ImageInternalFormat::RGBA32I:{ out = ImagePixelFormat::RGBA_INTEGER;break; }
        case ImageInternalFormat::RGBA32UI:{ out = ImagePixelFormat::RGBA_INTEGER;break; }
        case ImageInternalFormat::RGBA4:{ out = ImagePixelFormat::RGBA;break; }
        case ImageInternalFormat::RGBA8:{ out = ImagePixelFormat::RGBA;break; }
        case ImageInternalFormat::RGBA8I:{ out = ImagePixelFormat::RGBA_INTEGER;break; }
        case ImageInternalFormat::RGBA8UI:{ out = ImagePixelFormat::RGBA_INTEGER;break; }
        case ImageInternalFormat::RGBA8_SNORM:{ out = ImagePixelFormat::RGBA;break; }
        case ImageInternalFormat::SRGB8:{ out = ImagePixelFormat::RGB;break; }
        case ImageInternalFormat::SRGB8_ALPHA8:{ out = ImagePixelFormat::RGBA;break; }
        case ImageInternalFormat::StencilIndex8:{ out = ImagePixelFormat::STENCIL_INDEX;break; }
        case ImageInternalFormat::STENCIL_INDEX:{ out = ImagePixelFormat::STENCIL_INDEX;break; }
        default:{ out = ImagePixelFormat::RGBA;break; }
    }
}
const bool epriv::TextureLoader::IsCompressedType(const ImageInternalFormat::Format& format) {
    switch(format){
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
void epriv::TextureLoader::GenerateMipmapsOpenGL(Texture& texture){
    auto& i = *texture.m_i;
    if(i.m_Mipmapped) 
        return;
    //const auto& image = *i.m_ImagesDatas[0];
    //const uint& _w = image.mipmaps[0].width;
    //const uint& _h = image.mipmaps[0].height;
    Renderer::bindTextureForModification(i.m_Type, i.m_TextureAddress[0]);
    glTexParameteri(i.m_Type, GL_TEXTURE_BASE_LEVEL, 0);
    if(i.m_MinFilter == GL_LINEAR){        
        i.m_MinFilter = GL_LINEAR_MIPMAP_LINEAR; 
    }else if(i.m_MinFilter == GL_NEAREST){  
        i.m_MinFilter = GL_NEAREST_MIPMAP_NEAREST;
    }
    glTexParameteri(i.m_Type, GL_TEXTURE_MIN_FILTER, i.m_MinFilter);
    glGenerateMipmap(i.m_Type);
    i.m_Mipmapped = true;
    //uint mipmaplevels = uint(glm::log2(glm::max(_w,_h)) + 1.0f);
}
void epriv::TextureLoader::EnumWrapToGL(uint& gl, const TextureWrap::Wrap& wrap){
    if(wrap == TextureWrap::Repeat)              gl = GL_REPEAT;
    else if(wrap == TextureWrap::RepeatMirrored) gl = GL_MIRRORED_REPEAT;
    else if(wrap == TextureWrap::ClampToBorder)  gl = GL_CLAMP_TO_BORDER;
    else if(wrap == TextureWrap::ClampToEdge)    gl = GL_CLAMP_TO_EDGE;
}
void epriv::TextureLoader::EnumFilterToGL(uint& gl, const TextureFilter::Filter& filter, const bool& min){
    if(min){
        if     (filter == TextureFilter::Linear)                  gl = GL_LINEAR;
        else if(filter == TextureFilter::Nearest)                 gl = GL_NEAREST;
        else if(filter == TextureFilter::Linear_Mipmap_Linear)    gl = GL_LINEAR_MIPMAP_LINEAR;
        else if(filter == TextureFilter::Linear_Mipmap_Nearest)   gl = GL_LINEAR_MIPMAP_NEAREST;
        else if(filter == TextureFilter::Nearest_Mipmap_Linear)   gl = GL_NEAREST_MIPMAP_LINEAR;
        else if(filter == TextureFilter::Nearest_Mipmap_Nearest)  gl = GL_NEAREST_MIPMAP_NEAREST;
    }else{
             if(filter == TextureFilter::Linear)                  gl = GL_LINEAR;
        else if(filter == TextureFilter::Nearest)                 gl = GL_NEAREST;
        else if(filter == TextureFilter::Linear_Mipmap_Linear)    gl = GL_LINEAR;
        else if(filter == TextureFilter::Linear_Mipmap_Nearest)   gl = GL_LINEAR;
        else if(filter == TextureFilter::Nearest_Mipmap_Linear)   gl = GL_NEAREST;
        else if(filter == TextureFilter::Nearest_Mipmap_Nearest)  gl = GL_NEAREST;
    }
}

Texture::Texture(const uint& w, const uint& h, const ImagePixelType::Type& pxlType, const ImagePixelFormat::Format& pxlFormat, const ImageInternalFormat::Format& _internal, const float& divisor):m_i(new impl){
    m_i->m_TextureType = TextureType::RenderTarget;
    m_i->_initFramebuffer(w,h,pxlType,pxlFormat,_internal,divisor,*this);
}
Texture::Texture(const sf::Image& sfImage, const string& name, const bool& genMipMaps, const ImageInternalFormat::Format& _internal, const GLuint& openglTextureType):m_i(new impl){
    m_i->m_TextureType = TextureType::Texture2D;
    m_i->_initFromPixelsMemory(sfImage,name,openglTextureType,genMipMaps, _internal,*this);
}
Texture::Texture(const string& filename, const bool& genMipMaps, const ImageInternalFormat::Format& _internal, const GLuint& openglTextureType):m_i(new impl){
    m_i->m_TextureType = TextureType::Texture2D;
    m_i->_initFromImageFile(filename, openglTextureType, genMipMaps, _internal, *this);
}
Texture::Texture(const string files[], const string& name, const bool& genMipMaps, const ImageInternalFormat::Format& _internal):m_i(new impl){
    m_i->m_TextureType = TextureType::CubeMap;
    for(uint j = 0; j < 6; ++j){ 
        ImageLoadedStructure* image = new ImageLoadedStructure();
        image->filename = files[j];
        m_i->m_ImagesDatas.push_back(image);
    }
    m_i->_initCubemap(name,_internal,genMipMaps,*this);
}
Texture::~Texture(){
    unload();
    SAFE_DELETE_VECTOR(m_i->m_ImagesDatas);
}
void Texture::render(const glm::vec2& position, const glm::vec4& color, const float& angle, const glm::vec2& scale, const float& depth){
    if (m_i->m_TextureType == TextureType::CubeMap)
        return;
    Renderer::renderTexture(*this, position, color, angle, scale, depth);
}
void Texture::setXWrapping(const TextureWrap::Wrap& wrap){
    Texture::setXWrapping(m_i->m_Type, wrap);
}
void Texture::setYWrapping(const TextureWrap::Wrap& wrap){
    Texture::setYWrapping(m_i->m_Type, wrap);
}
void Texture::setZWrapping(const TextureWrap::Wrap& wrap){
    Texture::setZWrapping(m_i->m_Type, wrap);
}
void Texture::setWrapping(const TextureWrap::Wrap& wrap){
    Texture::setWrapping(m_i->m_Type, wrap);
}
void Texture::setMinFilter(const TextureFilter::Filter& filter){
    Texture::setMinFilter(m_i->m_Type, filter);
    m_i->m_MinFilter = filter;
}
void Texture::setMaxFilter(const TextureFilter::Filter& filter){
    Texture::setMaxFilter(m_i->m_Type, filter);
}
void Texture::setFilter(const TextureFilter::Filter& filter){
    Texture::setFilter(m_i->m_Type, filter);
}
void Texture::setXWrapping(const GLuint& type, const TextureWrap::Wrap& wrap){
    GLuint gl;
    TextureLoader::EnumWrapToGL(gl, wrap);
    glTexParameteri(type, GL_TEXTURE_WRAP_S, gl);
}
void Texture::setYWrapping(const GLuint& type, const TextureWrap::Wrap& wrap){
    GLuint gl;
    TextureLoader::EnumWrapToGL(gl, wrap);
    glTexParameteri(type, GL_TEXTURE_WRAP_T, gl);
}
void Texture::setZWrapping(const GLuint& type, const TextureWrap::Wrap& wrap){
    if (type != GL_TEXTURE_CUBE_MAP)
        return;
    GLuint gl;
    TextureLoader::EnumWrapToGL(gl, wrap);
    glTexParameteri(type, GL_TEXTURE_WRAP_R, gl);
}
void Texture::setWrapping(const GLuint& type, const TextureWrap::Wrap& wrap){
    Texture::setXWrapping(type, wrap);
    Texture::setYWrapping(type, wrap);
    Texture::setZWrapping(type, wrap);
}
void Texture::setMinFilter(const GLuint& type, const TextureFilter::Filter& filter){
    GLuint gl;
    TextureLoader::EnumFilterToGL(gl, filter, true);
    glTexParameteri(type, GL_TEXTURE_MIN_FILTER, gl);
}
void Texture::setMaxFilter(const GLuint& type, const TextureFilter::Filter& filter){
    GLuint gl;
    TextureLoader::EnumFilterToGL(gl, filter, false);
    glTexParameteri(type, GL_TEXTURE_MAG_FILTER, gl);
}
void Texture::setFilter(const GLuint& type, const TextureFilter::Filter& filter){
    Texture::setMinFilter(type, filter);
    Texture::setMaxFilter(type, filter);
}
void Texture::setAnisotropicFiltering(const float& anisotropicFiltering){
    auto& i = *m_i;
    Renderer::bindTextureForModification(i.m_Type, i.m_TextureAddress[0]);
    if(RenderManager::OPENGL_VERSION >= 46){
    	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, const_cast<GLfloat*>(&anisotropicFiltering));
    	glTexParameterf(i.m_Type, GL_TEXTURE_MAX_ANISOTROPY, anisotropicFiltering);
    }else{     
        if(OpenGLExtension::supported(OpenGLExtension::ARB_Ansiotropic_Filtering)){
            //glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_ARB, const_cast<GLfloat*>(&anisotropicFiltering));
            //glTexParameterf(i.m_Type, GL_TEXTURE_MAX_ANISOTROPY_ARB, anisotropicFiltering);
        }else if(OpenGLExtension::supported(OpenGLExtension::EXT_Ansiotropic_Filtering)){
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, const_cast<GLfloat*>(&anisotropicFiltering));
            glTexParameterf(i.m_Type, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropicFiltering);
        }
    }
}

void epriv::InternalTexturePublicInterface::LoadCPU(Texture& texture){
    texture.m_i->_load_CPU(texture);
}
void epriv::InternalTexturePublicInterface::LoadGPU(Texture& texture){
    texture.m_i->_load_GPU(texture);
    texture.EngineResource::load();
}
void epriv::InternalTexturePublicInterface::UnloadCPU(Texture& texture){
    texture.m_i->_unload_CPU(texture);
    texture.EngineResource::unload();
}
void epriv::InternalTexturePublicInterface::UnloadGPU(Texture& texture){
    texture.m_i->_unload_GPU(texture);      
}


void Texture::load(){
    if(!isLoaded()){
        auto& i = *m_i;
        auto& _this = *this;
        i._load_CPU(_this);
        i._load_GPU(_this);
        cout << "(Texture) ";
        EngineResource::load();
    }
}
void Texture::unload(){
    if(isLoaded()){
        auto& i = *m_i;
        auto& _this = *this;
        i._unload_GPU(_this);
        i._unload_CPU(_this);
        cout << "(Texture) ";
        EngineResource::unload();
    }
}
void Texture::genPBREnvMapData(const uint& convoludeTextureSize, const uint& preEnvFilterSize){
    auto& i = *m_i;
    if (i.m_TextureAddress.size() == 1) {
        i.m_TextureAddress.push_back(0);
        Renderer::genAndBindTexture(i.m_Type, i.m_TextureAddress[1]);
        for (uint i = 0; i < 6; ++i) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, convoludeTextureSize, convoludeTextureSize, 0, GL_RGB, GL_FLOAT, NULL);
        }
        setWrapping(TextureWrap::ClampToEdge);
        setFilter(TextureFilter::Linear);
    }
    if (i.m_TextureAddress.size() == 2) {
        i.m_TextureAddress.push_back(0);
        Renderer::genAndBindTexture(i.m_Type, i.m_TextureAddress[2]);
        for (uint i = 0; i < 6; ++i) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, preEnvFilterSize, preEnvFilterSize, 0, GL_RGB, GL_FLOAT, NULL);
        }
        setWrapping(TextureWrap::ClampToEdge);
        setMinFilter(TextureFilter::Linear_Mipmap_Linear);
        setMaxFilter(TextureFilter::Linear);
        glGenerateMipmap(i.m_Type);
    }
    Core::m_Engine->m_RenderManager._genPBREnvMapData(*this, convoludeTextureSize, preEnvFilterSize);
}
void Texture::resize(epriv::FramebufferObject& fbo, const uint& w, const uint& h){
    m_i->_resize(fbo,w,h); 
}
const bool Texture::mipmapped() const {
    return m_i->m_Mipmapped; 
}
const bool Texture::compressed() const {
    if(m_i->m_ImagesDatas[0]->mipmaps[0].compressedSize > 0) return true; return false;
}
const uchar* Texture::pixels(){
    TextureLoader::WithdrawPixelsFromOpenGLMemory(*this); 
    return &(m_i->m_ImagesDatas[0]->mipmaps[0].pixels)[0]; 
}
const GLuint& Texture::address(const uint& index) const { 
    return m_i->m_TextureAddress[index]; 
}
const uint Texture::numAddresses() const {
    return m_i->m_TextureAddress.size(); 
}
const GLuint Texture::type() const { 
    return m_i->m_Type; 
}
const uint Texture::width() const { 
    return m_i->m_ImagesDatas[0]->mipmaps[0].width; 
}
const uint Texture::height() const { 
    return m_i->m_ImagesDatas[0]->mipmaps[0].height; 
}
const ImageInternalFormat::Format Texture::internalFormat() const { 
    return m_i->m_ImagesDatas[0]->internalFormat; 
}
const ImagePixelFormat::Format Texture::pixelFormat() const { 
    return m_i->m_ImagesDatas[0]->pixelFormat; 
}
const ImagePixelType::Type Texture::pixelType() const { 
    return m_i->m_ImagesDatas[0]->pixelType; 
}
