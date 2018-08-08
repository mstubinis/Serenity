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
using namespace Engine::epriv;
using namespace Engine::epriv::textures;
using namespace std;

#define FOURCC(a,b,c,d)((uint32_t) (((d)<<24) | ((c)<<16) | ((b)<<8) | (a)))
string fourcc(uint32_t e){char c[5]={'\0'};c[0]=e>>0 & 0xFF;c[1]=e>>8 & 0xFF;c[2]=e>>16 & 0xFF;c[3]=e>>24 & 0xFF;return c;}

namespace Engine{
    namespace epriv{
        namespace textures{

            typedef enum DXGI_FORMAT {
              DXGI_FORMAT_UNKNOWN                     ,
              DXGI_FORMAT_R32G32B32A32_TYPELESS       ,
              DXGI_FORMAT_R32G32B32A32_FLOAT          ,
              DXGI_FORMAT_R32G32B32A32_UINT           ,
              DXGI_FORMAT_R32G32B32A32_SINT           ,
              DXGI_FORMAT_R32G32B32_TYPELESS          ,
              DXGI_FORMAT_R32G32B32_FLOAT             ,
              DXGI_FORMAT_R32G32B32_UINT              ,
              DXGI_FORMAT_R32G32B32_SINT              ,
              DXGI_FORMAT_R16G16B16A16_TYPELESS       ,
              DXGI_FORMAT_R16G16B16A16_FLOAT          ,
              DXGI_FORMAT_R16G16B16A16_UNORM          ,
              DXGI_FORMAT_R16G16B16A16_UINT           ,
              DXGI_FORMAT_R16G16B16A16_SNORM          ,
              DXGI_FORMAT_R16G16B16A16_SINT           ,
              DXGI_FORMAT_R32G32_TYPELESS             ,
              DXGI_FORMAT_R32G32_FLOAT                ,
              DXGI_FORMAT_R32G32_UINT                 ,
              DXGI_FORMAT_R32G32_SINT                 ,
              DXGI_FORMAT_R32G8X24_TYPELESS           ,
              DXGI_FORMAT_D32_FLOAT_S8X24_UINT        ,
              DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS    ,
              DXGI_FORMAT_X32_TYPELESS_G8X24_UINT     ,
              DXGI_FORMAT_R10G10B10A2_TYPELESS        ,
              DXGI_FORMAT_R10G10B10A2_UNORM           ,
              DXGI_FORMAT_R10G10B10A2_UINT            ,
              DXGI_FORMAT_R11G11B10_FLOAT             ,
              DXGI_FORMAT_R8G8B8A8_TYPELESS           ,
              DXGI_FORMAT_R8G8B8A8_UNORM              ,
              DXGI_FORMAT_R8G8B8A8_UNORM_SRGB         ,
              DXGI_FORMAT_R8G8B8A8_UINT               ,
              DXGI_FORMAT_R8G8B8A8_SNORM              ,
              DXGI_FORMAT_R8G8B8A8_SINT               ,
              DXGI_FORMAT_R16G16_TYPELESS             ,
              DXGI_FORMAT_R16G16_FLOAT                ,
              DXGI_FORMAT_R16G16_UNORM                ,
              DXGI_FORMAT_R16G16_UINT                 ,
              DXGI_FORMAT_R16G16_SNORM                ,
              DXGI_FORMAT_R16G16_SINT                 ,
              DXGI_FORMAT_R32_TYPELESS                ,
              DXGI_FORMAT_D32_FLOAT                   ,
              DXGI_FORMAT_R32_FLOAT                   ,
              DXGI_FORMAT_R32_UINT                    ,
              DXGI_FORMAT_R32_SINT                    ,
              DXGI_FORMAT_R24G8_TYPELESS              ,
              DXGI_FORMAT_D24_UNORM_S8_UINT           ,
              DXGI_FORMAT_R24_UNORM_X8_TYPELESS       ,
              DXGI_FORMAT_X24_TYPELESS_G8_UINT        ,
              DXGI_FORMAT_R8G8_TYPELESS               ,
              DXGI_FORMAT_R8G8_UNORM                  ,
              DXGI_FORMAT_R8G8_UINT                   ,
              DXGI_FORMAT_R8G8_SNORM                  ,
              DXGI_FORMAT_R8G8_SINT                   ,
              DXGI_FORMAT_R16_TYPELESS                ,
              DXGI_FORMAT_R16_FLOAT                   ,
              DXGI_FORMAT_D16_UNORM                   ,
              DXGI_FORMAT_R16_UNORM                   ,
              DXGI_FORMAT_R16_UINT                    ,
              DXGI_FORMAT_R16_SNORM                   ,
              DXGI_FORMAT_R16_SINT                    ,
              DXGI_FORMAT_R8_TYPELESS                 ,
              DXGI_FORMAT_R8_UNORM                    ,
              DXGI_FORMAT_R8_UINT                     ,
              DXGI_FORMAT_R8_SNORM                    ,
              DXGI_FORMAT_R8_SINT                     ,
              DXGI_FORMAT_A8_UNORM                    ,
              DXGI_FORMAT_R1_UNORM                    ,
              DXGI_FORMAT_R9G9B9E5_SHAREDEXP          ,
              DXGI_FORMAT_R8G8_B8G8_UNORM             ,
              DXGI_FORMAT_G8R8_G8B8_UNORM             ,
              DXGI_FORMAT_BC1_TYPELESS                ,
              DXGI_FORMAT_BC1_UNORM                   ,
              DXGI_FORMAT_BC1_UNORM_SRGB              ,
              DXGI_FORMAT_BC2_TYPELESS                ,
              DXGI_FORMAT_BC2_UNORM                   ,
              DXGI_FORMAT_BC2_UNORM_SRGB              ,
              DXGI_FORMAT_BC3_TYPELESS                ,
              DXGI_FORMAT_BC3_UNORM                   ,
              DXGI_FORMAT_BC3_UNORM_SRGB              ,
              DXGI_FORMAT_BC4_TYPELESS                ,
              DXGI_FORMAT_BC4_UNORM                   ,
              DXGI_FORMAT_BC4_SNORM                   ,
              DXGI_FORMAT_BC5_TYPELESS                ,
              DXGI_FORMAT_BC5_UNORM                   ,
              DXGI_FORMAT_BC5_SNORM                   ,
              DXGI_FORMAT_B5G6R5_UNORM                ,
              DXGI_FORMAT_B5G5R5A1_UNORM              ,
              DXGI_FORMAT_B8G8R8A8_UNORM              ,
              DXGI_FORMAT_B8G8R8X8_UNORM              ,
              DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM  ,
              DXGI_FORMAT_B8G8R8A8_TYPELESS           ,
              DXGI_FORMAT_B8G8R8A8_UNORM_SRGB         ,
              DXGI_FORMAT_B8G8R8X8_TYPELESS           ,
              DXGI_FORMAT_B8G8R8X8_UNORM_SRGB         ,
              DXGI_FORMAT_BC6H_TYPELESS               ,
              DXGI_FORMAT_BC6H_UF16                   ,
              DXGI_FORMAT_BC6H_SF16                   ,
              DXGI_FORMAT_BC7_TYPELESS                ,
              DXGI_FORMAT_BC7_UNORM                   ,
              DXGI_FORMAT_BC7_UNORM_SRGB              ,
              DXGI_FORMAT_AYUV                        ,
              DXGI_FORMAT_Y410                        ,
              DXGI_FORMAT_Y416                        ,
              DXGI_FORMAT_NV12                        ,
              DXGI_FORMAT_P010                        ,
              DXGI_FORMAT_P016                        ,
              DXGI_FORMAT_420_OPAQUE                  ,
              DXGI_FORMAT_YUY2                        ,
              DXGI_FORMAT_Y210                        ,
              DXGI_FORMAT_Y216                        ,
              DXGI_FORMAT_NV11                        ,
              DXGI_FORMAT_AI44                        ,
              DXGI_FORMAT_IA44                        ,
              DXGI_FORMAT_P8                          ,
              DXGI_FORMAT_A8P8                        ,
              DXGI_FORMAT_B4G4R4A4_UNORM              ,
              DXGI_FORMAT_P208                        ,
              DXGI_FORMAT_V208                        ,
              DXGI_FORMAT_V408                        ,
              DXGI_FORMAT_FORCE_UINT
            } ;
            typedef enum D3D_RESOURCE_DIMENSION {
              D3D_RESOURCE_DIMENSION_UNKNOWN    ,
              D3D_RESOURCE_DIMENSION_BUFFER     ,
              D3D_RESOURCE_DIMENSION_TEXTURE1D  ,
              D3D_RESOURCE_DIMENSION_TEXTURE2D  ,
              D3D_RESOURCE_DIMENSION_TEXTURE3D
            } ;

