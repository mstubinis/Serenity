#pragma once
#ifndef ENGINE_ENGINE_RENDERER_H
#define ENGINE_ENGINE_RENDERER_H

#include "Engine_Math.h"
#include "ShaderProgram.h"

#include <glm/gtc/type_ptr.hpp>

typedef unsigned int uint;

class Texture;
class Font;
class GBuffer;
class Object;
class ObjectDisplay;
class ObjectDynamic;
class RenderedItem;
class Camera;

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

class HDRToneMapAlgorithm{
    public: enum Algorithm{
        REINHARD,
        FILMIC,
        EXPOSURE,
        UNCHARTED
    };
};
class AntiAliasingAlgorithm{
    public: enum Algorithm{
        None,
        FXAA,
        SMAA
    };
};

namespace Engine{
    namespace Renderer{
        namespace Detail{
            struct RendererInfo final{
                struct GeneralInfo final{
                    static float gamma;
                    static bool alpha_test;
                    static bool depth_mask;
                    static bool depth_test;
                    static ShaderP* current_shader_program;
                    static std::string current_bound_material;
                    static GLuint current_bound_read_fbo;
                    static GLuint current_bound_draw_fbo;
                    static AntiAliasingAlgorithm::Algorithm aa_algorithm;

                    static bool cull_face_enabled;
                    static bool draw_physics_debug;
                    static unsigned char cull_face_status;
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
                    static HDRToneMapAlgorithm::Algorithm hdr_algorithm;
                };
                struct GodRaysInfo final{
                    static bool godRays;
                    static float godRays_exposure;
                    static float godRays_decay;
                    static float godRays_density;
                    static float godRays_weight;
                    static uint godRays_samples;
                    static float godRays_fovDegrees;
                    static float godRays_alphaFalloff;
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
                private:
                    static std::vector<FontRenderInfo> m_FontsToBeRendered;
                    static std::vector<TextureRenderInfo> m_TexturesToBeRendered;

                    static void _renderText(Camera*,uint& fbufferWidth, uint& fbufferHeight);
                    static void _renderTextures(Camera*,uint& fbufferWidth, uint& fbufferHeight);
                    static void _passGodsRays(Camera*,uint& fbufferWidth, uint& fbufferHeight,glm::vec2,bool,float);
                    static void _passHDR(Camera*,uint& fbufferWidth, uint& fbufferHeight);
                    static void _passGeometry(Camera*,uint& fbufferWidth, uint& fbufferHeight,bool renderGodRays);
                    static void _passForwardRendering(Camera*,uint& fbufferWidth, uint& fbufferHeight,bool renderGodRays);
                    static void _passLighting(Camera*,uint& fbufferWidth, uint& fbufferHeight);
                    static void _passSSAO(Camera*,uint& fbufferWidth, uint& fbufferHeight,bool renderSSAO);
                    static void _passEdge(Camera*,uint& fbufferWidth, uint& fbufferHeight,GLuint texture,float radius = 1.0f);
                    static void _passBlur(Camera*,uint& fbufferWidth, uint& fbufferHeight,std::string type,GLuint texture,std::string channels = "RGBA");
                    static void _passFinal(Camera*,uint& fbufferWidth, uint& fbufferHeight);
                    static void _passFXAA(Camera*,uint& fbufferWidth, uint& fbufferHeight,bool renderAA);
                    static void _passSMAA(Camera*,uint& fbufferWidth, uint& fbufferHeight,bool renderAA);
                    static void _passCopyDepth(Camera*,uint& fbufferWidth, uint& fbufferHeight);
                public:
                    static GBuffer* m_gBuffer;
                    static glm::mat4 m_2DProjectionMatrix;

                    static void render(Camera*,uint fbufferWidth, uint fbufferHeight,bool ssao = true, bool godRays = true, bool AA = true);

                    static void init();
                    static void destruct();

                    static std::vector<ShaderP*> m_GeometryPassShaderPrograms;
                    static std::vector<ShaderP*> m_ForwardPassShaderPrograms;
                
                    static std::vector<FontRenderInfo>& getFontRenderQueue(){ return m_FontsToBeRendered; }
                    static std::vector<TextureRenderInfo>& getTextureRenderQueue(){ return m_TexturesToBeRendered; }
            };
            void renderFullscreenQuad(uint width, uint height);
        };

        namespace Settings{

            void setGamma(float g);
            float getGamma();

            void clear(bool color = true, bool depth = true, bool stencil = true);

            void enableCullFace(bool b = true);
            void disableCullFace();
            void cullFace(uint state);

            void setAntiAliasingAlgorithm(AntiAliasingAlgorithm::Algorithm);

            void enableAlphaTest(bool b = true);
            void disableAlphaTest();
            void enableDepthTest(bool b = true);
            void disableDepthTest();
            void enableDepthMask(bool b = true);
            void disableDepthMask();
            void enableDrawPhysicsInfo(bool b = true);
            void disableDrawPhysicsInfo();
        
