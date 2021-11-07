
#include <serenity/resources/texture/Texture.h>
#include <serenity/resources/texture/TextureCubemap.h>
#include <serenity/resources/texture/TextureLoader.h>
#include <serenity/resources/texture/DDS.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/system/Engine.h>
#include <serenity/renderer/FramebufferObject.h>
#include <serenity/events/Event.h>
#include <serenity/resources/texture/TextureRequest.h>

#include <SFML/Graphics.hpp>
#include <fstream>
#include <filesystem>

using namespace Engine;
using namespace Engine::priv;
using namespace Engine::priv::textures;


void TextureLoader::ImportIntoOpengl(Texture& texture, const Engine::priv::ImageMipmap& mipmap, TextureType textureType) {
    auto& imageData = texture.m_CPUData.m_ImagesDatas[0];
    if (imageData.m_InternalFormat.isCompressedType() && mipmap.compressedSize != 0) {
        glCompressedTexImage2D(textureType.toGLType(), mipmap.level, imageData.m_InternalFormat, mipmap.width, mipmap.height, 0, mipmap.compressedSize, &mipmap.pixels[0]);
    } else {
        glTexImage2D(textureType.toGLType(), mipmap.level, imageData.m_InternalFormat, mipmap.width, mipmap.height, 0, imageData.m_PixelFormat, imageData.m_PixelType, &mipmap.pixels[0]);
    }
}
bool TextureLoader::LoadDDSFile(TextureCPUData& cpuData, ImageData& image_loaded_struct) {
    std::ifstream stream(image_loaded_struct.m_Filename.c_str(), std::ios::binary);
    if (!stream) {
        return false;
    }
    std::streampos fileSize;
    stream.unsetf(std::ios::skipws);
    stream.seekg(0, std::ios::end);
    fileSize = stream.tellg();
    stream.seekg(0, std::ios::beg);

    auto file_data = Engine::create_and_reserve<std::vector<uint8_t>>(fileSize);

    file_data.insert(std::begin(file_data), std::istream_iterator<uint8_t>(stream), std::istream_iterator<uint8_t>());
    stream.close();

    std::array<uint8_t, 128> header_buffer;
    uint32_t progress = 0;
    for (size_t i = 0; i < header_buffer.size(); ++i) {
        header_buffer[i] = file_data[i];
        ++progress;
    }

    DDS::DDS_Header head{ header_buffer };
    if (head.magic != DDS::DDS_MAGIC_NUMBER) {  //checks if this is "DDS "
        return false; 
    }
    //DX10 header here
    DDS::DDS_Header_DX10 headDX10;
    if ((head.header_flags & DDS::DDPF_FOURCC) && head.format.fourCC == FourCC_DX10) {
        std::array<uint8_t, 20> header_buffer_DX10;
        for (size_t i = 0; i < header_buffer_DX10.size(); ++i) {
            header_buffer_DX10[i] = file_data[i + progress];
            ++progress;
        }
        headDX10.fill(header_buffer_DX10);
    }
    uint32_t factor, blockSize, offset = progress;
    //TODO: fill the rest of these out
    switch (head.format.fourCC) {
        case FourCC_DXT1: {
            factor    = 2;
            blockSize = 8;
            image_loaded_struct.m_InternalFormat = ImageInternalFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT;
            break;
        } case FourCC_DXT2: {
            factor    = 4;
            blockSize = 16;
            break;
        } case FourCC_DXT3: {
            factor    = 4;
            blockSize = 16;
            image_loaded_struct.m_InternalFormat = ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
            break;
        } case FourCC_DXT4: {
            factor    = 4;
            blockSize = 16;
            break;
        } case FourCC_DXT5: {
            factor    = 4;
            blockSize = 16;
            image_loaded_struct.m_InternalFormat = ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
            break;
        } case FourCC_DX10: {
            break;
        } case FourCC_ATI1: { //useful for 1 channel textures (greyscales, glow / specular / ao / smoothness / metalness etc)
            factor    = 2;
            blockSize = 8;
            image_loaded_struct.m_InternalFormat = ImageInternalFormat::COMPRESSED_RED_RGTC1;
            break;
        } case FourCC_ATI2: {//aka ATI2n aka 3Dc aka LATC2 aka BC5 - used for normal maps (store x,y recalc z) z = sqrt( 1-x*x-y*y )
            blockSize = 16;
            image_loaded_struct.m_InternalFormat = ImageInternalFormat::COMPRESSED_RG_RGTC2;
            break;
        } case FourCC_RXGB: { //By its design, it is just a DXT5 image with reading it in the shader differently
            //As I recall, the most you would have to do in the shader is something like:
            //vec3 normal;
            //normal.xy = texture2D(RXGBnormalmap, texcoord).ag;
            //normal.z = sqrt(1.0 - normal.x * normal.x - normal.y * normal.y);
            factor    = 4;
            blockSize = 16;
            image_loaded_struct.m_InternalFormat = ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT;
            break;
        } case FourCC_$: {
            break;
        } case FourCC_o: {
            break;
        } case FourCC_p: {
            break;
        } case FourCC_q: {
            break;
        } case FourCC_r: {
            break;
        } case FourCC_s: {
            break;
        } case FourCC_t: {
            break;
        } case FourCC_BC4U: {
            break;
        } case FourCC_BC4S: {
            break;
        } case FourCC_BC5U: {
            break;
        } case FourCC_BC5S: {
            break;
        } case FourCC_RGBG: {
            break;
        } case FourCC_GRGB: {
            break;
        } case FourCC_YUY2: {
            break;
        } default: {
            ENGINE_PRODUCTION_LOG(__FUNCTION__ << "(): could not evalutate switch statement for head.format.fourCC!")
            return false;
        }
    }

    uint32_t numberOfMainImages = 1;
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
    uint32_t width_                   = head.w;
    uint32_t height_                  = head.h;
    for (uint32_t i = 0; i < numberOfMainImages; ++i) {
        ImageData* imgPtr = nullptr;
        if (i == 0) {
            imgPtr = &image_loaded_struct;
        } else if (i >= cpuData.m_ImagesDatas.size()) {
            imgPtr                    = &cpuData.m_ImagesDatas.emplace_back();
            imgPtr->m_PixelFormat     = image_loaded_struct.m_PixelFormat;
            imgPtr->m_PixelType       = image_loaded_struct.m_PixelType;
            imgPtr->m_InternalFormat  = image_loaded_struct.m_InternalFormat;
        } else {
            imgPtr = &cpuData.m_ImagesDatas[i];
        }
        width_  = head.w;
        height_ = head.h;
        for (uint32_t level = 0; level < head.mipMapCount && (width_ || height_); ++level) {
            if (level > 0 && (width_ < 64 || height_ < 64)) {
                break;
            }
            ImageMipmap* mipmap = nullptr;
            if (level > 0) {
                mipmap = &imgPtr->m_Mipmaps.emplace_back();
            } else {
                mipmap = &imgPtr->m_Mipmaps[0];
            }
            mipmap->level                  = level;
            mipmap->width                  = width_;
            mipmap->height                 = height_;
            const uint32_t compressed_size = ((width_ + 3U) / 4U) * ((height_ + 3U) / 4U) * blockSize;
            mipmap->compressedSize         = compressed_size;

            auto& pixels                   = mipmap->pixels;
            pixels.reserve(compressed_size);
            for (uint32_t t = 0; t < compressed_size; ++t) {
                pixels.emplace_back( file_data[offset + t] );
            }
            width_                         = std::max(width_ / 2U, 1U);
            height_                        = std::max(height_ / 2U, 1U);
            offset                        += compressed_size;
        }
    }
    return true;
}
void TextureLoader::LoadTexture2DIntoOpenGL(Texture& texture) {
    Engine::Renderer::bindTextureForModification(texture.m_CPUData.m_TextureType, texture.m_TextureAddress);
    for (const auto& mipmap : texture.m_CPUData.m_ImagesDatas[0].m_Mipmaps) {
        TextureLoader::ImportIntoOpengl(texture, mipmap, texture.m_CPUData.m_TextureType);
        //TextureLoader::WithdrawPixelsFromOpenGLMemory(texture, 0, mipmap.level);
    }
    texture.setFilter(TextureFilter::Linear);
    texture.setWrapping(TextureWrap::Repeat);
}
void TextureLoader::LoadTextureFramebufferIntoOpenGL(Texture& texture) {
    Engine::Renderer::bindTextureForModification(texture.m_CPUData.m_TextureType, texture.m_TextureAddress);
    const auto& image = texture.m_CPUData.m_ImagesDatas[0];
    glTexImage2D(texture.m_CPUData.m_TextureType.toGLType(), 0, image.m_InternalFormat, image.m_Mipmaps[0].width, image.m_Mipmaps[0].height, 0, image.m_PixelFormat, image.m_PixelType, nullptr);
    texture.setFilter(TextureFilter::Linear);
    texture.setWrapping(TextureWrap::ClampToEdge);
}
void TextureLoader::LoadTextureCubemapIntoOpenGL(Texture& texture) {
    Engine::Renderer::bindTextureForModification(texture.m_CPUData.m_TextureType, texture.m_TextureAddress);
    uint32_t imageIndex = 0;
    for (const auto& image : texture.m_CPUData.m_ImagesDatas) {
        for (const auto& mipmap : image.m_Mipmaps) {
            TextureLoader::ImportIntoOpengl(texture, mipmap, TextureType::CubeMap_X_Pos + imageIndex);
        }
        ++imageIndex;
    }
    texture.setFilter(TextureFilter::Linear);
    texture.setWrapping(TextureWrap::ClampToEdge);
}
void TextureLoader::WithdrawPixelsFromOpenGLMemory(Texture& texture, uint32_t imageIndex, uint32_t mipmapLevel) {
    auto& image = texture.m_CPUData.m_ImagesDatas[imageIndex];
    auto& pxls  = image.m_Mipmaps[mipmapLevel].pixels;
    pxls.clear();
    pxls.resize(image.m_Mipmaps[mipmapLevel].width * image.m_Mipmaps[mipmapLevel].height * 4);
    Engine::Renderer::bindTextureForModification(texture.m_CPUData.m_TextureType, texture.m_TextureAddress);
    glGetTexImage(texture.m_CPUData.m_TextureType.toGLType(), 0, image.m_PixelFormat, image.m_PixelType, &pxls[0]);
}
bool TextureLoader::GenerateMipmapsOpenGL(Texture& texture) {
    if (texture.m_CPUData.m_Mipmapped || texture.m_TextureAddress == 0) {
        return false;
    }
    Engine::Renderer::bindTextureForModification(texture.m_CPUData.m_TextureType, texture.m_TextureAddress);
    glTexParameteri(texture.m_CPUData.m_TextureType.toGLType(), GL_TEXTURE_BASE_LEVEL, 0);
    if (texture.m_CPUData.m_MinFilter == GL_LINEAR) {
        texture.m_CPUData.m_MinFilter = GL_LINEAR_MIPMAP_LINEAR;
    } else if (texture.m_CPUData.m_MinFilter == GL_NEAREST) {
        texture.m_CPUData.m_MinFilter = GL_NEAREST_MIPMAP_NEAREST;
    }
    glTexParameteri(texture.m_CPUData.m_TextureType.toGLType(), GL_TEXTURE_MIN_FILTER, texture.m_CPUData.m_MinFilter);
    glGenerateMipmap(texture.m_CPUData.m_TextureType.toGLType());
    texture.m_CPUData.m_Mipmapped = true;
    return true;
}
void TextureLoader::LoadCPU(TextureCPUData& cpuData, Handle inHandle) {
    for (auto& imageData : cpuData.m_ImagesDatas) {
        if (!imageData.m_Filename.empty()) {
            if (imageData.hasBlankMipmap()) {
                const std::string extension = std::filesystem::path(imageData.m_Filename).extension().string();
                if (extension == ".dds") {
                    TextureLoader::LoadDDSFile(cpuData, imageData);
                } else {
                    sf::Image sfImage;
                    sfImage.loadFromFile(imageData.m_Filename);
                    imageData.load(sfImage, imageData.m_Filename);
                }
            }
        }
    }
    if (!inHandle.null()) {
        std::lock_guard lock(*inHandle.getMutex());
        inHandle.get<Texture>()->m_CPUData = std::move(cpuData);
    }
}
void TextureLoader::LoadGPU(Handle textureHandle) {
    std::lock_guard lock{ Engine::Resources::getMutex() };
    LoadGPU(*textureHandle.get<Texture>());
}
void TextureLoader::LoadGPU(Texture& texture) {
    Engine::Renderer::genAndBindTexture(texture.m_CPUData.m_TextureType, texture.m_TextureAddress);
    switch (texture.m_CPUData.m_TextureType) {
        case TextureType::RenderTarget: {
            TextureLoader::LoadTextureFramebufferIntoOpenGL(texture);
            break;
        } case TextureType::Texture1D: {
            break;
        } case TextureType::Texture2D: {
            TextureLoader::LoadTexture2DIntoOpenGL(texture);
            break;
        } case TextureType::Texture3D: {
            break;
        } case TextureType::CubeMap: {
            TextureLoader::LoadTextureCubemapIntoOpenGL(texture);
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

void TextureLoader::UnloadGPU(Texture& texture) {
    glDeleteTextures(1, &texture.m_TextureAddress);
    texture.Resource::unload();
}
void TextureLoader::Load(Texture& texture) {
    if (!texture.isLoaded()) {
        TextureLoader::LoadCPU(texture.m_CPUData, Handle{});
        TextureLoader::LoadGPU(texture);
    }
}
void TextureLoader::Unload(Texture& texture) {
    if (texture.isLoaded()) {
        TextureLoader::UnloadGPU(texture);
    }
}


void TextureLoader::Resize(Texture& texture, Engine::priv::FramebufferObject& fbo, int width, int height) {
    if (texture.m_CPUData.m_TextureType != TextureType::RenderTarget) {
        ENGINE_PRODUCTION_LOG(__FUNCTION__ << "() error: Non-framebuffer texture cannot be resized. Returning...")
        return;
    }
    const float divisor           = fbo.divisor();
    Engine::Renderer::bindTextureForModification(texture.m_CPUData.m_TextureType, texture.m_TextureAddress);
    const int w                   = int(float(width) * divisor);
    const int h                   = int(float(height) * divisor);
    auto& imageData               = texture.m_CPUData.m_ImagesDatas[0];
    imageData.m_Mipmaps[0].width  = w;
    imageData.m_Mipmaps[0].height = h;

    glTexImage2D(texture.m_CPUData.m_TextureType.toGLType(), 0, imageData.m_InternalFormat, w, h, 0, imageData.m_PixelFormat, imageData.m_PixelType, NULL);
}