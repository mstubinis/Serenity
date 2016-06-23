#pragma once
#ifndef ENGINE_ENGINE_RENDERER_H
#define ENGINE_ENGINE_RENDERER_H

#include <glm/glm.hpp>
#include <string>
#include <vector>

#ifdef _WIN32
#include <d3d11.h>
#include <d3d10.h>
#pragma comment (lib, "d3d11.lib")// include the Direct3D Library file
#pragma comment (lib, "d3d10.lib")// include the Direct3D Library file
#endif

class Texture;
class Font;
class GBuffer;
class Object;
typedef unsigned int GLuint;
typedef unsigned int uint;

struct GeometryRenderInfo final{
    Object* object;
    GLuint shader;
    GeometryRenderInfo(Object* _obj, GLuint _shader){
        object = _obj; shader = _shader;
    }
};
struct TextureRenderInfo{
    std::string texture;
    glm::vec2 pos;
    glm::vec4 col;
    glm::vec2 scl;
    float rot;
    float depth;
    TextureRenderInfo(){
        texture = ""; pos = scl = glm::vec2(0); col = glm::vec4(1); rot = depth = 0;
    }
    TextureRenderInfo(std::string _texture, glm::vec2 _pos, glm::vec4 _col, glm::vec2 _scl, float _rot, float _depth){
        texture = _texture; pos = _pos; col = _col; scl = _scl; rot = _rot; depth = _depth;
    }
};
struct FontRenderInfo final: public TextureRenderInfo{
    std::string text;
    FontRenderInfo():TextureRenderInfo(){
        text = "";
    }
    FontRenderInfo(std::string _font, std::string _text, glm::vec2 _pos, glm::vec4 _col, glm::vec2 _scl, float _rot, float _depth):TextureRenderInfo(_font,_pos,_col,_scl,_rot,_depth){
        text = _text;
    }
};

namespace Engine{
    namespace Renderer{
        namespace Detail{
			struct RendererInfo final{
				struct DebugDrawingInfo final{
					static bool debug;
				};
				struct LightingInfo final{
					static bool lighting;
				};
				struct BloomInfo final{
					static bool bloom;
					static float bloom_radius;
					static float bloom_strength;
				};
				struct HDRInfo final{
					static bool hdr;
					static float hdr_exposure;
					static float hdr_gamma;
				};
				struct GodRaysInfo final{
					static bool godRays;
					static float godRays_exposure;
					static float godRays_decay;
					static float godRays_density;
					static float godRays_weight;
					static uint godRays_samples;
				};
				struct SSAOInfo final{
					static bool ssao;
					static bool ssao_do_blur;
					static float ssao_blur_strength;
					static uint ssao_samples;
					static float ssao_bias;
					static float ssao_scale;
					static float ssao_radius;
					static float ssao_intensity;

					static const int SSAO_KERNEL_COUNT = 32;
					static const int SSAO_NORMALMAP_SIZE = 128;
					static glm::vec2 ssao_Kernels[SSAO_KERNEL_COUNT];
					static GLuint ssao_noise_texture;
					static uint ssao_noise_texture_size;
				};
			};
            class RenderManagement final{
                #ifdef _WIN32
                    public: static IDXGISwapChain* m_DirectXSwapChain;
                    public: static ID3D11Device* m_DirectXDevice;
                    public: static ID3D11DeviceContext* m_DirectXDeviceContext;
                    public: static ID3D11RenderTargetView* m_DirectXBackBuffer;
                    public: static void renderDirectX();
                #endif

                private:
                    static std::vector<FontRenderInfo> m_FontsToBeRendered;
                    static std::vector<TextureRenderInfo> m_TexturesToBeRendered;
                    static std::vector<GeometryRenderInfo> m_ObjectsToBeRendered;
                    static std::vector<GeometryRenderInfo> m_ForegroundObjectsToBeRendered;
                    static std::vector<GeometryRenderInfo> m_ObjectsToBeForwardRendered;

                    static void _renderForwardRenderedObjects();
                    static void _renderObjects();
                    static void _renderForegroundObjects();
                    static void _renderText();
                    static void _renderTextures();
                    static void _passGodsRays(glm::vec2);
                    static void _passHDR();
                    static void _passGeometry();
                    static void _passLighting();
                    static void _passSSAO(bool ssao,bool bloom);
                    static void _passEdge(GLuint texture,float radius = 1.0f);
					static void _passBlur(std::string type,GLuint texture,float radius = 1.0f,glm::vec4 strengthModifier = glm::vec4(1.0f),std::string channels = "RGBA");
                    static void _passBlur(std::string type,GLuint texture,float radius = 1.0f,float strengthModifier = 1.0f,std::string channels = "RGBA");
                    static void _passFinal();
                public:
                    static GBuffer* m_gBuffer;
                    static glm::mat4 m_2DProjectionMatrix;

                    static void render();

                    static void init();
                    static void destruct();

                    static std::vector<GeometryRenderInfo>& getForegroundObjectRenderQueue(){ return m_ForegroundObjectsToBeRendered; }
                    static std::vector<GeometryRenderInfo>& getObjectRenderQueue(){ return m_ObjectsToBeRendered; }
                    static std::vector<FontRenderInfo>& getFontRenderQueue(){ return m_FontsToBeRendered; }
                    static std::vector<TextureRenderInfo>& getTextureRenderQueue(){ return m_TexturesToBeRendered; }
            };
            void renderFullscreenQuad(GLuint shader, uint width, uint height,float scale = 1.0f);
        };

