#pragma once
#ifndef ENGINE_TEXTURES_DDS_INCLUDE_GUARD
#define ENGINE_TEXTURES_DDS_INCLUDE_GUARD

#include <core/engine/utils/Utils.h>

namespace Engine {
namespace epriv {
namespace textures {
    class DXGI_FORMAT { public: enum Format {
        DXGI_FORMAT_UNKNOWN,
        DXGI_FORMAT_R32G32B32A32_TYPELESS,
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT_R32G32B32A32_UINT,
        DXGI_FORMAT_R32G32B32A32_SINT,
        DXGI_FORMAT_R32G32B32_TYPELESS,
        DXGI_FORMAT_R32G32B32_FLOAT,
        DXGI_FORMAT_R32G32B32_UINT,
        DXGI_FORMAT_R32G32B32_SINT,
        DXGI_FORMAT_R16G16B16A16_TYPELESS,
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        DXGI_FORMAT_R16G16B16A16_UNORM,
        DXGI_FORMAT_R16G16B16A16_UINT,
        DXGI_FORMAT_R16G16B16A16_SNORM,
        DXGI_FORMAT_R16G16B16A16_SINT,
        DXGI_FORMAT_R32G32_TYPELESS,
        DXGI_FORMAT_R32G32_FLOAT,
        DXGI_FORMAT_R32G32_UINT,
        DXGI_FORMAT_R32G32_SINT,
        DXGI_FORMAT_R32G8X24_TYPELESS,
        DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
        DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS,
        DXGI_FORMAT_X32_TYPELESS_G8X24_UINT,
        DXGI_FORMAT_R10G10B10A2_TYPELESS,
        DXGI_FORMAT_R10G10B10A2_UNORM,
        DXGI_FORMAT_R10G10B10A2_UINT,
        DXGI_FORMAT_R11G11B10_FLOAT,
        DXGI_FORMAT_R8G8B8A8_TYPELESS,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        DXGI_FORMAT_R8G8B8A8_UINT,
        DXGI_FORMAT_R8G8B8A8_SNORM,
        DXGI_FORMAT_R8G8B8A8_SINT,
        DXGI_FORMAT_R16G16_TYPELESS,
        DXGI_FORMAT_R16G16_FLOAT,
        DXGI_FORMAT_R16G16_UNORM,
        DXGI_FORMAT_R16G16_UINT,
        DXGI_FORMAT_R16G16_SNORM,
        DXGI_FORMAT_R16G16_SINT,
        DXGI_FORMAT_R32_TYPELESS,
        DXGI_FORMAT_D32_FLOAT,
        DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_R32_UINT,
        DXGI_FORMAT_R32_SINT,
        DXGI_FORMAT_R24G8_TYPELESS,
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
        DXGI_FORMAT_X24_TYPELESS_G8_UINT,
        DXGI_FORMAT_R8G8_TYPELESS,
        DXGI_FORMAT_R8G8_UNORM,
        DXGI_FORMAT_R8G8_UINT,
        DXGI_FORMAT_R8G8_SNORM,
        DXGI_FORMAT_R8G8_SINT,
        DXGI_FORMAT_R16_TYPELESS,
        DXGI_FORMAT_R16_FLOAT,
        DXGI_FORMAT_D16_UNORM,
        DXGI_FORMAT_R16_UNORM,
        DXGI_FORMAT_R16_UINT,
        DXGI_FORMAT_R16_SNORM,
        DXGI_FORMAT_R16_SINT,
        DXGI_FORMAT_R8_TYPELESS,
        DXGI_FORMAT_R8_UNORM,
        DXGI_FORMAT_R8_UINT,
        DXGI_FORMAT_R8_SNORM,
        DXGI_FORMAT_R8_SINT,
        DXGI_FORMAT_A8_UNORM,
        DXGI_FORMAT_R1_UNORM,
        DXGI_FORMAT_R9G9B9E5_SHAREDEXP,
        DXGI_FORMAT_R8G8_B8G8_UNORM,
        DXGI_FORMAT_G8R8_G8B8_UNORM,
        DXGI_FORMAT_BC1_TYPELESS,
        DXGI_FORMAT_BC1_UNORM,
        DXGI_FORMAT_BC1_UNORM_SRGB,
        DXGI_FORMAT_BC2_TYPELESS,
        DXGI_FORMAT_BC2_UNORM,
        DXGI_FORMAT_BC2_UNORM_SRGB,
        DXGI_FORMAT_BC3_TYPELESS,
        DXGI_FORMAT_BC3_UNORM,
        DXGI_FORMAT_BC3_UNORM_SRGB,
        DXGI_FORMAT_BC4_TYPELESS,
        DXGI_FORMAT_BC4_UNORM,
        DXGI_FORMAT_BC4_SNORM,
        DXGI_FORMAT_BC5_TYPELESS,
        DXGI_FORMAT_BC5_UNORM,
        DXGI_FORMAT_BC5_SNORM,
        DXGI_FORMAT_B5G6R5_UNORM,
        DXGI_FORMAT_B5G5R5A1_UNORM,
        DXGI_FORMAT_B8G8R8A8_UNORM,
        DXGI_FORMAT_B8G8R8X8_UNORM,
        DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM,
        DXGI_FORMAT_B8G8R8A8_TYPELESS,
        DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
        DXGI_FORMAT_B8G8R8X8_TYPELESS,
        DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,
        DXGI_FORMAT_BC6H_TYPELESS,
        DXGI_FORMAT_BC6H_UF16,
        DXGI_FORMAT_BC6H_SF16,
        DXGI_FORMAT_BC7_TYPELESS,
        DXGI_FORMAT_BC7_UNORM,
        DXGI_FORMAT_BC7_UNORM_SRGB,
        DXGI_FORMAT_AYUV,
        DXGI_FORMAT_Y410,
        DXGI_FORMAT_Y416,
        DXGI_FORMAT_NV12,
        DXGI_FORMAT_P010,
        DXGI_FORMAT_P016,
        DXGI_FORMAT_420_OPAQUE,
        DXGI_FORMAT_YUY2,
        DXGI_FORMAT_Y210,
        DXGI_FORMAT_Y216,
        DXGI_FORMAT_NV11,
        DXGI_FORMAT_AI44,
        DXGI_FORMAT_IA44,
        DXGI_FORMAT_P8,
        DXGI_FORMAT_A8P8,
        DXGI_FORMAT_B4G4R4A4_UNORM,
        DXGI_FORMAT_P208,
        DXGI_FORMAT_V208,
        DXGI_FORMAT_V408,
        DXGI_FORMAT_FORCE_UINT,
        _TOTAL,
    };};
    class D3D_RESOURCE_DIMENSION { public: enum Dimension {
        D3D_RESOURCE_DIMENSION_UNKNOWN,
        D3D_RESOURCE_DIMENSION_BUFFER,
        D3D_RESOURCE_DIMENSION_TEXTURE1D,
        D3D_RESOURCE_DIMENSION_TEXTURE2D,
        D3D_RESOURCE_DIMENSION_TEXTURE3D,
        _TOTAL,
    };};
    static const uint32_t FourCC_DXT1 = 0x31545844;
    static const uint32_t FourCC_DXT2 = 0x32545844;
    static const uint32_t FourCC_DXT3 = 0x33545844;
    static const uint32_t FourCC_DXT4 = 0x34545844;
    static const uint32_t FourCC_DXT5 = 0x35545844;
    static const uint32_t FourCC_DX10 = 0x30315844;
    static const uint32_t FourCC_ATI1 = 0x31495441;
    static const uint32_t FourCC_ATI2 = 0x32495441;
    static const uint32_t FourCC_RXGB = 0x42475852;
    static const uint32_t FourCC_$    = 0x00000024;
    static const uint32_t FourCC_o    = 0x0000006f;
    static const uint32_t FourCC_p    = 0x00000070;
    static const uint32_t FourCC_q    = 0x00000071;
    static const uint32_t FourCC_r    = 0x00000072;
    static const uint32_t FourCC_s    = 0x00000073;
    static const uint32_t FourCC_t    = 0x00000074;
    static const uint32_t FourCC_BC4U = 0x55344342;
    static const uint32_t FourCC_BC4S = 0x53344342;
    static const uint32_t FourCC_BC5U = 0x55354342;
    static const uint32_t FourCC_BC5S = 0x53354342;
    static const uint32_t FourCC_RGBG = 0x47424752;
    static const uint32_t FourCC_GRGB = 0x42475247;
    static const uint32_t FourCC_YUY2 = 0x32595559;

namespace DDS {
    struct DDS_PixelFormat final {
        uint32_t pxl_size, pxl_flags;
        uint32_t fourCC;
        uint32_t BitCountRGB; //Number of bits in an RGB (possibly including alpha) format. Valid when flags includes DDPF_RGB, DDPF_LUMINANCE, or DDPF_YUV.
        uint32_t BitMaskR, BitMaskG, BitMaskB, BitMaskA;
        DDS_PixelFormat() {}
        DDS_PixelFormat(uint32_t _size, uint32_t _flags, uint32_t _fourCC, uint32_t _bitCRGB, uint32_t _mskR, uint32_t _mskG, uint32_t _mskB, uint32_t _mskA) {
            pxl_size    = _size;
            pxl_flags   = _flags;
            fourCC      = _fourCC;
            BitCountRGB = _bitCRGB;
            BitMaskR    = _mskR;
            BitMaskG    = _mskG;
            BitMaskB    = _mskB;
            BitMaskA    = _mskA;
        }
        void fill(unsigned char _header[128]) {
            pxl_size    = *(uint32_t*)&_header[76];
            pxl_flags   = *(uint32_t*)&_header[80];
            fourCC      = *(uint32_t*)&_header[84];
            BitCountRGB = *(uint32_t*)&_header[88];
            BitMaskR    = *(uint32_t*)&_header[92];
            BitMaskG    = *(uint32_t*)&_header[96];
            BitMaskB    = *(uint32_t*)&_header[100];
            BitMaskA    = *(uint32_t*)&_header[104];
        }
        DDS_PixelFormat(unsigned char _header[128]) { fill(_header); }
        ~DDS_PixelFormat() {}
    };
    struct DDS_Header_DX10 final {
        textures::DXGI_FORMAT::Format               dxgiFormat;
        textures::D3D_RESOURCE_DIMENSION::Dimension resourceDimension;
        uint32_t miscFlag, arraySize, miscFlags2;
        DDS_Header_DX10() {}
        void fill(unsigned char _headerDX10[20]) {
            dxgiFormat        = (textures::DXGI_FORMAT::Format)(*(uint32_t*)&_headerDX10[0]);
            resourceDimension = (textures::D3D_RESOURCE_DIMENSION::Dimension)(*(uint32_t*)&_headerDX10[4]);
            miscFlag          = *(uint32_t*)&_headerDX10[8];
            arraySize         = *(uint32_t*)&_headerDX10[12];
            miscFlags2        = *(uint32_t*)&_headerDX10[16];
        }
        ~DDS_Header_DX10() {}
    };
    struct DDS_Header final {
        uint32_t magic, header_size;
        uint32_t header_flags;//if this is DDPF_FOURCC then we include the DDS_HEADER_DXT10
        uint32_t h, w;
        uint32_t pitchOrlinearSize;//The pitch or number of bytes per scan line in an uncompressed texture; the total number of bytes in the top level texture for a compressed texture.
        uint32_t depth;
        uint32_t mipMapCount;
        //uint32_t reserved1[11];
        DDS_PixelFormat format;
        uint32_t caps, caps2, caps3, caps4;
        uint32_t reserved2;
        DDS_Header() {}
        DDS_Header(unsigned char _header[128]) {
            magic             = *(uint32_t*)&_header[0];
            header_size       = *(uint32_t*)&_header[4];
            header_flags      = *(uint32_t*)&_header[8];
            h                 = *(uint32_t*)&_header[12];
            w                 = *(uint32_t*)&_header[16];
            pitchOrlinearSize = *(uint32_t*)&_header[20];
            depth             = *(uint32_t*)&_header[24];
            mipMapCount       = *(uint32_t*)&_header[28];

            //reserved1[11]   = *(uint32_t*)&_header[32-72];
            format.fill(_header);

            caps              = *(uint32_t*)&_header[108];
            caps2             = *(uint32_t*)&_header[112];
            caps3             = *(uint32_t*)&_header[116];
            caps4             = *(uint32_t*)&_header[120];
            reserved2         = *(uint32_t*)&_header[124];
        }
        ~DDS_Header() {}
    };