            namespace HDR{
                static void enable(bool b = true){ Detail::RendererInfo::HDRInfo::hdr = b; }
                static void disable(){ Detail::RendererInfo::HDRInfo::hdr = false; }

                static float getExposure(){ return Detail::RendererInfo::HDRInfo::hdr_exposure; }
                static void setExposure(float e){ Detail::RendererInfo::HDRInfo::hdr_exposure = e; }

                static void setAlgorithm(HDRToneMapAlgorithm::Algorithm a){ Detail::RendererInfo::HDRInfo::hdr_algorithm = a; }
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
                static float getFOVDegrees(){ return Detail::RendererInfo::GodRaysInfo::godRays_fovDegrees; }
                static float getAlphaFalloff(){ return Detail::RendererInfo::GodRaysInfo::godRays_alphaFalloff; }

                static void setExposure(float e){ Detail::RendererInfo::GodRaysInfo::godRays_exposure = e; }
                static void setDecay(float d){ Detail::RendererInfo::GodRaysInfo::godRays_decay = d; }
                static void setDensity(float d){ Detail::RendererInfo::GodRaysInfo::godRays_density = d; }
                static void setWeight(float w){ Detail::RendererInfo::GodRaysInfo::godRays_weight = w; }
                static void setSamples(uint s){ Detail::RendererInfo::GodRaysInfo::godRays_samples = s; }
                static void setFOVDegrees(float d){ Detail::RendererInfo::GodRaysInfo::godRays_fovDegrees = d; }
                static void setAlphaFalloff(float a){ Detail::RendererInfo::GodRaysInfo::godRays_alphaFalloff = a; }
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
                static void setSamples(uint s){ Detail::RendererInfo::SSAOInfo::ssao_samples = s; }
            };
            namespace Lighting{
                static void enable(bool b = true){ Detail::RendererInfo::LightingInfo::lighting = b; }
                static void disable(){ Detail::RendererInfo::LightingInfo::lighting = false; }
            };
        };
        inline const GLint getUniformLocation(const char* location){ const std::unordered_map<std::string,GLint>& m = Detail::RendererInfo::GeneralInfo::current_shader_program->uniforms();if(!m.count(location))return-1;return m.at(location); }
        inline const GLint& getUniformLocationUnsafe(const char* location){ return Detail::RendererInfo::GeneralInfo::current_shader_program->uniforms().at(location); }
        
        void bindFBO(GLuint);
        void bindReadFBO(GLuint);
        void bindDrawFBO(GLuint);
        void bindTexture(const char* location,Texture*,uint slot);
        void bindTexture(const char* location,GLuint textureAddress,uint slot,GLuint type);
        void bindTextureSafe(const char* location,Texture*,uint slot);
        void bindTextureSafe(const char* location,GLuint textureAddress,uint slot,GLuint type);
        void unbindTexture(uint slot,Texture*);
        void unbindTexture2D(uint slot);
        void unbindTextureCubemap(uint slot);
        
        
        //Uniform 1
        //seperate (dont need vectors for 1 component ;) )
        inline void sendUniform1(const char* l,double x){ glUniform1d(getUniformLocationUnsafe(l),x); }
        inline void sendUniform1(const char* l,int x){ glUniform1i(getUniformLocationUnsafe(l),x); }
        inline void sendUniform1(const char* l,float x){ glUniform1f(getUniformLocationUnsafe(l),x); }
        inline void sendUniform1d(const char* l,double x){ glUniform1d(getUniformLocationUnsafe(l),x); }
        inline void sendUniform1i(const char* l,int x){ glUniform1i(getUniformLocationUnsafe(l),x); }
        inline void sendUniform1f(const char* l,float x){ glUniform1f(getUniformLocationUnsafe(l),x); }
        inline void sendUniform1Safe(const char* l,double x){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform1d(o,x); }
        inline void sendUniform1Safe(const char* l,int x){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform1i(o,x); }
        inline void sendUniform1Safe(const char* l,float x){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform1f(o,x); }
        inline void sendUniform1dSafe(const char* l,double x){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform1d(o,x); }
        inline void sendUniform1iSafe(const char* l,int x){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform1i(o,x); }
        inline void sendUniform1fSafe(const char* l,float x){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform1f(o,x); }
        inline void sendUniform1Force(const char* l,double x){ glUniform1d(getUniformLocation(l),x); }
        inline void sendUniform1Force(const char* l,int x){ glUniform1i(getUniformLocation(l),x); }
        inline void sendUniform1Force(const char* l,float x){ glUniform1f(getUniformLocation(l),x); }
        inline void sendUniform1dForce(const char* l,double x){ glUniform1d(getUniformLocation(l),x); }
        inline void sendUniform1iForce(const char* l,int x){ glUniform1i(getUniformLocation(l),x); }
        inline void sendUniform1fForce(const char* l,float x){ glUniform1f(getUniformLocation(l),x); }
        
