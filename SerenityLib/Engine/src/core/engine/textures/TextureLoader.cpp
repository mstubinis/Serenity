#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/textures/TextureLoader.h>
#include <core/engine/textures/DDS.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/system/Engine.h>
#include <core/engine/renderer/FramebufferObject.h>
#include <core/engine/events/Event.h>

#include <boost/filesystem.hpp>
#include <SFML/Graphics.hpp>

using namespace Engine;
using namespace Engine::priv;
using namespace Engine::priv::textures;

void TextureLoader::CPUInitCommon(TextureCPUData& cpuData, TextureType textureType, bool toBeMipmapped) {
    cpuData.m_IsToBeMipmapped = toBeMipmapped;
    cpuData.m_TextureType     = textureType;
}

void TextureLoader::CPUInitFramebuffer(TextureCPUData& cpuData, int w, int h, ImagePixelType pxlType, ImagePixelFormat pxlFormat, ImageInternalFormat intFormat, float divisor) {
    cpuData.m_TextureType = TextureType::RenderTarget;
    int width             = (int)((float)w * divisor);
    int height            = (int)((float)h * divisor);

    TextureLoader::CPUInitCommon(cpuData, TextureType::Texture2D, false);

    auto& image                     = cpuData.m_ImagesDatas[0];
    image.m_Mipmaps[0].width        = width;
    image.m_Mipmaps[0].height       = height;
    image.m_PixelType               = pxlType;
    image.m_PixelFormat             = pxlFormat;
    image.m_InternalFormat          = intFormat;

    cpuData.m_Name = "RenderTgt";
}
void TextureLoader::CPUInitFromMemory(TextureCPUData& cpuData, const sf::Image& sfImage, const std::string& name, bool genMipMaps, ImageInternalFormat intFormat, TextureType textureType) {
    cpuData.m_TextureType       = TextureType::Texture2D;
    auto& image                 = cpuData.m_ImagesDatas[0];
    image.load(sfImage, name);
    image.m_PixelType           = ImagePixelType::UNSIGNED_BYTE;
    image.m_InternalFormat      = intFormat;

    TextureLoader::CPUInitCommon(cpuData, textureType, genMipMaps);

    image.m_PixelFormat = image.m_InternalFormat;
    cpuData.m_Name = name;
}
void TextureLoader::CPUInitFromMemory(Handle textureHandle, const sf::Image& sfImage, const std::string& name, bool genMipMaps, ImageInternalFormat intFormat, TextureType textureType) {
    auto& cpuData = textureHandle.get<Texture>()->m_CPUData;
    CPUInitFromMemory(cpuData, sfImage, name, genMipMaps, intFormat, textureType);
}
void TextureLoader::CPUInitFromFile(TextureCPUData& cpuData, const std::string& filename, bool genMipMaps, ImageInternalFormat intFormat, TextureType textureType) {
    cpuData.m_TextureType            = TextureType::Texture2D;
    auto& image                      = cpuData.m_ImagesDatas[0];
    image.m_Filename                 = filename;
    image.m_PixelType                = ImagePixelType::UNSIGNED_BYTE;
    image.m_InternalFormat           = intFormat;
    const std::string& extension     = boost::filesystem::extension(filename);
    TextureLoader::CPUInitCommon(cpuData, textureType, genMipMaps);
    if (extension == ".dds") {
        TextureLoader::LoadDDSFile(cpuData, filename, image);
    }

    image.m_PixelFormat = image.m_InternalFormat;
    cpuData.m_Name      = filename;
}
void TextureLoader::CPUInitFromFile(Handle textureHandle, const std::string& filename, bool genMipMaps, ImageInternalFormat intFormat, TextureType textureType) {
    auto& cpuData = textureHandle.get<Texture>()->m_CPUData;
    CPUInitFromFile(cpuData, filename, genMipMaps, intFormat, textureType);
}
void TextureLoader::CPUInitFromFilesCubemap(TextureCPUData& cpuData, const std::array<std::string, 6>& files, const std::string& name, bool genMipMaps, ImageInternalFormat intFormat) {
    cpuData.m_TextureType = TextureType::CubeMap;
    auto& image           = cpuData.m_ImagesDatas[0];
    image.m_Filename      = files[0];
    for (int j = 1; j < files.size(); ++j) {
        auto& imageInnerLoop      = cpuData.m_ImagesDatas.emplace_back();
        imageInnerLoop.m_Filename = files[j];
    }
    TextureLoader::CPUInitCommon(cpuData, TextureType::CubeMap, genMipMaps);
    for (auto& sideImage : cpuData.m_ImagesDatas) {
        sideImage.m_PixelType       = ImagePixelType::UNSIGNED_BYTE;
        sideImage.m_InternalFormat  = intFormat;
        sideImage.m_PixelFormat     = sideImage.m_InternalFormat;
    }
    cpuData.m_Name = name;
}
void TextureLoader::ImportIntoOpengl(Texture& texture, const Engine::priv::ImageMipmap& mipmap, TextureType textureType) {
    auto& imageData = texture.m_CPUData.m_ImagesDatas[0];
    if (imageData.m_InternalFormat.isCompressedType() && mipmap.compressedSize != 0) {
        GLCall(glCompressedTexImage2D(textureType.toGLType(), mipmap.level, (GLenum)imageData.m_InternalFormat, mipmap.width, mipmap.height, 0, mipmap.compressedSize,
            &mipmap.pixels[0]
        ));
    }else{
        GLCall(glTexImage2D(textureType.toGLType(), mipmap.level, (GLint)imageData.m_InternalFormat, mipmap.width, mipmap.height, 0,
            (GLenum)imageData.m_PixelFormat, (GLenum)imageData.m_PixelType, &mipmap.pixels[0]
        ));
    }
}