    static const uint32_t DDS_CAPS                      = 0x00000001;
    static const uint32_t DDS_HEIGHT                    = 0x00000002;
    static const uint32_t DDS_WIDTH                     = 0x00000004;
    static const uint32_t DDS_PITCH                     = 0x00000008;
    static const uint32_t DDS_PIXELFORMAT               = 0x00001000;
    static const uint32_t DDS_MIPMAPCOUNT               = 0x00020000;
    static const uint32_t DDS_LINEARSIZE                = 0x00080000;
    static const uint32_t DDS_DEPTH                     = 0x00800000;
    static const uint32_t DDS_HEADER_FLAGS_TEXTURE      = 0x00001007;
    static const uint32_t DDS_CAPS_COMPLEX              = 0x00000008;
    static const uint32_t DDS_CAPS_MIPMAP               = 0x00400000;
    static const uint32_t DDS_CAPS_TEXTURE              = 0x00001000;
    static const uint32_t DDS_SURFACE_FLAGS_MIPMAP      = 0x00400008;
    static const uint32_t DDS_CAPS2_CUBEMAP             = 0x00000200;
    static const uint32_t DDS_CAPS2_CUBEMAP_POSITIVEX   = 0x00000400;
    static const uint32_t DDS_CAPS2_CUBEMAP_NEGATIVEX   = 0x00000800;
    static const uint32_t DDS_CAPS2_CUBEMAP_POSITIVEY   = 0x00001000;
    static const uint32_t DDS_CAPS2_CUBEMAP_NEGATIVEY   = 0x00002000;
    static const uint32_t DDS_CAPS2_CUBEMAP_POSITIVEZ   = 0x00004000;
    static const uint32_t DDS_CAPS2_CUBEMAP_NEGATIVEZ   = 0x00008000;
    static const uint32_t DDS_CAPS2_VOLUME              = 0x00200000;