            static const uint FourCC_DXT1    = 0x31545844;
            static const uint FourCC_DXT2    = 0x32545844;
            static const uint FourCC_DXT3    = 0x33545844;
            static const uint FourCC_DXT4    = 0x34545844;
            static const uint FourCC_DXT5    = 0x35545844;
            static const uint FourCC_DX10    = 0x30315844;
            static const uint FourCC_ATI1    = 0x31495441;
            static const uint FourCC_ATI2    = 0x32495441;
            static const uint FourCC_RXGB    = 0x42475852;
            static const uint FourCC_$       = 0x00000024;
            static const uint FourCC_o       = 0x0000006f;
            static const uint FourCC_p       = 0x00000070;
            static const uint FourCC_q       = 0x00000071;
            static const uint FourCC_r       = 0x00000072;
            static const uint FourCC_s       = 0x00000073;
            static const uint FourCC_t       = 0x00000074;
            static const uint FourCC_BC4U    = 0x55344342;
            static const uint FourCC_BC4S    = 0x53344342;
            static const uint FourCC_BC5U    = 0x55354342;
            static const uint FourCC_BC5S    = 0x53354342;
            static const uint FourCC_RGBG    = 0x47424752;
            static const uint FourCC_GRGB    = 0x42475247;
            static const uint FourCC_YUY2    = 0x32595559;