bool TextureLoader::LoadDDSFile(TextureCPUData& cpuData, const std::string& filename, ImageLoadedStructure& image_loaded_struct) {
    std::ifstream stream(filename.c_str(), std::ios::binary);
    if (!stream) {
        return false;
    }
    std::streampos fileSize;
    stream.unsetf(std::ios::skipws);
    stream.seekg(0, std::ios::end);
    fileSize = stream.tellg();
    stream.seekg(0, std::ios::beg);

    std::vector<std::uint8_t> file_data;
    file_data.reserve((unsigned int)fileSize);
    file_data.insert(file_data.begin(), std::istream_iterator<std::uint8_t>(stream), std::istream_iterator<std::uint8_t>());
    stream.close();

    std::array<std::uint8_t, 128> header_buffer;
    std::uint32_t progress = 0;
    for (size_t i = 0; i < header_buffer.size(); ++i) {
        header_buffer[i] = file_data[i];
        ++progress;
    }

    DDS::DDS_Header head(header_buffer);
    if (head.magic != 0x20534444) {  //checks if this is "DDS "
        return false; 
    }
    //DX10 header here
    DDS::DDS_Header_DX10 headDX10;
    if ((head.header_flags & DDS::DDPF_FOURCC) && head.format.fourCC == FourCC_DX10) {
        std::array<std::uint8_t, 20> header_buffer_DX10;
        for (size_t i = 0; i < header_buffer_DX10.size(); ++i) {
            header_buffer_DX10[i] = file_data[i + progress];
            ++progress;
        }
        headDX10.fill(header_buffer_DX10);
    }
    std::uint32_t factor, blockSize, offset = progress;
    //TODO: fill the rest of these out
    switch (head.format.fourCC) {
        case FourCC_DXT1: {
            factor = 2;
            blockSize = 8;
            image_loaded_struct.m_InternalFormat = ImageInternalFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT;
            break;
        }case FourCC_DXT2: {
            factor = 4;
            blockSize = 16;
            break;
        }case FourCC_DXT3: {
            factor = 4;
            blockSize = 16;
            image_loaded_struct.m_InternalFormat = ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
            break;
        }case FourCC_DXT4: {
            factor = 4;
            blockSize = 16;
            break;
        }case FourCC_DXT5: {
            factor = 4;
            blockSize = 16;
            image_loaded_struct.m_InternalFormat = ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
            break;
        }case FourCC_DX10: {
            break;
        }case FourCC_ATI1: { //useful for 1 channel textures (greyscales, glow / specular / ao / smoothness / metalness etc)
            factor = 2;
            blockSize = 8;
            image_loaded_struct.m_InternalFormat = ImageInternalFormat::COMPRESSED_RED_RGTC1;
            break;
        }case FourCC_ATI2: {//aka ATI2n aka 3Dc aka LATC2 aka BC5 - used for normal maps (store x,y recalc z) z = sqrt( 1-x*x-y*y )
            blockSize = 16;
            image_loaded_struct.m_InternalFormat = ImageInternalFormat::COMPRESSED_RG_RGTC2;
            break;
        }case FourCC_RXGB: { //By its design, it is just a DXT5 image with reading it in the shader differently
            //As I recall, the most you would have to do in the shader is something like:
            //vec3 normal;
            //normal.xy = texture2D(RXGBnormalmap, texcoord).ag;
            //normal.z = sqrt(1.0 - normal.x * normal.x - normal.y * normal.y);
            factor = 4;
            blockSize = 16;
            image_loaded_struct.m_InternalFormat = ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT;
            break;
        }case FourCC_$: {
            break;
        }case FourCC_o: {
            break;
        }case FourCC_p: {
            break;
        }case FourCC_q: {
            break;
        }case FourCC_r: {
            break;
        }case FourCC_s: {
            break;
        }case FourCC_t: {
            break;
        }case FourCC_BC4U: {
            break;
        }case FourCC_BC4S: {
            break;
        }case FourCC_BC5U: {
            break;
        }case FourCC_BC5S: {
            break;
        }case FourCC_RGBG: {
            break;
        }case FourCC_GRGB: {
            break;
        }case FourCC_YUY2: {
            break;
        }default: {
            return false;
        }
    }

    std::uint32_t numberOfMainImages = 1;
    if (head.caps & DDS::DDS_CAPS_COMPLEX) {
        if (head.caps2 & DDS::DDS_CAPS2_CUBEMAP) {//cubemap
            //note: in skybox dds files, especially in gimp, layer order is as follows: right,left,top,bottom,front,back
            if (head.caps2 & DDS::DDS_CAPS2_CUBEMAP_POSITIVEX) { ++numberOfMainImages; }//right
            if (head.caps2 & DDS::DDS_CAPS2_CUBEMAP_NEGATIVEX) { ++numberOfMainImages; }//left
            if (head.caps2 & DDS::DDS_CAPS2_CUBEMAP_POSITIVEY) { ++numberOfMainImages; }//top
            if (head.caps2 & DDS::DDS_CAPS2_CUBEMAP_NEGATIVEY) { ++numberOfMainImages; }//bottom
            if (head.caps2 & DDS::DDS_CAPS2_CUBEMAP_POSITIVEZ) { ++numberOfMainImages; }//front
            if (head.caps2 & DDS::DDS_CAPS2_CUBEMAP_NEGATIVEZ) { ++numberOfMainImages; }//back
            cpuData.m_TextureType     = TextureType::CubeMap;
            cpuData.m_IsToBeMipmapped = false;
            --numberOfMainImages;
        }
    }
    image_loaded_struct.m_PixelFormat = ImagePixelFormat::RGBA;
    image_loaded_struct.m_PixelType   = ImagePixelType::UNSIGNED_BYTE;
    std::uint32_t width_              = head.w;
    std::uint32_t height_             = head.h;
    for (std::uint32_t i = 0; i < numberOfMainImages; ++i) {
        ImageLoadedStructure* imgPtr = nullptr;
        if (i == 0) {
            imgPtr = &image_loaded_struct;
        }else if (i >= cpuData.m_ImagesDatas.size()) {
            imgPtr                   = &cpuData.m_ImagesDatas.emplace_back();
            imgPtr->m_PixelFormat    = image_loaded_struct.m_PixelFormat;
            imgPtr->m_PixelType      = image_loaded_struct.m_PixelType;
            imgPtr->m_InternalFormat = image_loaded_struct.m_InternalFormat;
        }else{
            imgPtr = &cpuData.m_ImagesDatas[i];
        }
        width_  = head.w;
        height_ = head.h;
        for (std::uint32_t level = 0; level < head.mipMapCount && (width_ || height_); ++level) {
            if (level > 0 && (width_ < 64 || height_ < 64)) {
                break;
            }

            ImageMipmap* mipmap = nullptr;
            if (level > 0) {
                mipmap = &imgPtr->m_Mipmaps.emplace_back();
            }else{
                mipmap = &imgPtr->m_Mipmaps[0];
            }
            mipmap->level                       = level;
            mipmap->width                       = width_;
            mipmap->height                      = height_;
            const std::uint32_t compressed_size = ((width_ + 3U) / 4U) * ((height_ + 3U) / 4U) * blockSize;
            mipmap->compressedSize              = compressed_size;

            auto& pixels                        = mipmap->pixels;
            pixels.reserve(compressed_size);
            for (std::uint32_t t = 0; t < compressed_size; ++t) {
                pixels.emplace_back( file_data[offset + t] );
            }
            width_                              = std::max(width_ / 2U, 1U);
            height_                             = std::max(height_ / 2U, 1U);
            offset                             += compressed_size;
        }
    }
    return true;
}
void TextureLoader::LoadTexture2DIntoOpenGL(Texture& texture) {
    Engine::Renderer::bindTextureForModification(texture.m_CPUData.m_TextureType, texture.m_TextureAddress);
    for (auto& mipmap : texture.m_CPUData.m_ImagesDatas[0].m_Mipmaps) {
        TextureLoader::ImportIntoOpengl(texture, mipmap, texture.m_CPUData.m_TextureType);
        //TextureLoader::WithdrawPixelsFromOpenGLMemory(texture, 0, mipmap.level);
    }
    texture.setFilter(TextureFilter::Linear);
    texture.setWrapping(TextureWrap::Repeat);
}
void TextureLoader::LoadTextureFramebufferIntoOpenGL(Texture& texture) {
    Engine::Renderer::bindTextureForModification(texture.m_CPUData.m_TextureType, texture.m_TextureAddress);
    const auto& image = texture.m_CPUData.m_ImagesDatas[0];
    GLCall(glTexImage2D(
        texture.m_CPUData.m_TextureType.toGLType(),
        0, 
        (GLint)image.m_InternalFormat, 
        image.m_Mipmaps[0].width, 
        image.m_Mipmaps[0].height, 
        0, 
        (GLenum)image.m_PixelFormat, 
        (GLenum)image.m_PixelType, 
        NULL)
    );
    texture.setFilter(TextureFilter::Linear);
    texture.setWrapping(TextureWrap::ClampToEdge);
}
void TextureLoader::LoadTextureCubemapIntoOpenGL(Texture& texture) {
    Engine::Renderer::bindTextureForModification(texture.m_CPUData.m_TextureType, texture.m_TextureAddress);
    std::uint32_t imageIndex = 0;
    for (auto& image : texture.m_CPUData.m_ImagesDatas) {
        for (auto& mipmap : image.m_Mipmaps) {
            TextureLoader::ImportIntoOpengl(texture, mipmap, static_cast<TextureType>((std::uint32_t)TextureType::CubeMap_X_Pos + imageIndex));
        }
        ++imageIndex;
    }
    texture.setFilter(TextureFilter::Linear);
    texture.setWrapping(TextureWrap::ClampToEdge);
}
void TextureLoader::WithdrawPixelsFromOpenGLMemory(Texture& texture, unsigned int imageIndex, unsigned int mipmapLevel) {
    auto& image = texture.m_CPUData.m_ImagesDatas[imageIndex];
    auto& pxls  = image.m_Mipmaps[mipmapLevel].pixels;
    pxls.clear();
    pxls.resize(image.m_Mipmaps[mipmapLevel].width * image.m_Mipmaps[mipmapLevel].height * 4);
    Engine::Renderer::bindTextureForModification(texture.m_CPUData.m_TextureType, texture.m_TextureAddress);
    GLCall(glGetTexImage(texture.m_CPUData.m_TextureType.toGLType(), 0, (GLenum)image.m_PixelFormat, (GLenum)image.m_PixelType, &pxls[0]));
}
void TextureLoader::GenerateMipmapsOpenGL(Texture& texture) {
    if (texture.m_CPUData.m_Mipmapped) {
        return;
    }
    Engine::Renderer::bindTextureForModification(texture.m_CPUData.m_TextureType, texture.m_TextureAddress);
    GLCall(glTexParameteri(texture.m_CPUData.m_TextureType.toGLType(), GL_TEXTURE_BASE_LEVEL, 0));
    if (texture.m_CPUData.m_MinFilter == GL_LINEAR) {
        texture.m_CPUData.m_MinFilter = GL_LINEAR_MIPMAP_LINEAR;
    }else if (texture.m_CPUData.m_MinFilter == GL_NEAREST) {
        texture.m_CPUData.m_MinFilter = GL_NEAREST_MIPMAP_NEAREST;
    }
    GLCall(glTexParameteri(texture.m_CPUData.m_TextureType.toGLType(), GL_TEXTURE_MIN_FILTER, texture.m_CPUData.m_MinFilter));
    GLCall(glGenerateMipmap(texture.m_CPUData.m_TextureType.toGLType()));
    texture.m_CPUData.m_Mipmapped = true;
}
void TextureLoader::GeneratePBRData(Texture& texture, int convoludeTextureSize, int preEnvFilterSize) {
    auto cubemapConvolution            = Engine::priv::Core::m_Engine->m_ResourceManager.m_ResourceModule.get<Texture>(texture.name() + "Convolution");
    auto cubemapPreEnvFilter           = Engine::priv::Core::m_Engine->m_ResourceManager.m_ResourceModule.get<Texture>(texture.name() + "PreEnvFilter");
    texture.m_ConvolutionTextureHandle = cubemapConvolution.second;
    texture.m_PreEnvTextureHandle      = cubemapPreEnvFilter.second;
    if (!cubemapConvolution.first) {
        texture.m_ConvolutionTextureHandle = Engine::Resources::addResource<Texture>(
            texture.name() + "Convolution", TextureType::CubeMap, convoludeTextureSize, convoludeTextureSize, false
        );
        cubemapConvolution.first = texture.m_ConvolutionTextureHandle.get<Texture>();
        Engine::Renderer::genAndBindTexture(cubemapConvolution.first->getTextureType(), cubemapConvolution.first->internal_get_address_for_generation());
        for (unsigned int i = 0; i < 6; ++i) {
            GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, convoludeTextureSize, convoludeTextureSize, 0, GL_RGB, GL_FLOAT, NULL));
        }
        cubemapConvolution.first->setWrapping(TextureWrap::ClampToEdge);
        cubemapConvolution.first->setFilter(TextureFilter::Linear);
    }
    if (!cubemapPreEnvFilter.first) {
        texture.m_PreEnvTextureHandle = Engine::Resources::addResource<Texture>(
            texture.name() + "PreEnvFilter", TextureType::CubeMap, preEnvFilterSize, preEnvFilterSize, true
        );
        cubemapPreEnvFilter.first = texture.m_PreEnvTextureHandle.get<Texture>();
        Engine::Renderer::genAndBindTexture(cubemapPreEnvFilter.first->getTextureType(), cubemapPreEnvFilter.first->internal_get_address_for_generation());
        for (unsigned int i = 0; i < 6; ++i) {
            GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, preEnvFilterSize, preEnvFilterSize, 0, GL_RGB, GL_FLOAT, NULL));
        }
        cubemapPreEnvFilter.first->setWrapping(TextureWrap::ClampToEdge);
        cubemapPreEnvFilter.first->setMinFilter(TextureFilter::Linear_Mipmap_Linear);
        cubemapPreEnvFilter.first->setMaxFilter(TextureFilter::Linear);
    }

    Core::m_Engine->m_RenderManager._genPBREnvMapData(texture, texture.m_ConvolutionTextureHandle, texture.m_PreEnvTextureHandle, convoludeTextureSize, preEnvFilterSize);
}
void InternalTexturePublicInterface::LoadCPU(Handle textureHandle) {
    auto& texture = *textureHandle.get<Texture>();
    LoadCPU(texture);
}
void InternalTexturePublicInterface::LoadGPU(Handle textureHandle) {
    auto& texture = *textureHandle.get<Texture>();
    LoadGPU(texture);
}
void InternalTexturePublicInterface::UnloadCPU(Handle textureHandle) {
    auto& texture = *textureHandle.get<Texture>();
    UnloadCPU(texture);
}
void InternalTexturePublicInterface::UnloadGPU(Handle textureHandle) {
    auto& texture = *textureHandle.get<Texture>();
    UnloadGPU(texture);
}
void InternalTexturePublicInterface::Load(Handle textureHandle) {
    auto& texture = *textureHandle.get<Texture>();
    Load(texture);
}
void InternalTexturePublicInterface::Unload(Handle textureHandle) {
    auto& texture = *textureHandle.get<Texture>();
    Unload(texture);
}
void InternalTexturePublicInterface::LoadCPU(Texture& texture) {
    texture.setName(texture.m_CPUData.m_Name);
    for (auto& image : texture.m_CPUData.m_ImagesDatas) {
        if (!image.m_Filename.empty()) {
            bool do_ = false;
            if (image.m_Mipmaps.size() == 0) {
                do_ = true;
            }
            if (!do_) {
                for (auto& mip : image.m_Mipmaps) {
                    if (mip.pixels.size() == 0) {
                        do_ = true;
                        break;
                    }
                }
            }
            if (do_) {
                const std::string& extension = boost::filesystem::extension(image.m_Filename);
                if (extension == ".dds") {
                    TextureLoader::LoadDDSFile(texture.m_CPUData, image.m_Filename, image);
                }else{
                    sf::Image sfImage;
                    sfImage.loadFromFile(image.m_Filename);
                    image.load(sfImage, image.m_Filename);
                }
            }
        }
    }
}
void InternalTexturePublicInterface::LoadGPU(Texture& texture) {
    Engine::Renderer::genAndBindTexture(texture.m_CPUData.m_TextureType, texture.m_TextureAddress);
    switch ((TextureType::Type)texture.m_CPUData.m_TextureType) {
        case TextureType::RenderTarget: {
            TextureLoader::LoadTextureFramebufferIntoOpenGL(texture);
            break;
        }case TextureType::Texture1D: {
            break;
        }case TextureType::Texture2D: {
            TextureLoader::LoadTexture2DIntoOpenGL(texture);
            break;
        }case TextureType::Texture3D: {
            break;
        }case TextureType::CubeMap: {
            TextureLoader::LoadTextureCubemapIntoOpenGL(texture);
            break;
        }default: {
            break;
        }
    }
    if (texture.m_CPUData.m_IsToBeMipmapped) {
        TextureLoader::GenerateMipmapsOpenGL(texture);
    }
    while (texture.m_CommandQueue.size() > 0) {
        texture.m_CommandQueue.front()();
        texture.m_CommandQueue.pop();
    }
    texture.Resource::load();
}
void InternalTexturePublicInterface::UnloadCPU(Texture& texture) {
    for (auto& image : texture.m_CPUData.m_ImagesDatas) {
        if (!image.m_Filename.empty()) {
            for (auto& mipmap : image.m_Mipmaps) {
                if (mipmap.pixels.size() == 0) {
                    mipmap.pixels.shrink_to_fit();
                }
            }
        }
    }
    texture.m_CPUData.m_Mipmapped = false;
    texture.Resource::unload();
}
void InternalTexturePublicInterface::UnloadGPU(Texture& texture) {
    glDeleteTextures(1, &texture.m_TextureAddress);
}
void InternalTexturePublicInterface::Load(Texture& texture) {
    if (!texture.isLoaded()) {
        InternalTexturePublicInterface::LoadCPU(texture);
        InternalTexturePublicInterface::LoadGPU(texture);
    }
}
void InternalTexturePublicInterface::Unload(Texture& texture) {
    if (texture.isLoaded()) {
        InternalTexturePublicInterface::UnloadGPU(texture);
        InternalTexturePublicInterface::UnloadCPU(texture);
    }
}


void InternalTexturePublicInterface::Resize(Texture& texture, Engine::priv::FramebufferObject& fbo, int width, int height) {
    if (texture.m_CPUData.m_TextureType != TextureType::RenderTarget) {
        ENGINE_PRODUCTION_LOG("Error: Non-framebuffer texture cannot be resized. Returning...")
        return;
    }
    const float divisor           = fbo.divisor();
    Engine::Renderer::bindTextureForModification(texture.m_CPUData.m_TextureType, texture.m_TextureAddress);
    const int w                   = (int)((float)width * divisor);
    const int h                   = (int)((float)height * divisor);
    auto& imageData               = texture.m_CPUData.m_ImagesDatas[0];
    imageData.m_Mipmaps[0].width  = w;
    imageData.m_Mipmaps[0].height = h;

    GLCall(glTexImage2D(texture.m_CPUData.m_TextureType.toGLType(), 0, (GLint)imageData.m_InternalFormat, w, h, 0, (GLenum)imageData.m_PixelFormat, (GLenum)imageData.m_PixelType, NULL));
}