        //Uniform 2
        //vectors
        inline void sendUniform2(const char* l,glm::dvec2& v){ glUniform2d(getUniformLocationUnsafe(l),v.x,v.y); }
        inline void sendUniform2(const char* l,glm::ivec2& v){ glUniform2i(getUniformLocationUnsafe(l),v.x,v.y); }
        inline void sendUniform2(const char* l,glm::vec2& v){ glUniform2f(getUniformLocationUnsafe(l),v.x,v.y); }
        inline void sendUniform2d(const char* l,glm::dvec2& v){ glUniform2d(getUniformLocationUnsafe(l),v.x,v.y); }
        inline void sendUniform2i(const char* l,glm::ivec2& v){ glUniform2i(getUniformLocationUnsafe(l),v.x,v.y); }
        inline void sendUniform2f(const char* l,glm::vec2& v){ glUniform2f(getUniformLocationUnsafe(l),v.x,v.y); }
        inline void sendUniform2Safe(const char* l,glm::dvec2& v){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform2d(o,v.x,v.y); }
        inline void sendUniform2Safe(const char* l,glm::ivec2& v){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform2i(o,v.x,v.y); }
        inline void sendUniform2Safe(const char* l,glm::vec2& v){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform2f(o,v.x,v.y); }
        inline void sendUniform2dSafe(const char* l,glm::dvec2& v){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform2d(o,v.x,v.y); }
        inline void sendUniform2iSafe(const char* l,glm::ivec2& v){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform2i(o,v.x,v.y); }
        inline void sendUniform2fSafe(const char* l,glm::vec2& v){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform2f(o,v.x,v.y); }
        inline void sendUniform2Force(const char* l,glm::dvec2& v){ glUniform2d(getUniformLocation(l),v.x,v.y); }
        inline void sendUniform2Force(const char* l,glm::ivec2& v){ glUniform2i(getUniformLocation(l),v.x,v.y); }
        inline void sendUniform2Force(const char* l,glm::vec2& v){ glUniform2f(getUniformLocation(l),v.x,v.y); }
        inline void sendUniform2dForce(const char* l,glm::dvec2& v){ glUniform2d(getUniformLocation(l),v.x,v.y); }
        inline void sendUniform2iForce(const char* l,glm::ivec2& v){ glUniform2i(getUniformLocation(l),v.x,v.y); }
        inline void sendUniform2fForce(const char* l,glm::vec2& v){ glUniform2f(getUniformLocation(l),v.x,v.y); } 
        //seperate
        inline void sendUniform2(const char* l,double x,double y){ glUniform2d(getUniformLocationUnsafe(l),x,y); }
        inline void sendUniform2(const char* l,int x,int y){ glUniform2i(getUniformLocationUnsafe(l),x,y); }
        inline void sendUniform2(const char* l,float x,float y){ glUniform2f(getUniformLocationUnsafe(l),x,y); } 
        inline void sendUniform2d(const char* l,double x,double y){ glUniform2d(getUniformLocationUnsafe(l),x,y); }
        inline void sendUniform2i(const char* l,int x,int y){ glUniform2i(getUniformLocationUnsafe(l),x,y); }
        inline void sendUniform2f(const char* l,float x,float y){ glUniform2f(getUniformLocationUnsafe(l),x,y); }
        inline void sendUniform2Safe(const char* l,double x,double y){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform2d(o,x,y); }
        inline void sendUniform2Safe(const char* l,int x,int y){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform2i(o,x,y); }
        inline void sendUniform2Safe(const char* l,float x,float y){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform2f(o,x,y); }
        inline void sendUniform2dSafe(const char* l,double x,double y){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform2d(o,x,y); }
        inline void sendUniform2iSafe(const char* l,int x,int y){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform2i(o,x,y); }
        inline void sendUniform2fSafe(const char* l,float x,float y){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform2f(o,x,y); }
        inline void sendUniform2Force(const char* l,double x,double y){ glUniform2d(getUniformLocation(l),x,y); }
        inline void sendUniform2Force(const char* l,int x,int y){ glUniform2i(getUniformLocation(l),x,y); }
        inline void sendUniform2Force(const char* l,float x,float y){ glUniform2f(getUniformLocation(l),x,y); }
        inline void sendUniform2dForce(const char* l,double x,double y){ glUniform2d(getUniformLocation(l),x,y); }
        inline void sendUniform2iForce(const char* l,int x,int y){ glUniform2i(getUniformLocation(l),x,y); }
        inline void sendUniform2fForce(const char* l,float x,float y){ glUniform2f(getUniformLocation(l),x,y); }
        //vector and array of values
        inline void sendUniform2v(const char* l,std::vector<glm::dvec2>& d,uint i){ glUniform2dv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2v(const char* l,std::vector<glm::ivec2>& d,uint i){ glUniform2iv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2v(const char* l,std::vector<glm::vec2>& d,uint i){ glUniform2fv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2v(const char* l,glm::dvec2* d,uint i){ glUniform2dv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2v(const char* l,glm::ivec2* d,uint i){ glUniform2iv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2v(const char* l,glm::vec2* d,uint i){ glUniform2fv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2fv(const char* l,std::vector<glm::vec2>& d,uint i){ glUniform2fv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2fv(const char* l,glm::vec2* d,uint i){ glUniform2fv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2fvSafe(const char* l,std::vector<glm::vec2>& d,uint i){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform2fv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform2fvSafe(const char* l,glm::vec2* d,uint i){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform2fv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform2fvForce(const char* l,std::vector<glm::vec2>& d,uint i){ glUniform2fv(getUniformLocation(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2fvForce(const char* l,glm::vec2* d,uint i){ glUniform2fv(getUniformLocation(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2dv(const char* l,std::vector<glm::dvec2>& d,uint i){ glUniform2dv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2dv(const char* l,glm::dvec2* d,uint i){ glUniform2dv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2dvSafe(const char* l,std::vector<glm::dvec2>& d,uint i){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform2dv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform2dvSafe(const char* l,glm::dvec2* d,uint i){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform2dv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform2dvForce(const char* l,std::vector<glm::dvec2>& d,uint i){ glUniform2dv(getUniformLocation(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2dvForce(const char* l,glm::dvec2* d,uint i){ glUniform2dv(getUniformLocation(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2iv(const char* l,std::vector<glm::ivec2>& d,uint i){ glUniform2iv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2iv(const char* l,glm::ivec2* d,uint i){ glUniform2iv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2ivSafe(const char* l,std::vector<glm::ivec2>& d,uint i){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform2iv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform2ivSafe(const char* l,glm::ivec2* d,uint i){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform2iv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform2ivForce(const char* l,std::vector<glm::ivec2>& d,uint i){ glUniform2iv(getUniformLocation(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2ivForce(const char* l,glm::ivec2* d,uint i){ glUniform2iv(getUniformLocation(l),i,glm::value_ptr(d[0])); }
        
        //Uniform 3
        //vectors
        inline void sendUniform3(const char* l,glm::dvec3& v){ glUniform3d(getUniformLocationUnsafe(l),v.x,v.y,v.z); }
        inline void sendUniform3(const char* l,glm::ivec3& v){ glUniform3i(getUniformLocationUnsafe(l),v.x,v.y,v.z); }
        inline void sendUniform3(const char* l,glm::vec3& v){ glUniform3f(getUniformLocationUnsafe(l),v.x,v.y,v.z); }
        inline void sendUniform3d(const char* l,glm::dvec3& v){ glUniform3d(getUniformLocationUnsafe(l),v.x,v.y,v.z); }
        inline void sendUniform3i(const char* l,glm::ivec3& v){ glUniform3i(getUniformLocationUnsafe(l),v.x,v.y,v.z); }
        inline void sendUniform3f(const char* l,glm::vec3& v){ glUniform3f(getUniformLocationUnsafe(l),v.x,v.y,v.z); }
        inline void sendUniform3Safe(const char* l,glm::dvec3& v){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform3d(o,v.x,v.y,v.z); }
        inline void sendUniform3Safe(const char* l,glm::ivec3& v){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform3i(o,v.x,v.y,v.z); }
        inline void sendUniform3Safe(const char* l,glm::vec3& v){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform3f(o,v.x,v.y,v.z); } 
        inline void sendUniform3dSafe(const char* l,glm::dvec3& v){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform3d(o,v.x,v.y,v.z); }
        inline void sendUniform3iSafe(const char* l,glm::ivec3& v){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform3i(o,v.x,v.y,v.z); }
        inline void sendUniform3fSafe(const char* l,glm::vec3& v){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform3f(o,v.x,v.y,v.z); }
        inline void sendUniform3Force(const char* l,glm::dvec3& v){ glUniform3d(getUniformLocation(l),v.x,v.y,v.z); }
        inline void sendUniform3Force(const char* l,glm::ivec3& v){ glUniform3i(getUniformLocation(l),v.x,v.y,v.z); }
        inline void sendUniform3Force(const char* l,glm::vec3& v){ glUniform3f(getUniformLocation(l),v.x,v.y,v.z); }
        inline void sendUniform3dForce(const char* l,glm::dvec3& v){ glUniform3d(getUniformLocation(l),v.x,v.y,v.z); }
        inline void sendUniform3iForce(const char* l,glm::ivec3& v){ glUniform3i(getUniformLocation(l),v.x,v.y,v.z); }
        inline void sendUniform3fForce(const char* l,glm::vec3& v){ glUniform3f(getUniformLocation(l),v.x,v.y,v.z); }
        //seperate
        inline void sendUniform3(const char* l,double x,double y,double z){ glUniform3d(getUniformLocationUnsafe(l),x,y,z); }
        inline void sendUniform3(const char* l,int x,int y,int z){ glUniform3i(getUniformLocationUnsafe(l),x,y,z); }
        inline void sendUniform3(const char* l,float x,float y,float z){ glUniform3f(getUniformLocationUnsafe(l),x,y,z); }
        inline void sendUniform3d(const char* l,double x,double y,double z){ glUniform3d(getUniformLocationUnsafe(l),x,y,z); }
        inline void sendUniform3i(const char* l,int x,int y,int z){ glUniform3i(getUniformLocationUnsafe(l),x,y,z); }
        inline void sendUniform3f(const char* l,float x,float y,float z){ glUniform3f(getUniformLocationUnsafe(l),x,y,z); }
        inline void sendUniform3Safe(const char* l,double x,double y,double z){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform3d(o,x,y,z); }
        inline void sendUniform3Safe(const char* l,int x,int y,int z){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform3i(o,x,y,z); }
        inline void sendUniform3Safe(const char* l,float x,float y,float z){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform3f(o,x,y,z); }
        inline void sendUniform3dSafe(const char* l,double x,double y,double z){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform3d(o,x,y,z); }
        inline void sendUniform3iSafe(const char* l,int x,int y,int z){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform3i(o,x,y,z); }
        inline void sendUniform3fSafe(const char* l,float x,float y,float z){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform3f(o,x,y,z); }
        inline void sendUniform3Force(const char* l,double x,double y,double z){ glUniform3d(getUniformLocation(l),x,y,z); }
        inline void sendUniform3Force(const char* l,int x,int y,int z){ glUniform3i(getUniformLocation(l),x,y,z); }
        inline void sendUniform3Force(const char* l,float x,float y,float z){ glUniform3f(getUniformLocation(l),x,y,z); }
        inline void sendUniform3dForce(const char* l,double x,double y,double z){ glUniform3d(getUniformLocation(l),x,y,z); }
        inline void sendUniform3iForce(const char* l,int x,int y,int z){ glUniform3i(getUniformLocation(l),x,y,z); }
        inline void sendUniform3fForce(const char* l,float x,float y,float z){ glUniform3f(getUniformLocation(l),x,y,z); }
        //vector and array of values
        inline void sendUniform3v(const char* l,std::vector<glm::dvec3>& d,uint i){ glUniform3dv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3v(const char* l,std::vector<glm::ivec3>& d,uint i){ glUniform3iv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3v(const char* l,std::vector<glm::vec3>& d,uint i){ glUniform3fv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3v(const char* l,glm::dvec3* d,uint i){ glUniform3dv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3v(const char* l,glm::ivec3* d,uint i){ glUniform3iv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3v(const char* l,glm::vec3* d,uint i){ glUniform3fv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); } 
        inline void sendUniform3fv(const char* l,std::vector<glm::vec3>& d,uint i){ glUniform3fv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3fv(const char* l,glm::vec3* d,uint i){ glUniform3fv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3fvSafe(const char* l,std::vector<glm::vec3>& d,uint i){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform3fv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform3fvSafe(const char* l,glm::vec3* d,uint i){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform3fv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform3fvForce(const char* l,std::vector<glm::vec3>& d,uint i){ glUniform3fv(getUniformLocation(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3fvForce(const char* l,glm::vec3* d,uint i){ glUniform3fv(getUniformLocation(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3dv(const char* l,std::vector<glm::dvec3>& d,uint i){ glUniform3dv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3dv(const char* l,glm::dvec3* d,uint i){ glUniform3dv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3dvSafe(const char* l,std::vector<glm::dvec3>& d,uint i){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform3dv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform3dvSafe(const char* l,glm::dvec3* d,uint i){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform3dv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform3dvForce(const char* l,std::vector<glm::dvec3>& d,uint i){ glUniform3dv(getUniformLocation(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3dvForce(const char* l,glm::dvec3* d,uint i){ glUniform3dv(getUniformLocation(l),i,glm::value_ptr(d[0])); } 
        inline void sendUniform3iv(const char* l,std::vector<glm::ivec3>& d,uint i){ glUniform3iv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3iv(const char* l,glm::ivec3* d,uint i){ glUniform3iv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3ivSafe(const char* l,std::vector<glm::ivec3>& d,uint i){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform3iv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform3ivSafe(const char* l,glm::ivec3* d,uint i){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform3iv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform3ivForce(const char* l,std::vector<glm::ivec3>& d,uint i){ glUniform3iv(getUniformLocation(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3ivForce(const char* l,glm::ivec3* d,uint i){ glUniform3iv(getUniformLocation(l),i,glm::value_ptr(d[0])); }  
        
        //Uniform 4
        //vectors
        inline void sendUniform4(const char* l,glm::dvec4& v){ glUniform4d(getUniformLocationUnsafe(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4(const char* l,glm::ivec4& v){ glUniform4i(getUniformLocationUnsafe(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4(const char* l,glm::vec4& v){ glUniform4f(getUniformLocationUnsafe(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4d(const char* l,glm::dvec4& v){ glUniform4d(getUniformLocationUnsafe(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4i(const char* l,glm::ivec4& v){ glUniform4i(getUniformLocationUnsafe(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4f(const char* l,glm::vec4& v){ glUniform4f(getUniformLocationUnsafe(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4Safe(const char* l,glm::dvec4& v){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform4d(o,v.x,v.y,v.z,v.w); }
        inline void sendUniform4Safe(const char* l,glm::ivec4& v){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform4i(o,v.x,v.y,v.z,v.w); }
        inline void sendUniform4Safe(const char* l,glm::vec4& v){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform4f(o,v.x,v.y,v.z,v.w); }
        inline void sendUniform4dSafe(const char* l,glm::dvec4& v){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform4d(o,v.x,v.y,v.z,v.w); }
        inline void sendUniform4iSafe(const char* l,glm::ivec4& v){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform4i(o,v.x,v.y,v.z,v.w); }
        inline void sendUniform4fSafe(const char* l,glm::vec4& v){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform4f(o,v.x,v.y,v.z,v.w); }
        inline void sendUniform4Force(const char* l,glm::dvec4& v){ glUniform4d(getUniformLocation(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4Force(const char* l,glm::ivec4& v){ glUniform4i(getUniformLocation(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4Force(const char* l,glm::vec4& v){ glUniform4f(getUniformLocation(l),v.x,v.y,v.z,v.w); } 
        inline void sendUniform4dForce(const char* l,glm::dvec4& v){ glUniform4d(getUniformLocation(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4iForce(const char* l,glm::ivec4& v){ glUniform4i(getUniformLocation(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4fForce(const char* l,glm::vec4& v){ glUniform4f(getUniformLocation(l),v.x,v.y,v.z,v.w); }
        //seperate
        inline void sendUniform4(const char* l,double x,double y,double z,double w){ glUniform4d(getUniformLocationUnsafe(l),x,y,z,w); }
        inline void sendUniform4(const char* l,int x,int y,int z,int w){ glUniform4i(getUniformLocationUnsafe(l),x,y,z,w); }
        inline void sendUniform4(const char* l,float x,float y,float z,float w){ glUniform4f(getUniformLocationUnsafe(l),x,y,z,w); }
        inline void sendUniform4d(const char* l,double x,double y,double z,double w){ glUniform4d(getUniformLocationUnsafe(l),x,y,z,w); }
        inline void sendUniform4i(const char* l,int x,int y,int z,int w){ glUniform4i(getUniformLocationUnsafe(l),x,y,z,w); }
        inline void sendUniform4f(const char* l,float x,float y,float z,float w){ glUniform4f(getUniformLocationUnsafe(l),x,y,z,w); }
        inline void sendUniform4Safe(const char* l,double x,double y,double z,double w){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform4d(o,x,y,z,w); }
        inline void sendUniform4Safe(const char* l,int x,int y,int z,int w){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform4i(o,x,y,z,w); }
        inline void sendUniform4Safe(const char* l,float x,float y,float z,float w){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform4f(o,x,y,z,w); }
        inline void sendUniform4dSafe(const char* l,double x,double y,double z,double w){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform4d(o,x,y,z,w); }
        inline void sendUniform4iSafe(const char* l,int x,int y,int z,int w){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform4i(o,x,y,z,w); }
        inline void sendUniform4fSafe(const char* l,float x,float y,float z,float w){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform4f(o,x,y,z,w); }
        inline void sendUniform4Force(const char* l,double x,double y,double z,double w){ glUniform4d(getUniformLocation(l),x,y,z,w); }
        inline void sendUniform4Force(const char* l,int x,int y,int z,int w){ glUniform4i(getUniformLocation(l),x,y,z,w); }
        inline void sendUniform4Force(const char* l,float x,float y,float z,float w){ glUniform4f(getUniformLocation(l),x,y,z,w); }
        inline void sendUniform4dForce(const char* l,double x,double y,double z,double w){ glUniform4d(getUniformLocation(l),x,y,z,w); }
        inline void sendUniform4iForce(const char* l,int x,int y,int z,int w){ glUniform4i(getUniformLocation(l),x,y,z,w); }
        inline void sendUniform4fForce(const char* l,float x,float y,float z,float w){ glUniform4f(getUniformLocation(l),x,y,z,w); }
        //vector / array of values
        inline void sendUniform4v(const char* l,std::vector<glm::dvec4>& d,uint i){ glUniform4dv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4v(const char* l,std::vector<glm::ivec4>& d,uint i){ glUniform4iv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4v(const char* l,std::vector<glm::vec4>& d,uint i){ glUniform4fv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); } 
        inline void sendUniform4v(const char* l,glm::dvec4* d,uint i){ glUniform4dv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4v(const char* l,glm::ivec4* d,uint i){ glUniform4iv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4v(const char* l,glm::vec4* d,uint i){ glUniform4fv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4fv(const char* l,std::vector<glm::vec4>& d,uint i){ glUniform4fv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4fv(const char* l,glm::vec4* d,uint i){ glUniform4fv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4fvSafe(const char* l,std::vector<glm::vec4>& d,uint i){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform4fv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform4fvSafe(const char* l,glm::vec4* d,uint i){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform4fv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform4fvForce(const char* l,std::vector<glm::vec4>& d,uint i){ glUniform4fv(getUniformLocation(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4fvForce(const char* l,glm::vec4* d,uint i){ glUniform4fv(getUniformLocation(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4dv(const char* l,std::vector<glm::dvec4>& d,uint i){ glUniform4dv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4dv(const char* l,glm::dvec4* d,uint i){ glUniform4dv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4dvSafe(const char* l,std::vector<glm::dvec4>& d,uint i){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform4dv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform4dvSafe(const char* l,glm::dvec4* d,uint i){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform4dv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform4dvForce(const char* l,std::vector<glm::dvec4>& d,uint i){ glUniform4dv(getUniformLocation(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4dvForce(const char* l,glm::dvec4* d,uint i){ glUniform4dv(getUniformLocation(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4iv(const char* l,std::vector<glm::ivec4>& d,uint i){ glUniform4iv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4iv(const char* l,glm::ivec4* d,uint i){ glUniform4iv(getUniformLocationUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4ivSafe(const char* l,std::vector<glm::ivec4>& d,uint i){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform4iv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform4ivSafe(const char* l,glm::ivec4* d,uint i){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniform4iv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform4ivForce(const char* l,std::vector<glm::ivec4>& d,uint i){ glUniform4iv(getUniformLocation(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4ivForce(const char* l,glm::ivec4* d,uint i){ glUniform4iv(getUniformLocation(l),i,glm::value_ptr(d[0])); }

        //Matrix 2x2
        inline void sendUniformMatrix2(const char* l,glm::mat2& m){ glUniformMatrix2fv(getUniformLocationUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix2v(const char* l,std::vector<glm::mat2>& m,uint count){ std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 4; i++){d.push_back(m[i]);}}glUniformMatrix2fv(getUniformLocationUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix2(const char* l,glm::dmat2& m){ glUniformMatrix2dv(getUniformLocationUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix2v(const char* l,std::vector<glm::dmat2>& m,uint count){ std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 4; i++){d.push_back(m[i]);}}glUniformMatrix2dv(getUniformLocationUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix2f(const char* l,glm::mat2& m){ glUniformMatrix2fv(getUniformLocationUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix2fv(const char* l,std::vector<glm::mat2>& m,uint count){ std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 4; i++){d.push_back(m[i]);}}glUniformMatrix2fv(getUniformLocationUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix2d(const char* l,glm::dmat2& m){ glUniformMatrix2dv(getUniformLocationUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix2dv(const char* l,std::vector<glm::dmat2>& m,uint count){ std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 4; i++){d.push_back(m[i]);}}glUniformMatrix2dv(getUniformLocationUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix2fSafe(const char* l,glm::mat2& m){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniformMatrix2fv(o,1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix2fvSafe(const char* l,std::vector<glm::mat2>& m, uint count){ const GLint& o=getUniformLocation(l);if(o==-1)return;std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 4; i++){d.push_back(m[i]);}}glUniformMatrix2fv(o,count,0,&d[0]); }  
        inline void sendUniformMatrix2dSafe(const char* l,glm::dmat2& m){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniformMatrix2dv(o,1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix2dvSafe(const char* l,std::vector<glm::dmat2>& m, uint count){ const GLint& o=getUniformLocation(l);if(o==-1)return;std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 4; i++){d.push_back(m[i]);}}glUniformMatrix2dv(o,count,0,&d[0]); }   
        inline void sendUniformMatrix2fForce(const char* l,glm::mat2& m){ glUniformMatrix2fv(getUniformLocation(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix2fvForce(const char* l,std::vector<glm::mat2>& m,uint count){ std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 4; i++){d.push_back(m[i]);}}glUniformMatrix2fv(getUniformLocation(l),count,0,&d[0]); }  
        inline void sendUniformMatrix2dForce(const char* l,glm::dmat2& m){ glUniformMatrix2dv(getUniformLocation(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix2dvForce(const char* l,std::vector<glm::dmat2>& m,uint count){ std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 4; i++){d.push_back(m[i]);}}glUniformMatrix2dv(getUniformLocation(l),count,0,&d[0]); }   
        
        //Matrix 3x3
        inline void sendUniformMatrix3(const char* l,glm::mat3& m){ glUniformMatrix3fv(getUniformLocationUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix3v(const char* l,std::vector<glm::mat3>& m,uint count){ std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 9; i++){d.push_back(m[i]);}}glUniformMatrix3fv(getUniformLocationUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix3(const char* l,glm::dmat3& m){ glUniformMatrix3dv(getUniformLocationUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix3v(const char* l,std::vector<glm::dmat3>& m,uint count){ std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 9; i++){d.push_back(m[i]);}}glUniformMatrix3dv(getUniformLocationUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix3f(const char* l,glm::mat3& m){ glUniformMatrix3fv(getUniformLocationUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix3fv(const char* l,std::vector<glm::mat3>& m,uint count){ std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 9; i++){d.push_back(m[i]);}}glUniformMatrix3fv(getUniformLocationUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix3d(const char* l,glm::dmat3& m){ glUniformMatrix3dv(getUniformLocationUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix3dv(const char* l,std::vector<glm::dmat3>& m,uint count){ std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 9; i++){d.push_back(m[i]);}}glUniformMatrix3dv(getUniformLocationUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix3fSafe(const char* l,glm::mat3& m){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniformMatrix3fv(o,1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix3fvSafe(const char* l,std::vector<glm::mat3>& m, uint count){ const GLint& o=getUniformLocation(l);if(o==-1)return;std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 9; i++){d.push_back(m[i]);}}glUniformMatrix3fv(o,count,0,&d[0]); }  
        inline void sendUniformMatrix3dSafe(const char* l,glm::dmat3& m){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniformMatrix3dv(o,1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix3dvSafe(const char* l,std::vector<glm::dmat3>& m, uint count){ const GLint& o=getUniformLocation(l);if(o==-1)return;std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 9; i++){d.push_back(m[i]);}}glUniformMatrix3dv(o,count,0,&d[0]); }  
        inline void sendUniformMatrix3fForce(const char* l,glm::mat3& m){ glUniformMatrix3fv(getUniformLocation(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix3fvForce(const char* l,std::vector<glm::mat3>& m,uint count){ std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 9; i++){d.push_back(m[i]);}}glUniformMatrix3fv(getUniformLocation(l),count,0,&d[0]); }  
        inline void sendUniformMatrix3dForce(const char* l,glm::dmat3& m){ glUniformMatrix3dv(getUniformLocation(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix3dvForce(const char* l,std::vector<glm::dmat3>& m,uint count){ std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 9; i++){d.push_back(m[i]);}}glUniformMatrix3dv(getUniformLocation(l),count,0,&d[0]); }   
        
        //Matrix4x4
        inline void sendUniformMatrix4(const char* l,glm::mat4& m){ glUniformMatrix4fv(getUniformLocationUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix4v(const char* l,std::vector<glm::mat4>& m,uint count){ std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 16; i++){d.push_back(m[i]);}}glUniformMatrix4fv(getUniformLocationUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix4(const char* l,glm::dmat4& m){ glUniformMatrix4dv(getUniformLocationUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix4v(const char* l,std::vector<glm::dmat4>& m,uint count){ std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 16; i++){d.push_back(m[i]);}}glUniformMatrix4dv(getUniformLocationUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix4f(const char* l,glm::mat4& m){ glUniformMatrix4fv(getUniformLocationUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix4fv(const char* l,std::vector<glm::mat4>& m,uint count){ std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 16; i++){d.push_back(m[i]);}}glUniformMatrix4fv(getUniformLocationUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix4d(const char* l,glm::dmat4& m){ glUniformMatrix4dv(getUniformLocationUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix4dv(const char* l,std::vector<glm::dmat4>& m,uint count){ std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 16; i++){d.push_back(m[i]);}}glUniformMatrix4dv(getUniformLocationUnsafe(l),count,0,&d[0]); } 
        inline void sendUniformMatrix4fSafe(const char* l,glm::mat4& m){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniformMatrix4fv(o,1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix4fvSafe(const char* l,std::vector<glm::mat4>& m, uint count){ const GLint& o=getUniformLocation(l);if(o==-1)return;std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 16; i++){d.push_back(m[i]);}}glUniformMatrix4fv(o,count,0,&d[0]); }  
        inline void sendUniformMatrix4dSafe(const char* l,glm::dmat4& m){ const GLint& o=getUniformLocation(l);if(o==-1)return;glUniformMatrix4dv(o,1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix4dvSafe(const char* l,std::vector<glm::dmat4>& m, uint count){ const GLint& o=getUniformLocation(l);if(o==-1)return;std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 16; i++){d.push_back(m[i]);}}glUniformMatrix4dv(o,count,0,&d[0]); }  
        inline void sendUniformMatrix4fForce(const char* l,glm::mat4& m){ glUniformMatrix4fv(getUniformLocation(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix4fvForce(const char* l,std::vector<glm::mat4>& m,uint count){ std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 16; i++){d.push_back(m[i]);}}glUniformMatrix4fv(getUniformLocation(l),count,0,&d[0]); }  
        inline void sendUniformMatrix4dForce(const char* l,glm::dmat4& m){ glUniformMatrix4dv(getUniformLocation(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix4dvForce(const char* l,std::vector<glm::dmat4>& m,uint count){ std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 16; i++){d.push_back(m[i]);}}glUniformMatrix4dv(getUniformLocation(l),count,0,&d[0]); }   

        void renderTexture(Texture*,glm::vec2& pos, glm::vec4& col,float angle, glm::vec2& scl, float depth);
        void renderText(std::string& text,Font*, glm::vec2& pos,glm::vec4& color, float angle, glm::vec2& scl, float depth);
        void renderRectangle(glm::vec2& pos, glm::vec4& col, float w, float h, float angle, float depth);
    };
};
#endif