            namespace DDS{
                struct DDS_PixelFormat final{
                    uint32_t pxl_size,pxl_flags;
                    uint32_t fourCC;
                    uint32_t BitCountRGB; //Number of bits in an RGB (possibly including alpha) format. Valid when flags includes DDPF_RGB, DDPF_LUMINANCE, or DDPF_YUV.
                    uint32_t BitMaskR,BitMaskG,BitMaskB,BitMaskA;
                    DDS_PixelFormat(){}
                    DDS_PixelFormat(uint32_t _size,uint32_t _flags,uint32_t _fourCC,uint32_t _bitCRGB,uint32_t _mskR,uint32_t _mskG,uint32_t _mskB,uint32_t _mskA){
                        pxl_size = _size;
                        pxl_flags = _flags;
                        fourCC = _fourCC;
                        BitCountRGB = _bitCRGB;
                        BitMaskR = _mskR;
                        BitMaskG = _mskG;
                        BitMaskB = _mskB;
                        BitMaskA = _mskA;
                    }
                    void fill(uchar _header[128]){
                        pxl_size           = *(uint32_t*)&_header[76];
                        pxl_flags          = *(uint32_t*)&_header[80];
                        fourCC             = *(uint32_t*)&_header[84];
                        BitCountRGB        = *(uint32_t*)&_header[88];
                        BitMaskR           = *(uint32_t*)&_header[92];
                        BitMaskG           = *(uint32_t*)&_header[96];
                        BitMaskB           = *(uint32_t*)&_header[100];
                        BitMaskA           = *(uint32_t*)&_header[104];
                    }
                    DDS_PixelFormat(uchar _header[128]){ fill(_header); }
                    ~DDS_PixelFormat(){}
                };
                struct DDS_Header_DX10 final{
                    textures::DXGI_FORMAT            dxgiFormat;
                    textures::D3D_RESOURCE_DIMENSION resourceDimension;
                    uint32_t miscFlag, arraySize, miscFlags2;
                    DDS_Header_DX10(){}
                    void fill(uchar _headerDX10[20]){
                        dxgiFormat        = (textures::DXGI_FORMAT)(*(uint32_t*)&_headerDX10[0]);
                        resourceDimension = (textures::D3D_RESOURCE_DIMENSION)(*(uint32_t*)&_headerDX10[4]);
                        miscFlag          = *(uint32_t*)&_headerDX10[8];
                        arraySize         = *(uint32_t*)&_headerDX10[12];
                        miscFlags2        = *(uint32_t*)&_headerDX10[16];
                    }
                    ~DDS_Header_DX10(){}
                };
                struct DDS_Header final{
                    uint32_t magic;
                    uint32_t header_size;
                    uint32_t header_flags;//if this is DDPF_FOURCC then we include the DDS_HEADER_DXT10
                    uint32_t h,w;
                    uint32_t pitchOrlinearSize;
                    uint32_t depth;
                    uint32_t mipMapCount;
                    //uint32_t reserved1[11];
                    DDS_PixelFormat format;
                    uint32_t caps,caps2,caps3,caps4;
                    uint32_t reserved2;
                    DDS_Header(){}
                    DDS_Header(uchar _header[128]){
                        magic              = *(uint32_t*)&_header[0];
                        header_size        = *(uint32_t*)&_header[4];
                        header_flags       = *(uint32_t*)&_header[8];
                        h                  = *(uint32_t*)&_header[12];
                        w                  = *(uint32_t*)&_header[16];
                        pitchOrlinearSize  = *(uint32_t*)&_header[20];
                        depth              = *(uint32_t*)&_header[24];
                        mipMapCount        = *(uint32_t*)&_header[28];

                        //reserved1[11]    = *(uint32_t*)&_header[32-72];
                        format.fill(_header);

                        caps               = *(uint32_t*)&_header[108];
                        caps2              = *(uint32_t*)&_header[112];
                        caps3              = *(uint32_t*)&_header[116];
                        caps4              = *(uint32_t*)&_header[120];
                        reserved2          = *(uint32_t*)&_header[124];
                    }
                    ~DDS_Header(){}
                };
                
                static const uint DDS_CAPS                    = 0x00000001;
                static const uint DDS_HEIGHT                  = 0x00000002;
                static const uint DDS_WIDTH                   = 0x00000004;
                static const uint DDS_PITCH                   = 0x00000008;
                static const uint DDS_PIXELFORMAT	          = 0x00001000;
                static const uint DDS_MIPMAPCOUNT	          = 0x00020000;
                static const uint DDS_LINEARSIZE	          = 0x00080000;
                static const uint DDS_DEPTH	                  = 0x00800000;
                static const uint DDS_HEADER_FLAGS_TEXTURE    = 0x00001007;
                static const uint DDS_CAPS_COMPLEX            = 0x00000008;
                static const uint DDS_CAPS_MIPMAP             = 0x00400000;
                static const uint DDS_CAPS_TEXTURE            = 0x00001000;
                static const uint DDS_SURFACE_FLAGS_MIPMAP    = 0x00400008;
                static const uint DDS_CAPS2_CUBEMAP	          = 0x00000200;
                static const uint DDS_CAPS2_CUBEMAP_POSITIVEX = 0x00000400;
                static const uint DDS_CAPS2_CUBEMAP_NEGATIVEX = 0x00000800;
                static const uint DDS_CAPS2_CUBEMAP_POSITIVEY = 0x00001000;
                static const uint DDS_CAPS2_CUBEMAP_NEGATIVEY = 0x00002000;
                static const uint DDS_CAPS2_CUBEMAP_POSITIVEZ = 0x00004000;
                static const uint DDS_CAPS2_CUBEMAP_NEGATIVEZ = 0x00008000;
                static const uint DDS_CAPS2_VOLUME            = 0x00200000;