    static const uint32_t DDS_CUBEMAP_POSITIVEX         = 0x00000600;
    static const uint32_t DDS_CUBEMAP_NEGATIVEX         = 0x00000a00;
    static const uint32_t DDS_CUBEMAP_POSITIVEY         = 0x00001200;
    static const uint32_t DDS_CUBEMAP_NEGATIVEY         = 0x00002200;
    static const uint32_t DDS_CUBEMAP_POSITIVEZ         = 0x00004200;
    static const uint32_t DDS_CUBEMAP_NEGATIVEZ         = 0x00008200;
    static const uint32_t DDS_CUBEMAP_ALLFACES          = 0x0000fe00;

    static const uint32_t DDS_RESOURCE_MISC_TEXTURECUBE = 0x00000004;
    static const uint32_t DDS_ALPHA_MODE_UNKNOWN        = 0x00000000;
    static const uint32_t DDS_ALPHA_MODE_STRAIGHT       = 0x00000001;
    static const uint32_t DDS_ALPHA_MODE_PREMULTIPLIED  = 0x00000002;
    static const uint32_t DDS_ALPHA_MODE_OPAQUE         = 0x00000003;
    static const uint32_t DDS_ALPHA_MODE_CUSTOM         = 0x00000004;
    static const uint32_t DDPF_ALPHAPIXELS              = 0x00000001;
    static const uint32_t DDPF_ALPHA                    = 0x00000002;
    static const uint32_t DDPF_FOURCC                   = 0x00000004;
    static const uint32_t DDPF_RGB                      = 0x00000040;
    static const uint32_t DDPF_YUV                      = 0x00000200;
    static const uint32_t DDPF_LUMINANCE                = 0x00020000;
    static const uint32_t DDS_RGBA                      = 0x00000041;
    static const uint32_t DDS_LUMINANCEA                = 0x00020001;
    static const uint32_t DDPF_BUMPDUDV                 = 0x00080000;

