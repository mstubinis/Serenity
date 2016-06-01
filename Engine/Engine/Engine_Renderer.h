#pragma once
#ifndef ENGINE_ENGINE_RENDERER_H
#define ENGINE_ENGINE_RENDERER_H

#include <glm/glm.hpp>
#include <string>
#include <vector>

#ifdef _WIN32
#include <d3d11.h>
#include <d3d10.h>

// include the Direct3D Library file
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3d10.lib")
#endif

class GBuffer;
class Object;
typedef unsigned int GLuint;

struct GeometryRenderInfo final{
    Object* object;
    GLuint shader;
    GeometryRenderInfo(Object* _object, GLuint _shader){
        object = _object;
        shader = _shader;
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
        texture = _texture;
        pos = _pos;
        col = _col;
        scl = _scl;
        rot = _rot;
        depth = _depth;
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
        struct RendererInfo final{
            static bool positionOnly;
            static bool normalsOnly;
            static bool ssaoOnly;
            static bool diffuseOnly;
            static bool bloomOnly;

            static bool bloom;
            static bool lighting;
            static bool debug;

            struct HDRInfo{
                static bool hdr;
                static float hdr_exposure;
                static float hdr_gamma;
            };
            struct GodRaysInfo{
                static bool godRays;
                static float godRays_exposure;
                static float godRays_decay;
                static float godRays_density;
                static float godRays_weight;
                static unsigned int godRays_samples;
            };
            struct SSAOInfo{
                static bool ssao;
                static bool ssao_do_blur;
                static unsigned int ssao_samples;
                static float ssao_bias;
                static float ssao_scale;
                static float ssao_radius;
                static float ssao_intensity;
                static glm::vec2 ssao_Kernels[64];
                static GLuint ssao_noise_texture;
                static unsigned int ssao_noise_texture_size;
            };
        };
        namespace Settings{
            namespace GodRays{
                static void setExposure(float e){ Renderer::RendererInfo::GodRaysInfo::godRays_exposure = e; }
                static void setDecay(float d){ Renderer::RendererInfo::GodRaysInfo::godRays_decay = d; }
                static void setDensity(float d){ Renderer::RendererInfo::GodRaysInfo::godRays_density = d; }
                static void setWeight(float w){ Renderer::RendererInfo::GodRaysInfo::godRays_weight = w; }
                static void setSamples(unsigned int s){ Renderer::RendererInfo::GodRaysInfo::godRays_samples = s; }
            };
            namespace SSAO{
                static void setIntensity(float i){ Renderer::RendererInfo::SSAOInfo::ssao_intensity = i; }
                static void setRadius(float r){ Renderer::RendererInfo::SSAOInfo::ssao_radius = r; }
                static void setScale(float s){ Renderer::RendererInfo::SSAOInfo::ssao_scale = s; }
                static void setBias(float b){ Renderer::RendererInfo::SSAOInfo::ssao_bias = b; }
                static void setSamples(unsigned int s){ Renderer::RendererInfo::SSAOInfo::ssao_samples = s; }
            };
            static void renderDiffuseOnly(bool = true){
                Renderer::RendererInfo::diffuseOnly = true;

                Renderer::RendererInfo::positionOnly = false;
                Renderer::RendererInfo::normalsOnly = false;
                Renderer::RendererInfo::ssaoOnly = false;
                Renderer::RendererInfo::bloomOnly = false;
            }
            static void renderNormalsOnly(bool = true){
                Renderer::RendererInfo::normalsOnly = true;

                Renderer::RendererInfo::diffuseOnly = false;
                Renderer::RendererInfo::positionOnly = false;
                Renderer::RendererInfo::ssaoOnly = false;
                Renderer::RendererInfo::bloomOnly = false;
            }
            static void renderPositionOnly(bool = true){
                Renderer::RendererInfo::positionOnly = true;

                Renderer::RendererInfo::diffuseOnly = false;
                Renderer::RendererInfo::normalsOnly = false;
                Renderer::RendererInfo::ssaoOnly = false;
                Renderer::RendererInfo::bloomOnly = false;
            }
            static void renderSSAOOnly(bool = true){
                Renderer::RendererInfo::ssaoOnly = true;

                Renderer::RendererInfo::diffuseOnly = false;
                Renderer::RendererInfo::positionOnly = false;
                Renderer::RendererInfo::normalsOnly = false;
                Renderer::RendererInfo::bloomOnly = false;
            }
            static void renderBloomOnly(bool = true){
                Renderer::RendererInfo::bloomOnly = true;

                Renderer::RendererInfo::diffuseOnly = false;
                Renderer::RendererInfo::positionOnly = false;
                Renderer::RendererInfo::normalsOnly = false;
                Renderer::RendererInfo::ssaoOnly = false;
            }
            static void renderNormally(bool = true){
                Renderer::RendererInfo::bloomOnly = false;
                Renderer::RendererInfo::diffuseOnly = false;
                Renderer::RendererInfo::positionOnly = false;
                Renderer::RendererInfo::normalsOnly = false;
                Renderer::RendererInfo::ssaoOnly = false;
            }
            static void enableGodsRays(bool enabled = true){ Renderer::RendererInfo::GodRaysInfo::godRays = enabled; }
			static void enableHDR(bool enabled = true){ Renderer::RendererInfo::HDRInfo::hdr = enabled; }
            static void enableLighting(bool enabled = true){ Renderer::RendererInfo::lighting = enabled; }
            static void enableBloom(bool enabled = true){ Renderer::RendererInfo::bloom = enabled; }
            static void enableSSAO(bool enabled = true){ Renderer::RendererInfo::SSAOInfo::ssao = enabled;  }
            static void enableDebugDrawing(bool enabled = true){ Renderer::RendererInfo::debug = enabled;  }
        };
        namespace Detail{
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
                    static void _passSSAO();
                    static void _passEdge(GLuint texture,float radius = 1.0f);
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
            void renderFullscreenQuad(GLuint shader, unsigned int width, unsigned int height,float scale = 1.0f);
        };
        void renderRectangle(glm::vec2 pos, glm::vec4 color, float width, float height, float angle, float depth);
    };
};
#endif