                static const uint DDS_CUBEMAP_POSITIVEX       = 0x00000600;
                static const uint DDS_CUBEMAP_NEGATIVEX       = 0x00000a00;
                static const uint DDS_CUBEMAP_POSITIVEY       = 0x00001200;
                static const uint DDS_CUBEMAP_NEGATIVEY       = 0x00002200;
                static const uint DDS_CUBEMAP_POSITIVEZ       = 0x00004200;
                static const uint DDS_CUBEMAP_NEGATIVEZ       = 0x00008200;
                static const uint DDS_CUBEMAP_ALLFACES        = 0x0000fe00;

                static const uint DDS_RESOURCE_MISC_TEXTURECUBE = 0x00000004;
                static const uint DDS_ALPHA_MODE_UNKNOWN        = 0x00000000;
                static const uint DDS_ALPHA_MODE_STRAIGHT       = 0x00000001;
                static const uint DDS_ALPHA_MODE_PREMULTIPLIED  = 0x00000002;
                static const uint DDS_ALPHA_MODE_OPAQUE         = 0x00000003;
                static const uint DDS_ALPHA_MODE_CUSTOM         = 0x00000004;
                static const uint DDPF_ALPHAPIXELS              = 0x00000001;
                static const uint DDPF_ALPHA                    = 0x00000002;
                static const uint DDPF_FOURCC                   = 0x00000004;
                static const uint DDPF_RGB                      = 0x00000040;
                static const uint DDPF_YUV                      = 0x00000200;
                static const uint DDPF_LUMINANCE                = 0x00020000;
                static const uint DDS_RGBA                      = 0x00000041;
                static const uint DDS_LUMINANCEA                = 0x00020001;
                static const uint DDPF_BUMPDUDV                 = 0x00080000;

                static const DDS_PixelFormat DDSPF_DXT1 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_DXT1, 0, 0, 0, 0, 0);
                static const DDS_PixelFormat DDSPF_DXT2 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_DXT2, 0, 0, 0, 0, 0);
                static const DDS_PixelFormat DDSPF_DXT3 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_DXT3, 0, 0, 0, 0, 0);
                static const DDS_PixelFormat DDSPF_DXT4 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_DXT4, 0, 0, 0, 0, 0);
                static const DDS_PixelFormat DDSPF_DXT5 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_DXT5, 0, 0, 0, 0, 0);
                static const DDS_PixelFormat DDSPF_BC4_UNORM = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_BC4U, 0, 0, 0, 0, 0);
                static const DDS_PixelFormat DDSPF_BC4_SNORM = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_BC4S, 0, 0, 0, 0, 0);
                static const DDS_PixelFormat DDSPF_BC5_UNORM = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_BC5U, 0, 0, 0, 0, 0);
                static const DDS_PixelFormat DDSPF_BC5_SNORM = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_BC5S, 0, 0, 0, 0, 0);
                static const DDS_PixelFormat DDSPF_R8G8_B8G8 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_RGBG, 0, 0, 0, 0, 0);
                static const DDS_PixelFormat DDSPF_G8R8_G8B8 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_GRGB, 0, 0, 0, 0, 0);
                static const DDS_PixelFormat DDSPF_YUY2 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_FOURCC, FourCC_YUY2, 0, 0, 0, 0, 0);
                static const DDS_PixelFormat DDSPF_A8R8G8B8 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDS_RGBA, 0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
                static const DDS_PixelFormat DDSPF_X8R8G8B8 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_RGB,  0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000);
                static const DDS_PixelFormat DDSPF_A8B8G8R8 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDS_RGBA, 0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
                static const DDS_PixelFormat DDSPF_X8B8G8R8 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_RGB,  0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000);
                static const DDS_PixelFormat DDSPF_G16R16 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_RGB,  0, 32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000);
                static const DDS_PixelFormat DDSPF_R5G6B5 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_RGB, 0, 16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000);
                static const DDS_PixelFormat DDSPF_A1R5G5B5 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDS_RGBA, 0, 16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00008000);
                static const DDS_PixelFormat DDSPF_A4R4G4B4 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDS_RGBA, 0, 16, 0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000);
                static const DDS_PixelFormat DDSPF_R8G8B8 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_RGB, 0, 24, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000);
                static const DDS_PixelFormat DDSPF_L8 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_LUMINANCE, 0,  8, 0xff, 0x00, 0x00, 0x00);
                static const DDS_PixelFormat DDSPF_L16 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_LUMINANCE, 0, 16, 0xffff, 0x0000, 0x0000, 0x0000);
                static const DDS_PixelFormat DDSPF_A8L8 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDS_LUMINANCEA, 0, 16, 0x00ff, 0x0000, 0x0000, 0xff00);
                static const DDS_PixelFormat DDSPF_A8L8_ALT = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDS_LUMINANCEA, 0, 8, 0x00ff, 0x0000, 0x0000, 0xff00);
                static const DDS_PixelFormat DDSPF_A8 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_ALPHA, 0, 8, 0x00, 0x00, 0x00, 0xff);
                static const DDS_PixelFormat DDSPF_V8U8 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_BUMPDUDV, 0, 16, 0x00ff, 0xff00, 0x0000, 0x0000);
                static const DDS_PixelFormat DDSPF_Q8W8V8U8 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_BUMPDUDV, 0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
                static const DDS_PixelFormat DDSPF_V16U16 = DDS_PixelFormat(sizeof(DDS_PixelFormat), DDPF_BUMPDUDV, 0, 32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000);
            };
        };
    };
};