		namespace Settings{
			namespace HDR{
				static void enable(bool b = true){ Detail::RendererInfo::HDRInfo::hdr = b; }
				static void disable(){ Detail::RendererInfo::HDRInfo::hdr = false; }

				static float getExposure(){ return Detail::RendererInfo::HDRInfo::hdr_exposure; }
				static float getGamma(){ return Detail::RendererInfo::HDRInfo::hdr_gamma; }
				static void setExposure(float e){ Detail::RendererInfo::HDRInfo::hdr_exposure = e; }
				static void setGamma(float g){ Detail::RendererInfo::HDRInfo::hdr_gamma = g; }
			};
			namespace Bloom{
				static void enable(bool b = true){ Detail::RendererInfo::BloomInfo::bloom = b; }
				static void disable(){ Detail::RendererInfo::BloomInfo::bloom = false; }

				static float getRadius(){ return Detail::RendererInfo::BloomInfo::bloom_radius; }
				static float getStrength(){ return Detail::RendererInfo::BloomInfo::bloom_strength; }
				static void setRadius(float r){ Detail::RendererInfo::BloomInfo::bloom_radius = r; }
				static void setStrength(float r){ Detail::RendererInfo::BloomInfo::bloom_strength = r; }
			};
            namespace GodRays{
				static void enable(bool b = true){ Detail::RendererInfo::GodRaysInfo::godRays = b; }
				static void disable(){ Detail::RendererInfo::GodRaysInfo::godRays = false; }

                static float getExposure(){ return Detail::RendererInfo::GodRaysInfo::godRays_exposure; }
                static float getDecay(){ return Detail::RendererInfo::GodRaysInfo::godRays_decay; }
                static float getDensity(){ return Detail::RendererInfo::GodRaysInfo::godRays_density; }
                static float getWeight(){ return Detail::RendererInfo::GodRaysInfo::godRays_weight; }
                static uint getSamples(){ return Detail::RendererInfo::GodRaysInfo::godRays_samples; }

                static void setExposure(float e){ Detail::RendererInfo::GodRaysInfo::godRays_exposure = e; }
                static void setDecay(float d){ Detail::RendererInfo::GodRaysInfo::godRays_decay = d; }
                static void setDensity(float d){ Detail::RendererInfo::GodRaysInfo::godRays_density = d; }
                static void setWeight(float w){ Detail::RendererInfo::GodRaysInfo::godRays_weight = w; }
                static void setSamples(unsigned int s){ Detail::RendererInfo::GodRaysInfo::godRays_samples = s; }
            };
            namespace SSAO{
				static void enable(bool b = true){ Detail::RendererInfo::SSAOInfo::ssao = b;  }
				static void disable(){ Detail::RendererInfo::SSAOInfo::ssao = false;  }

				static void enableBlur(bool b = true){ Detail::RendererInfo::SSAOInfo::ssao_do_blur = b;  }
				static void disableBlur(){ Detail::RendererInfo::SSAOInfo::ssao_do_blur = false;  }

				static float getBlurStrength(){ return Detail::RendererInfo::SSAOInfo::ssao_blur_strength; }

                static float getIntensity(){ return Detail::RendererInfo::SSAOInfo::ssao_intensity; }
                static float getRadius(){ return Detail::RendererInfo::SSAOInfo::ssao_radius; }
                static float getScale(){ return Detail::RendererInfo::SSAOInfo::ssao_scale; }
                static float getBias(){ return Detail::RendererInfo::SSAOInfo::ssao_bias; }
                static uint getSamples(){ return Detail::RendererInfo::SSAOInfo::ssao_samples; }

				static void setBlurStrength(float s){ Detail::RendererInfo::SSAOInfo::ssao_blur_strength = s; }

                static void setIntensity(float i){ Detail::RendererInfo::SSAOInfo::ssao_intensity = i; }
                static void setRadius(float r){ Detail::RendererInfo::SSAOInfo::ssao_radius = r; }
                static void setScale(float s){ Detail::RendererInfo::SSAOInfo::ssao_scale = s; }
                static void setBias(float b){ Detail::RendererInfo::SSAOInfo::ssao_bias = b; }
                static void setSamples(unsigned int s){ Detail::RendererInfo::SSAOInfo::ssao_samples = s; }
            };
			namespace Lighting{
				static void enable(bool b = true){ Detail::RendererInfo::LightingInfo::lighting = b; }
				static void disable(){ Detail::RendererInfo::LightingInfo::lighting = false; }
			};
			namespace Debug{
				static void enable(bool b = true){ Detail::RendererInfo::DebugDrawingInfo::debug = b;  }
				static void disable(){ Detail::RendererInfo::DebugDrawingInfo::debug = false;  }
			};
        };
		void renderTexture(Texture*,glm::vec2 pos, glm::vec4 col,float angle, glm::vec2 scl, float depth);
		void renderText(std::string text,Font*, glm::vec2 pos,glm::vec4 color, float angle, glm::vec2 scl, float depth);
        void renderRectangle(glm::vec2 pos, glm::vec4 col, float w, float h, float angle, float depth);
    };
};
#endif