
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

namespace {
    void flip_block_dxt1_aka_bc1(uint8_t* data) {
        std::swap(data[4], data[7]);
        std::swap(data[5], data[6]);
    }
    void flip_block_dxt3_aka_bc2(uint8_t* block) {
        std::swap(block[0], block[6]);
        std::swap(block[1], block[7]);
        std::swap(block[2], block[4]);
        std::swap(block[3], block[5]);
        flip_block_dxt1_aka_bc1(block + 8);
    }
    void flip_block_alpha(uint8_t* block) {
        uint32_t line_0_1 = (block[4] << 16) | (block[3] << 8) | block[2];
        uint32_t line_2_3 = (block[7] << 16) | (block[6] << 8) | block[5];
        uint32_t line_1_0 = ((line_0_1 & 0b0000'0000'0000'0000'0000'1111'1111'1111) << 12) | ((line_0_1 & 0b0000'0000'1111'1111'1111'0000'0000'0000) >> 12);
        uint32_t line_3_2 = ((line_2_3 & 0b0000'0000'0000'0000'0000'1111'1111'1111) << 12) | ((line_2_3 & 0b0000'0000'1111'1111'1111'0000'0000'0000) >> 12);

        //24 bits
        block[2] = (line_3_2 & 0b0000'0000'0000'0000'1111'1111);
        block[3] = (line_3_2 & 0b0000'0000'1111'1111'0000'0000) >> 8;
        block[4] = (line_3_2 & 0b1111'1111'0000'0000'0000'0000) >> 16;

        //24 bits
        block[5] = (line_1_0 & 0b0000'0000'0000'0000'1111'1111);
        block[6] = (line_1_0 & 0b0000'0000'1111'1111'0000'0000) >> 8;
        block[7] = (line_1_0 & 0b1111'1111'0000'0000'0000'0000) >> 16;
    }
    void flip_block_dxt5_aka_bc3(uint8_t* block) {
        flip_block_alpha(block);
        flip_block_dxt1_aka_bc1(block + 8);
    }
    void flip_blocks_ati1_aka_bc4(uint8_t* block) {
        flip_block_alpha(block);
    }
    void flip_blocks_ati2_aka_bc5(uint8_t* block) {
        flip_block_alpha(block);
        flip_block_alpha(block + 8);
    }
    void ConvertARGB2RGBA(uint8_t* a, int n) {
        int i;
        uint8_t t;
        n /= 4;
        for (i = 0; i < n; i++) {
            t     = a[2];
            a[2]  = a[0];
            a[0]  = t;
            a    += 4;
        }
    }
}

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
        case FourCC_DXT1: { //aka BC1
            factor    = 2;
            blockSize = 8;
            image_loaded_struct.m_InternalFormat = ImageInternalFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT;
            break;
        } case FourCC_DXT2: {
            factor    = 4;
            blockSize = 16;
            break;
        } case FourCC_DXT3: { //aka BC2
            factor    = 4;
            blockSize = 16;
            image_loaded_struct.m_InternalFormat = ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
            break;
        } case FourCC_DXT4: {
            factor    = 4;
            blockSize = 16;
            break;
        } case FourCC_DXT5: { //aka BC3
            factor    = 4;
            blockSize = 16;
            image_loaded_struct.m_InternalFormat = ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
            break;
        } case FourCC_DX10: {
            break;
        } case FourCC_ATI1: { //aka BC4 useful for 1 channel textures (greyscales, glow / specular / ao / smoothness / metalness etc)
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
            const uint32_t widBytes        = ((width_ + 3U) / 4U) * blockSize;
            mipmap->compressedSize         = compressed_size;

            auto& pixels                   = mipmap->pixels;
            pixels.resize(compressed_size);

            std::copy(&file_data[offset], &file_data[offset + compressed_size - 1], pixels.data());

            //mirror pixels vertically
            std::vector<uint8_t> temp = pixels;
            uint8_t* s = temp.data();
            uint8_t* d = &pixels[i] + ((height_ + 3) / 4 - 1) * widBytes;
            auto flip_pixels_vertically = [widBytes, blockSize](uint8_t* d, auto func) {
                for (uint32_t k = 0; k < widBytes / blockSize; k++) {
                    func((uint8_t*)d + k * blockSize);
                }
            };
            for (size_t j = 0; j < (height_ + 3) / 4; j++) {
                std::copy(s, s + widBytes - 1, d);
                if (head.format.fourCC == FourCC_ATI1) {
                    flip_pixels_vertically(d, flip_blocks_ati1_aka_bc4);
                } else if(head.format.fourCC == FourCC_DXT1){
                    flip_pixels_vertically(d, flip_block_dxt1_aka_bc1);
                } else if (head.format.fourCC == FourCC_DXT3) {
                    flip_pixels_vertically(d, flip_block_dxt3_aka_bc2);
                } else if (head.format.fourCC == FourCC_ATI2) {
                    flip_pixels_vertically(d, flip_blocks_ati2_aka_bc5);
                } else if(head.format.fourCC == FourCC_DXT5) {
                    flip_pixels_vertically(d, flip_block_dxt5_aka_bc3);
                }       
                s += widBytes;
                d -= widBytes;
            }
            //end mirror pixels vertically
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
void TextureLoader::MirrorPixelsVertically(uint8_t* pixels, uint32_t width, uint32_t height, uint32_t colorsPerPixel) {
    auto swapPixels = [width, pixels, colorsPerPixel](uint32_t x, uint32_t y, uint32_t x2, uint32_t y2) {
        std::array<uint8_t, 4> temp;
        auto getIdx = [width, colorsPerPixel](uint32_t x, uint32_t y) {
            return ((x * width) + y) * colorsPerPixel;
        };
        for (uint32_t i = 0; i < colorsPerPixel; ++i) {
            temp[i] = pixels[getIdx(x, y) + i];
        }
        for (uint32_t i = 0; i < colorsPerPixel; ++i) {
            pixels[getIdx(x, y) + i] = pixels[getIdx(x2, y2) + i];
        }
        for (uint32_t i = 0; i < colorsPerPixel; ++i) {
            pixels[getIdx(x2, y2) + i] = temp[i];
        }
    };
    //mirror pixels along y-axis. this is a sfml needed thing
    for (size_t row = 0; row < height / 2; ++row) {
        for (size_t col = 0; col < width; ++col) {
            swapPixels(row, col, height - row - 1, col);
        }
    }
}
void TextureLoader::LoadCPU(TextureCPUData& cpuData, Handle inHandle) {
    for (auto& imageData : cpuData.m_ImagesDatas) {
        if (!imageData.m_Filename.empty()) {
            if (imageData.hasBlankMipmap()) {
                const std::string extension = std::filesystem::path(imageData.m_Filename).extension().string();
                if (extension == ".dds") {
                    TextureLoader::LoadDDSFile(cpuData, imageData);
                } else {
                    sfImageLoader sfImage(imageData.m_Filename.c_str());
                    imageData.load(sfImage.getPixels(), sfImage.getWidth(), sfImage.getHeight(), imageData.m_Filename);
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