struct textures::ImageMipmap final{
    uint                          width;
    uint                          height;
    uint                          compressedSize;
    vector<uchar>                 pixels;
    uint                          level;
    ImageMipmap(){
        level = 0; width = 0; height = 0; compressedSize = 0;
    }
    ~ImageMipmap(){}
};

struct textures::ImageLoadedStructure final{
    ImageInternalFormat::Format      internalFormat;
    ImagePixelFormat::Format         pixelFormat;
    ImagePixelType::Type             pixelType;
    vector<textures::ImageMipmap>    mipmaps;
	string                           filename;
    ImageLoadedStructure(){
        ImageMipmap baseImage;
		filename = "";
        baseImage.compressedSize = 0;
        mipmaps.push_back(baseImage);
    }
    ~ImageLoadedStructure(){}
	void load(uint _width,uint _height){
        ImageMipmap* baseImage = nullptr;
		if(mipmaps.size() > 0){
			baseImage = &(mipmaps.at(0));
		}
		else{
			baseImage = new ImageMipmap();
		}
		filename = "";
        baseImage->width = _width;
        baseImage->height = _height;
        baseImage->compressedSize = 0;
		if(mipmaps.size() == 0){
			mipmaps.push_back(*baseImage);
		}
	}
	void load(const sf::Image& i,string _filename = ""){
        ImageMipmap* baseImage = nullptr;
		if(mipmaps.size() > 0){
			baseImage = &(mipmaps.at(0));
		}
		else{
			baseImage = new ImageMipmap();
		}
		filename = _filename;
        baseImage->width = i.getSize().x;
        baseImage->height = i.getSize().y;
        baseImage->pixels.assign(i.getPixelsPtr(),i.getPixelsPtr() + baseImage->width * baseImage->height * 4);
        baseImage->compressedSize = 0;
		if(mipmaps.size() == 0){
			mipmaps.push_back(*baseImage);
		}
	}
    ImageLoadedStructure(uint _width,uint _height){ load(_width,_height); }
    ImageLoadedStructure(const sf::Image& i,string _filename = ""){ load(i,_filename); }
};


class Texture::impl final{
    friend class epriv::TextureLoader;
    friend class ::Texture;
    public:
		vector<ImageLoadedStructure*> m_ImagesDatas;
        vector<GLuint> m_TextureAddress;
        GLuint m_Type;
		TextureType::Type m_TextureType;
        bool m_Mipmapped;
        bool m_IsToBeMipmapped;
        GLuint m_MinFilter; //used to determine filter type for mipmaps