    static const DDS_PixelFormat DDSPF_DXT1 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_DXT1, 0, 0, 0, 0, 0);
    static const DDS_PixelFormat DDSPF_DXT2 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_DXT2, 0, 0, 0, 0, 0);
    static const DDS_PixelFormat DDSPF_DXT3 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_DXT3, 0, 0, 0, 0, 0);
    static const DDS_PixelFormat DDSPF_DXT4 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_DXT4, 0, 0, 0, 0, 0);
    static const DDS_PixelFormat DDSPF_DXT5 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_DXT5, 0, 0, 0, 0, 0);
    static const DDS_PixelFormat DDSPF_BC4_UNORM = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_BC4U, 0, 0, 0, 0, 0);
    static const DDS_PixelFormat DDSPF_BC4_SNORM = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_BC4S, 0, 0, 0, 0, 0);
    static const DDS_PixelFormat DDSPF_BC5_UNORM = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_BC5U, 0, 0, 0, 0, 0);
    static const DDS_PixelFormat DDSPF_BC5_SNORM = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_BC5S, 0, 0, 0, 0, 0);
    static const DDS_PixelFormat DDSPF_R8G8_B8G8 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_RGBG, 0, 0, 0, 0, 0);
    static const DDS_PixelFormat DDSPF_G8R8_G8B8 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_GRGB, 0, 0, 0, 0, 0);
    static const DDS_PixelFormat DDSPF_YUY2 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_YUY2, 0, 0, 0, 0, 0);
    static const DDS_PixelFormat DDSPF_A8R8G8B8 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDS_RGBA, 0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    static const DDS_PixelFormat DDSPF_X8R8G8B8 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_RGB, 0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000);
    static const DDS_PixelFormat DDSPF_A8B8G8R8 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDS_RGBA, 0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    static const DDS_PixelFormat DDSPF_X8B8G8R8 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_RGB, 0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000);
    static const DDS_PixelFormat DDSPF_G16R16 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_RGB, 0, 32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000);
    static const DDS_PixelFormat DDSPF_R5G6B5 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_RGB, 0, 16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000);
    static const DDS_PixelFormat DDSPF_A1R5G5B5 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDS_RGBA, 0, 16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00008000);
    static const DDS_PixelFormat DDSPF_A4R4G4B4 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDS_RGBA, 0, 16, 0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000);
    static const DDS_PixelFormat DDSPF_R8G8B8 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_RGB, 0, 24, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000);
    static const DDS_PixelFormat DDSPF_L8 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_LUMINANCE, 0, 8, 0xff, 0x00, 0x00, 0x00);
    static const DDS_PixelFormat DDSPF_L16 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_LUMINANCE, 0, 16, 0xffff, 0x0000, 0x0000, 0x0000);
    static const DDS_PixelFormat DDSPF_A8L8 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDS_LUMINANCEA, 0, 16, 0x00ff, 0x0000, 0x0000, 0xff00);
    static const DDS_PixelFormat DDSPF_A8L8_ALT = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDS_LUMINANCEA, 0, 8, 0x00ff, 0x0000, 0x0000, 0xff00);
    static const DDS_PixelFormat DDSPF_A8 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_ALPHA, 0, 8, 0x00, 0x00, 0x00, 0xff);
    static const DDS_PixelFormat DDSPF_V8U8 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_BUMPDUDV, 0, 16, 0x00ff, 0xff00, 0x0000, 0x0000);
    static const DDS_PixelFormat DDSPF_Q8W8V8U8 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_BUMPDUDV, 0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    static const DDS_PixelFormat DDSPF_V16U16 = 
        DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_BUMPDUDV, 0, 32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000);
};
};
};
};

#endif