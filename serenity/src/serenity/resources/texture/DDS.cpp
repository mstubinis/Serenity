#include <serenity/resources/texture/DDS.h>
#include <serenity/utils/Utils.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/resources/texture/TextureCubemap.h>

#include <fstream>

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
            t = a[2];
            a[2] = a[0];
            a[0] = t;
            a += 4;
        }
    }
	bool load_dds_file(auto& cpuData, Engine::priv::ImageData* inImageData) {
        std::ifstream stream(inImageData->m_Filename.c_str(), std::ios::binary);
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

        Engine::priv::DDS::DDS_Header head{ header_buffer };
        if (head.magic != Engine::priv::DDS::DDS_MAGIC_NUMBER) {  //checks if this is "DDS "
            return false;
        }
        //DX10 header here
        Engine::priv::DDS::DDS_Header_DX10 headDX10;
        if ((head.header_flags & Engine::priv::DDS::DDPF_FOURCC) && head.format.fourCC == Engine::priv::FourCC_DX10) {
            std::array<uint8_t, 20> header_buffer_DX10;
            for (size_t i = 0; i < header_buffer_DX10.size(); ++i) {
                header_buffer_DX10[i] = file_data[i + progress];
                ++progress;
            }
            headDX10.fill(header_buffer_DX10);
        }
        [[maybe_unused]] uint32_t factor;
        uint32_t blockSizeInBytes;
        uint32_t offset = progress;
        //TODO: fill the rest of these out
        switch (head.format.fourCC) {
            case Engine::priv::FourCC_DXT1: { //aka BC1
                factor = 2;
                blockSizeInBytes = 8;
                inImageData->m_InternalFormat = ImageInternalFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT;
                break;
            } case Engine::priv::FourCC_DXT2: {
                factor = 4;
                blockSizeInBytes = 16;
                break;
            } case Engine::priv::FourCC_DXT3: { //aka BC2
                factor = 4;
                blockSizeInBytes = 16;
                inImageData->m_InternalFormat = ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
                break;
            } case Engine::priv::FourCC_DXT4: {
                factor = 4;
                blockSizeInBytes = 16;
                break;
            } case Engine::priv::FourCC_DXT5: { //aka BC3
                factor = 4;
                blockSizeInBytes = 16;
                inImageData->m_InternalFormat = ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
                break;
            } case Engine::priv::FourCC_DX10: {
                break;
            } case Engine::priv::FourCC_ATI1: { //aka BC4 useful for 1 channel textures (greyscales, glow / specular / ao / smoothness / metalness etc)
                factor = 2;
                blockSizeInBytes = 8;
                inImageData->m_InternalFormat = ImageInternalFormat::COMPRESSED_RED_RGTC1;
                break;
            } case Engine::priv::FourCC_ATI2: {//aka ATI2n aka 3Dc aka LATC2 aka BC5 - used for normal maps (store x,y recalc z) z = sqrt( 1-x*x-y*y )
                blockSizeInBytes = 16;
                inImageData->m_InternalFormat = ImageInternalFormat::COMPRESSED_RG_RGTC2;
                break;
            } case Engine::priv::FourCC_RXGB: { //By its design, it is just a DXT5 image with reading it in the shader differently
                //As I recall, the most you would have to do in the shader is something like:
                //vec3 normal;
                //normal.xy = texture2D(RXGBnormalmap, texcoord).ag;
                //normal.z = sqrt(1.0 - normal.x * normal.x - normal.y * normal.y);
                factor = 4;
                blockSizeInBytes = 16;
                inImageData->m_InternalFormat = ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT;
                break;
            } case Engine::priv::FourCC_$: {
                break;
            } case Engine::priv::FourCC_o: {
                break;
            } case Engine::priv::FourCC_p: {
                break;
            } case Engine::priv::FourCC_q: {
                break;
            } case Engine::priv::FourCC_r: {
                break;
            } case Engine::priv::FourCC_s: {
                break;
            } case Engine::priv::FourCC_t: {
                break;
            } case Engine::priv::FourCC_BC4U: {
                break;
            } case Engine::priv::FourCC_BC4S: {
                break;
            } case Engine::priv::FourCC_BC5U: {
                break;
            } case Engine::priv::FourCC_BC5S: {
                break;
            } case Engine::priv::FourCC_RGBG: {
                break;
            } case Engine::priv::FourCC_GRGB: {
                break;
            } case Engine::priv::FourCC_YUY2: {
                break;
            } default: {
                ENGINE_LOG(__FUNCTION__ << "(): could not evalutate switch statement for head.format.fourCC!");
                return false;
            }
        }
        uint32_t numberOfMainImages = 1;
        bool isCubeMap = (head.caps & Engine::priv::DDS::DDS_CAPS_COMPLEX) && (head.caps2 & Engine::priv::DDS::DDS_CAPS2_CUBEMAP);
        if (isCubeMap) { //cubemap
            //note: in skybox dds files, especially in gimp, layer order is as follows: right,left,top,bottom,front,back
            if (head.caps2 & Engine::priv::DDS::DDS_CAPS2_CUBEMAP_POSITIVEX) { ++numberOfMainImages; }//right
            if (head.caps2 & Engine::priv::DDS::DDS_CAPS2_CUBEMAP_NEGATIVEX) { ++numberOfMainImages; }//left
            if (head.caps2 & Engine::priv::DDS::DDS_CAPS2_CUBEMAP_POSITIVEY) { ++numberOfMainImages; }//top
            if (head.caps2 & Engine::priv::DDS::DDS_CAPS2_CUBEMAP_NEGATIVEY) { ++numberOfMainImages; }//bottom
            if (head.caps2 & Engine::priv::DDS::DDS_CAPS2_CUBEMAP_POSITIVEZ) { ++numberOfMainImages; }//front
            if (head.caps2 & Engine::priv::DDS::DDS_CAPS2_CUBEMAP_NEGATIVEZ) { ++numberOfMainImages; }//back
            //cpuData.m_TextureType = TextureType::CubeMap;
            cpuData.m_IsToBeMipmapped = false;
            --numberOfMainImages;
        }
        if (isCubeMap && numberOfMainImages < 6) {
            ENGINE_LOG(__FUNCTION__ << "(): warning - cubemap has less than 6 textures!");
        }
        if (numberOfMainImages == 0) {
            return false;
        }
        inImageData->m_PixelFormat = ImagePixelFormat::RGBA;
        inImageData->m_PixelType   = ImagePixelType::UNSIGNED_BYTE;
        uint32_t width_  = head.w;
        uint32_t height_ = head.h;
        for (uint32_t imgIdx = 0; imgIdx != numberOfMainImages; ++imgIdx) {
            if (imgIdx >= cpuData.m_ImagesDatas.size()) {
                ImagePixelFormat pxl    = inImageData->m_PixelFormat;
                ImagePixelType pxlType  = inImageData->m_PixelType;
                ImageInternalFormat fmt = inImageData->m_InternalFormat;
                //image_loaded_struct is now no longer valid after next line
                inImageData = &cpuData.m_ImagesDatas.emplace_back();
                inImageData->m_PixelFormat    = pxl;
                inImageData->m_PixelType      = pxlType;
                inImageData->m_InternalFormat = fmt;
            } else {
                inImageData = &cpuData.m_ImagesDatas[imgIdx];
            }
            width_ = head.w;
            height_ = head.h;
            for (uint32_t mipmapLevel = 0; mipmapLevel < head.mipMapCount && (width_ || height_); ++mipmapLevel) {
                if (mipmapLevel > 0 && (width_ < 64 || height_ < 64)) {
                    break;
                }
                Engine::priv::ImageMipmap* mipmap = nullptr;
                if (mipmapLevel > 0) {
                    mipmap = &inImageData->m_Mipmaps.emplace_back();
                } else {
                    mipmap = &inImageData->m_Mipmaps[0];
                }
                mipmap->level = mipmapLevel;
                mipmap->width = width_;
                mipmap->height = height_;
                const uint32_t compressed_size = ((width_ + 3U) / 4U) * ((height_ + 3U) / 4U) * blockSizeInBytes;
                const uint32_t widthBytes = ((width_ + 3U) / 4U) * blockSizeInBytes;
                mipmap->compressedSize = compressed_size;

                auto& mipmapPixels = mipmap->pixels;
                mipmapPixels.resize(compressed_size);

                std::copy(&file_data[offset], &file_data[offset] + compressed_size, mipmapPixels.data());

                //bool width_pow_2 = (width_ > 0 && ((width_ & (width_ - 1)) == 0));
                //bool height_pow_2 = (height_ > 0 && ((height_ & (height_ - 1)) == 0));

                //mirror pixels vertically
                if(!isCubeMap && height_ >= 2) {
                    std::vector<uint8_t> temp = mipmapPixels;
                    uint8_t* src  = temp.data();
                    uint8_t* dst  = mipmapPixels.data() + (compressed_size - widthBytes);
                    auto flip_pixels_vertically = [widthBytes, blockSizeInBytes](uint8_t* d, auto func) {
                        auto loopEnd = widthBytes / blockSizeInBytes;
                        for (uint32_t column = 0; column != loopEnd; ++column) {
                            func(d + column * blockSizeInBytes);
                        }
                    };
                    auto loopEnd = ((height_ + 3) / 4);
                    for (size_t row = 0; row != loopEnd; ++row) {
                        std::copy(src, src + widthBytes, dst);
                        if (head.format.fourCC == Engine::priv::FourCC_ATI1) {
                            flip_pixels_vertically(dst, flip_blocks_ati1_aka_bc4);
                        } else if (head.format.fourCC == Engine::priv::FourCC_DXT1) {
                            flip_pixels_vertically(dst, flip_block_dxt1_aka_bc1);
                        } else if (head.format.fourCC == Engine::priv::FourCC_DXT3) {
                            flip_pixels_vertically(dst, flip_block_dxt3_aka_bc2);
                        } else if (head.format.fourCC == Engine::priv::FourCC_ATI2) {
                            flip_pixels_vertically(dst, flip_blocks_ati2_aka_bc5);
                        } else if (head.format.fourCC == Engine::priv::FourCC_DXT5) {
                            flip_pixels_vertically(dst, flip_block_dxt5_aka_bc3);
                        }
                        src += widthBytes;
                        dst -= widthBytes;
                    }
                    //end mirror pixels vertically
                }
                width_ = std::max(width_ / 2U, 1U);
                height_ = std::max(height_ / 2U, 1U);
                offset += compressed_size;
            }
        }
        return true;
	}
}


bool Engine::priv::LoadDDSFile(TextureCPUData& cpuData, ImageData& imgData) {
    return load_dds_file(cpuData, &imgData);
}
bool Engine::priv::LoadDDSFile(TextureCubemapCPUData& cpuData, ImageData& imgData) {
    return load_dds_file(cpuData, &imgData);
}