        void _initFramebuffer(uint _w,uint _h,ImagePixelType::Type _pixelType,ImagePixelFormat::Format _pixelFormat,ImageInternalFormat::Format _internalFormat,float _divisor,Texture* _super){
            ImageLoadedStructure* image = new ImageLoadedStructure(uint(float(_w)*_divisor),uint(float(_h)*_divisor));
			image->pixelFormat = _pixelFormat;
			image->pixelType = _pixelType;
			image->internalFormat = _internalFormat;
			_baseInit(GL_TEXTURE_2D,"RenderTarget",false,_super);
			m_ImagesDatas.push_back(image);
            _super->load();
        }
        void _initFromPixelsMemory(const sf::Image& _sfImage,string _name,GLuint _openglTextureType,bool _genMipMaps,ImageInternalFormat::Format _internalFormat,Texture* _super){
            ImageLoadedStructure* image = new ImageLoadedStructure(_sfImage,_name);
			image->pixelType = ImagePixelType::UNSIGNED_BYTE;
			image->internalFormat = _internalFormat;
            _baseInit(_openglTextureType,_name,false,_super);
			m_ImagesDatas.push_back(image);
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
                ImageLoadedStructure* image = new ImageLoadedStructure();
				image->filename = _filename;
                TextureLoader::LoadDDSFile(_super,_filename,*image);
                _baseInit(_openglTextureType,_filename,false,_super);
				m_ImagesDatas.push_back(image);
                _super->load();

                //mipmaps
                if(image->mipmaps.size() >= 2){
                    glBindTexture(m_Type, m_TextureAddress.at(0));	
                    for(auto mip:image->mipmaps){
                        if(mip.level >= 1)
                            glCompressedTexImage2D(GL_TEXTURE_2D,mip.level,image->internalFormat,mip.width,mip.height,0,mip.compressedSize,&mip.pixels[0]);
                    }
                    glBindTexture(m_Type, 0);
                }
            }
        }
        void _initCubemap(string _name,ImageInternalFormat::Format _internalFormat,bool _genMipMaps,Texture* _super){
			for(auto sideImage:m_ImagesDatas){
				m_Type = GL_TEXTURE_CUBE_MAP;
				sf::Image img; img.loadFromFile(sideImage->filename);
				sideImage->load(img,sideImage->filename);
				sideImage->pixelType = ImagePixelType::UNSIGNED_BYTE;
				sideImage->internalFormat = _internalFormat;
				TextureLoader::ChoosePixelFormat(sideImage->pixelFormat,sideImage->internalFormat);	
			}
			_super->load();
        }
        void _baseInit(GLuint type,string n,bool genMipMaps,Texture* _super){
            m_Mipmapped = false;
            m_IsToBeMipmapped = genMipMaps;
            m_MinFilter = GL_LINEAR;
            m_Type = type;
            _super->setName(n);		
        }
        void _load(Texture* super){
            if(m_TextureAddress.size() == 0)
                m_TextureAddress.push_back(0); //vector.at(0) will be the default address. at(1) is the colvoluted map address (for cubemap only)
            glGenTextures(1, &m_TextureAddress.at(0));
            glBindTexture(m_Type, m_TextureAddress.at(0));
            
			switch(m_TextureType){
				case TextureType::RenderTarget:{
					TextureLoader::LoadTextureFramebufferIntoOpenGL(super); break;
                }
				case TextureType::Texture1D:{ break; }
				case TextureType::Texture2D:{
					TextureLoader::LoadTexture2DIntoOpenGL(super); break;
                }
				case TextureType::Texture3D:{ break; }
				case TextureType::CubeMap:{
					TextureLoader::LoadTextureCubemapIntoOpenGL(super); break;
                }
				default:{ break; }
			}
            if(m_IsToBeMipmapped) TextureLoader::GenerateMipmapsOpenGL(super);
        }
        void _unload(){
            for(uint i = 0; i < m_TextureAddress.size(); ++i){
                glDeleteTextures(1,&m_TextureAddress.at(i));
            }
            glBindTexture(m_Type,0);
            m_Mipmapped = false;
            vector_clear(m_TextureAddress);
        }
        void _resize(epriv::FramebufferTexture* t,uint w, uint h){
            if(m_TextureType != TextureType::RenderTarget){
                cout << "Error: Non-framebuffer texture cannot be resized. Returning..." << endl;
                return;
            }
            glBindTexture(m_Type, m_TextureAddress.at(0));
			uint _w = uint(float(w) * t->divisor());
			uint _h = uint(float(h) * t->divisor());
			m_ImagesDatas.at(0)->mipmaps.at(0).width = _w; 
            m_ImagesDatas.at(0)->mipmaps.at(0).height = _h;
			glTexImage2D(m_Type,0,ImageInternalFormat::at(m_ImagesDatas.at(0)->internalFormat),_w,_h,0,ImagePixelFormat::at(m_ImagesDatas.at(0)->pixelFormat),ImagePixelType::at(m_ImagesDatas.at(0)->pixelType),NULL);
        }
};

void epriv::TextureLoader::LoadDDSFile(Texture* _texture,string _filename,ImageLoadedStructure& image){
    FILE* fileparser = fopen(_filename.c_str(), "rb");
    if (!fileparser) return;
    uchar header_buffer[128];
    fread(&header_buffer, 128, 1, fileparser);

    DDS::DDS_Header head(header_buffer);
    if (head.magic != 0x20534444){ // "DDS "
        fclose(fileparser); return;
    }
    //DX10 header here
    DDS::DDS_Header_DX10 headDX10;
    if( (head.header_flags & DDS::DDPF_FOURCC) && head.format.fourCC == FourCC_DX10 ){
        uchar header_buffer_DX10[20];
        fread(&header_buffer_DX10, 20, 1, fileparser);
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
        case FourCC_ATI1:{ 
            factor = 2;
            blockSize = 8;
            break;
        }
        case FourCC_ATI2:{//aka ATI2n aka 3Dc aka LATC2 aka BC5 - used for normal maps (store x,y recalc z) z = sqrt( 1-x*x-y*y )
            blockSize = 16;
            image.internalFormat = ImageInternalFormat::COMPRESSED_RG_RGTC2;
            break; 
        }
        case FourCC_RXGB:{ 
            factor = 4;
            blockSize = 16;
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

    uint bufferSize = head.mipMapCount >= 2 ? head.pitchOrlinearSize * factor : head.pitchOrlinearSize;
    uchar* pxls = (uchar*)malloc(bufferSize * sizeof(uchar));
    fread(pxls, 1, bufferSize, fileparser);
    fclose(fileparser);
    image.pixelFormat = ImagePixelFormat::RGBA;
    image.pixelType = ImagePixelType::UNSIGNED_BYTE;	
    offset = bufferSize;
    for (uint level = 0; level < head.mipMapCount && (head.w || head.h); ++level){
        if(level > 0 && (head.w < 64 || head.h < 64)) break;
        ImageMipmap* mipmap = nullptr;
        ImageMipmap mipMapCon = ImageMipmap();
        if(level >= 1){ mipmap = &mipMapCon; }
        else{           mipmap = &image.mipmaps.at(0); }
        mipmap->level = level;
        mipmap->width = head.w;
        mipmap->height = head.h;
        mipmap->compressedSize = ((head.w + 3) / 4) * ((head.h + 3) / 4) * blockSize;
        mipmap->pixels.assign(pxls,pxls + offset);
        head.w = Math::Max(uint(head.w / 2), 1);
        head.h = Math::Max(uint(head.h / 2), 1);
        offset += mipmap->compressedSize;
    }
    free(pxls);
}
void epriv::TextureLoader::LoadTexture2DIntoOpenGL(Texture* _texture){
    Texture::impl& i = *_texture->m_i;
	TextureLoader::ChoosePixelFormat(i.m_ImagesDatas.at(0)->pixelFormat,i.m_ImagesDatas.at(0)->internalFormat);
	uint level = 0;
	for(auto mipmap:i.m_ImagesDatas.at(0)->mipmaps){
		if(TextureLoader::IsCompressedType(i.m_ImagesDatas.at(0)->internalFormat) && mipmap.compressedSize != 0){
			glCompressedTexImage2D(i.m_Type,level,ImageInternalFormat::at(i.m_ImagesDatas.at(0)->internalFormat),mipmap.width,mipmap.height,0,mipmap.compressedSize,&(mipmap.pixels)[0]);
		}
		else{
			glTexImage2D(i.m_Type,level,ImageInternalFormat::at(i.m_ImagesDatas.at(0)->internalFormat),mipmap.width,mipmap.height,0,ImagePixelFormat::at(i.m_ImagesDatas.at(0)->pixelFormat),ImagePixelType::at(i.m_ImagesDatas.at(0)->pixelType),&(mipmap.pixels)[0]);
		}
		++level;
	}
    _texture->setFilter(TextureFilter::Linear);
    glBindTexture(i.m_Type,0);
    epriv::TextureLoader::WithdrawPixelsFromOpenGLMemory(_texture);
}
void epriv::TextureLoader::LoadTextureFramebufferIntoOpenGL(Texture* _texture){
    Texture::impl& i = *_texture->m_i;
    glBindTexture(i.m_Type,i.m_TextureAddress.at(0));
	uint& _w = i.m_ImagesDatas.at(0)->mipmaps.at(0).width;
	uint& _h = i.m_ImagesDatas.at(0)->mipmaps.at(0).height;
    glTexImage2D(i.m_Type,0,ImageInternalFormat::at(i.m_ImagesDatas.at(0)->internalFormat),_w,_h,0,ImagePixelFormat::at(i.m_ImagesDatas.at(0)->pixelFormat),ImagePixelType::at(i.m_ImagesDatas.at(0)->pixelType),NULL);
    _texture->setFilter(TextureFilter::Linear);
    _texture->setWrapping(TextureWrap::ClampToEdge);
    glBindTexture(i.m_Type,0);
}
void epriv::TextureLoader::LoadTextureCubemapIntoOpenGL(Texture* _texture){
    Texture::impl& i = *_texture->m_i;
	glBindTexture(i.m_Type,i.m_TextureAddress.at(0));
	uint imageIndex = 0;
	for(auto image:i.m_ImagesDatas){
		uint level = 0;
		for(auto mipmap:image->mipmaps){
			if(TextureLoader::IsCompressedType(i.m_ImagesDatas.at(0)->internalFormat) && mipmap.compressedSize != 0){
				glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + imageIndex,level,ImageInternalFormat::at(i.m_ImagesDatas.at(0)->internalFormat),mipmap.width,mipmap.height,0,mipmap.compressedSize,&(mipmap.pixels)[0]);
			}
			else{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + imageIndex,level,ImageInternalFormat::at(i.m_ImagesDatas.at(0)->internalFormat),mipmap.width,mipmap.height,0,ImagePixelFormat::at(i.m_ImagesDatas.at(0)->pixelFormat),ImagePixelType::at(i.m_ImagesDatas.at(0)->pixelType),&(mipmap.pixels)[0]);
			}
			TextureLoader::WithdrawPixelsFromOpenGLMemory(_texture,imageIndex,level);
			++level;
		}
		++imageIndex;
	}
	_texture->setFilter(TextureFilter::Linear);
	_texture->setWrapping(TextureWrap::ClampToEdge);
	glBindTexture(i.m_Type,0);
}
void epriv::TextureLoader::WithdrawPixelsFromOpenGLMemory(Texture* _texture,uint imageIndex,uint mipmapLevel){
    Texture::impl& i = *_texture->m_i;
	vector<uchar>& pxls = i.m_ImagesDatas.at(imageIndex)->mipmaps.at(mipmapLevel).pixels;
	if(pxls.size() != 0) return;
	uint& _w = i.m_ImagesDatas.at(imageIndex)->mipmaps.at(mipmapLevel).width;
	uint& _h = i.m_ImagesDatas.at(imageIndex)->mipmaps.at(mipmapLevel).height;
    pxls.resize(_w * _h * 4);
    glBindTexture(i.m_Type,i.m_TextureAddress.at(0));
	glGetTexImage(i.m_Type,0,ImagePixelFormat::at(i.m_ImagesDatas.at(imageIndex)->pixelFormat),ImagePixelType::at(i.m_ImagesDatas.at(imageIndex)->pixelType),&pxls[0]);
    glBindTexture(i.m_Type,0);

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
	uint& _w = i.m_ImagesDatas.at(0)->mipmaps.at(0).width;
	uint& _h = i.m_ImagesDatas.at(0)->mipmaps.at(0).height;
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
    //uint mipmaplevels = uint(glm::log2(glm::max(_w,_h)) + 1.0f);
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
	m_i->m_TextureType = TextureType::RenderTarget;
    m_i->_initFramebuffer(_w,_h,_pxlType,_pxlFormat,_internal,_divisor,this);
}
Texture::Texture(const sf::Image& _sfImage,string _name,GLuint _openglTextureType,bool _genMipMaps,ImageInternalFormat::Format _internalFormat):m_i(new impl){
	m_i->m_TextureType = TextureType::Texture2D;
    m_i->_initFromPixelsMemory(_sfImage,_name,_openglTextureType,_genMipMaps,_internalFormat,this);
}
Texture::Texture(string _filename,GLuint _openglTextureType,bool _genMipMaps,ImageInternalFormat::Format _internalFormat):m_i(new impl){
    m_i->m_TextureType = TextureType::Texture2D;
    m_i->_initFromImageFile(_filename,_openglTextureType,_genMipMaps,_internalFormat,this);
}
//CubemapFrom6ImageFiles
Texture::Texture(string files[],string _name,bool _genMipMaps,ImageInternalFormat::Format _internalFormat):m_i(new impl){
	m_i->m_TextureType = TextureType::CubeMap;
    for(uint j = 0; j < 6; ++j){ 
		ImageLoadedStructure* image = new ImageLoadedStructure();
		image->filename = files[j];
		m_i->m_ImagesDatas.push_back(image);
    }
    m_i->_initCubemap(_name,_internalFormat,_genMipMaps,this);
}
Texture::~Texture(){
    unload();
	for(auto data:m_i->m_ImagesDatas)
		SAFE_DELETE(data);
}
void Texture::render(glm::vec2& pos, glm::vec4& color,float angle, glm::vec2& scl, float depth){
	if(m_i->m_TextureType == TextureType::CubeMap) return;
    Core::m_Engine->m_RenderManager->_renderTexture(this,pos,color,scl,angle,depth);
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
    TextureLoader::EnumWrapToGL(gl,w);
    glTexParameteri(type, GL_TEXTURE_WRAP_S, gl);
}
void Texture::setYWrapping(GLuint type,TextureWrap::Wrap w){ 
    GLuint gl; 
    TextureLoader::EnumWrapToGL(gl,w);
    glTexParameteri(type, GL_TEXTURE_WRAP_T, gl);
}
void Texture::setZWrapping(GLuint type,TextureWrap::Wrap w){
    if(type != GL_TEXTURE_CUBE_MAP) return;
    GLuint gl;
    TextureLoader::EnumWrapToGL(gl,w);
    glTexParameteri(type,GL_TEXTURE_WRAP_R,gl);
}
void Texture::setWrapping(GLuint type,TextureWrap::Wrap w){ 
    Texture::setXWrapping(type,w);
    Texture::setYWrapping(type,w);
    Texture::setZWrapping(type,w);
}
void Texture::setMinFilter(GLuint type,TextureFilter::Filter filter){ 
    GLuint gl;
    TextureLoader::EnumFilterToGL(gl,filter,true);
    glTexParameteri(type,GL_TEXTURE_MIN_FILTER,gl); 
}
void Texture::setMaxFilter(GLuint type,TextureFilter::Filter filter){ 
    GLuint gl;
    TextureLoader::EnumFilterToGL(gl,filter,false);
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
    Core::m_Engine->m_RenderManager->_genPBREnvMapData(this,convoludeTextureSize,preEnvFilterSize);
}
void Texture::resize(FramebufferTexture* t,uint w, uint h){ m_i->_resize(t,w,h); }
bool Texture::mipmapped(){ return m_i->m_Mipmapped; }
bool Texture::compressed(){
	//if(m_i->m_ImagesDatas.size() == 0) return false;
	//else if(m_i->m_ImagesDatas.at(0)->mipmaps.size() == 0) return false;
	if(m_i->m_ImagesDatas.at(0)->mipmaps.at(0).compressedSize > 0) return true; return false;
}
uchar* Texture::pixels(){ TextureLoader::WithdrawPixelsFromOpenGLMemory(this); return &(m_i->m_ImagesDatas.at(0)->mipmaps.at(0).pixels)[0]; }
GLuint& Texture::address(){ return m_i->m_TextureAddress.at(0); }
GLuint& Texture::address(uint index){ return m_i->m_TextureAddress.at(index); }
uint Texture::numAddresses(){ return m_i->m_TextureAddress.size(); }
GLuint Texture::type(){ return m_i->m_Type; }
uint Texture::width(){ return m_i->m_ImagesDatas.at(0)->mipmaps.at(0).width; }
uint Texture::height(){ return m_i->m_ImagesDatas.at(0)->mipmaps.at(0).height; }
ImageInternalFormat::Format Texture::internalFormat(){ return m_i->m_ImagesDatas.at(0)->internalFormat; }
ImagePixelFormat::Format Texture::pixelFormat(){ return m_i->m_ImagesDatas.at(0)->pixelFormat; }
ImagePixelType::Type Texture::pixelType(){ return m_i->m_ImagesDatas.at(0)->pixelType; }
