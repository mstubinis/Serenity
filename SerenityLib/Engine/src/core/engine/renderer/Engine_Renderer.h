#pragma once
#ifndef ENGINE_ENGINE_RENDERER_H
#define ENGINE_ENGINE_RENDERER_H

#include <core/engine/renderer/RenderGraph.h>
#include <core/engine/Engine_GLStateMachine.h>

#include <glm/gtc/type_ptr.hpp>
#include <SFML/Window.hpp>

typedef std::uint32_t uint;

class  Texture;
class  Font;
struct Entity;
struct BufferObject;

struct DepthFunc{enum Func{ 
    Never = GL_NEVER,
    Less = GL_LESS,
    Equal = GL_EQUAL,
    LEqual = GL_LEQUAL,
    Greater = GL_GREATER,
    NotEqual = GL_NOTEQUAL,
    GEqual = GL_GEQUAL,
    Always = GL_ALWAYS,
};};
struct SMAAQualityLevel{enum Level{
    Low,Medium,High,Ultra,
};};
struct HDRAlgorithm{enum Algorithm{
    Reinhard,Filmic,Exposure,Uncharted,
};};
struct AntiAliasingAlgorithm{enum Algorithm{
    None,FXAA,SMAA,
};};

namespace Engine {
    namespace epriv {
        class GBuffer;
        class FramebufferObject;
        class RenderbufferObject;
        class RenderManager;
    };
};

namespace Engine{
    namespace epriv{
        class RenderManager final{
            public:
                epriv::GLStateMachineDataCustom glSM;

                class impl;
                std::unique_ptr<impl> m_i;

                RenderManager(const char* name,uint w,uint h);
                ~RenderManager();

                static uint GLSL_VERSION;
                static uint OPENGL_VERSION;

                static std::vector<bool> OPENGL_EXTENSIONS;

                void _init(const char* name,uint w,uint h);
                void _resize(uint width, uint height);
                void _resizeGbuffer(uint width,uint height);

                void _render(
                    Camera&, const uint fboWidth, const uint fboHeight,bool HUD=true,
                    Entity* ignore=nullptr,const bool mainRenderFunc=true,const GLuint display_fbo=0, const GLuint display_rbo=0
                );
                void _onFullscreen(sf::Window* sfWindow,sf::VideoMode videoMode,const char* winName,uint style,sf::ContextSettings&);
                void _onOpenGLContextCreation(uint width,uint height,uint glslVersion,uint openglVersion);
                void _renderText(Font*,std::string& text,glm::vec2& pos,glm::vec4& color,glm::vec2& scl,float& angle,float& depth);
                void _renderTexture(Texture*,glm::vec2& pos,glm::vec4& color,glm::vec2& scl,float& angle,float& depth);
                void _bindShaderProgram(ShaderP*);
                void _unbindShaderProgram();
                void _bindMaterial(Material*);
                void _unbindMaterial();
                void _genPBREnvMapData(Texture&,uint,uint);
        };
        struct OpenGLExtensionEnum final{
            enum Extension{
                EXT_Ansiotropic_Filtering,
                ARB_Ansiotropic_Filtering,
                EXT_draw_instanced,
                ARB_draw_instanced,
                EXT_separate_shader_objects,
                ARB_separate_shader_objects,
                EXT_explicit_attrib_location,
                ARB_explicit_attrib_location,
                EXT_geometry_shader_4,
                ARB_geometry_shader_4,
                EXT_compute_shader,
                ARB_compute_shader,
                EXT_tessellation_shader,
                ARB_tessellation_shader,
            _TOTAL};
            static bool supported(OpenGLExtensionEnum::Extension e){ return RenderManager::OPENGL_EXTENSIONS[e]; }
        };
    };
    namespace Renderer{
        namespace Settings{

            void setGamma(float g);
            float getGamma();

            void clear(bool color = true, bool depth = true, bool stencil = true);
            void cullFace(uint state);

            void setAntiAliasingAlgorithm(AntiAliasingAlgorithm::Algorithm);

            void enableDrawPhysicsInfo(bool b = true);
            void disableDrawPhysicsInfo();

            namespace General {
                void enable1(bool b = true);
                void disable1();
                bool enabled1();
            };
            namespace DepthOfField {
                void enable(bool b = true);
                void disable();
                bool enabled();
                float getFocus();
                void setFocus(float);
                float getBias();
                void setBias(float);
                float getBlurRadius();
                void setBlurRadius(float);
            }
            namespace Fog{
                void enable(bool b = true);
                void disable();
                bool enabled();
                void setColor(glm::vec4& color);
                void setColor(float r,float g,float b,float a);
                void setNullDistance(float d);
                void setBlendDistance(float d);
                float getNullDistance();
                float getBlendDistance();
            };
            namespace SMAA{
                void setThreshold(float f);
                void setSearchSteps(uint s);
                void disableCornerDetection();
                void enableCornerDetection(uint c = 25);
                void disableDiagonalDetection();
                void enableDiagonalDetection(uint d = 8);
                void setQuality(SMAAQualityLevel::Level l);
                void setPredicationThreshold(float f);
                void setPredicationScale(float f);
                void setPredicationStrength(float s);
                void setReprojectionScale(float s);
                void enablePredication(bool b = true);
                void disablePredication();
                void enableReprojection(bool b = true);
                void disableReprojection();
            };
            namespace FXAA{
                void setReduceMin(float r);
                float getReduceMin();
                void setReduceMul(float r);
                float getReduceMul();
                void setSpanMax(float r);
                float getSpanMax();
            };
            namespace HDR{
                bool enabled();
                void enable(bool b = true);
                void disable();
                float getExposure();
                void setExposure(float e);
                void setAlgorithm(HDRAlgorithm::Algorithm a);
            };
            namespace Bloom{
                uint getNumPasses();
                void setNumPasses(uint);
                void enable(bool b = true);
                void disable();
                bool enabled();
                float getThreshold();
                void setThreshold(float t);
                float getExposure();
                void setExposure(float e);
                float getBlurRadius();
                void setBlurRadius(float r);
                float getBlurStrength();
                void setBlurStrength(float r);
                float getScale();
                void setScale(float s);
            };
            namespace GodRays{
                bool enabled();
                void enable(bool b);
                void disable();
                float getExposure();
                void setExposure(float e);
                float getFactor();
                void setFactor(float f);
                float getDecay();
                void setDecay(float d);
                float getDensity();
                void setDensity(float d);
                float getWeight();
                void setWeight(float w);
                uint getSamples();
                void setSamples(uint s);
                float getFOVDegrees();
                void setFOVDegrees(float d);
                float getAlphaFalloff();
                void setAlphaFalloff(float a);
                void setObject(Entity*);
                Entity* getObject();
            };
            namespace SSAO{
                bool enabled();
                void enable(bool b = true);
                void disable();
                void enableBlur(bool b = true);
                void disableBlur();
                float getBlurRadius();
                void setBlurRadius(float r);
                float getBlurStrength();
                void setBlurStrength(float s);
                float getIntensity();
                void setIntensity(float i);
                float getRadius();
                void setRadius(float r);
                float getScale();
                void setScale(float s);
                float getBias();
                void setBias(float b);
                uint getSamples();
                void setSamples(uint s);
            };
            namespace Lighting{
                void enable(bool b = true);
                void disable();
                float getGIContributionGlobal();
                void setGIContributionGlobal(float giGlobal);
                float getGIContributionDiffuse();
                void setGIContributionDiffuse(float giDiffuse);
                float getGIContributionSpecular();
                void setGIContributionSpecular(float giSpecular);
                void setGIContribution(float global, float diffuse, float specular);
            };
        };
        void renderFullscreenQuad(uint width, uint height, uint startX = 0, uint startY = 0);
        void renderFullscreenTriangle(uint width,uint height, uint startX = 0, uint startY = 0);

        void renderFullscreenQuad();
        void renderFullscreenTriangle();

        inline const GLint getUniformLoc(const char* location);
        inline const GLint& getUniformLocUnsafe(const char* location);

        void setDepthFunc(DepthFunc::Func);
        void setViewport(uint x, uint y, uint width, uint height);
        void bindFBO(GLuint fbo);
        void bindFBO(epriv::FramebufferObject& rbo);
        void bindRBO(GLuint rbo);
        void bindRBO(epriv::RenderbufferObject& rbo);
        void bindReadFBO(GLuint fbo);
        void bindDrawFBO(GLuint fbo);

        void bindTexture(GLuint _textureType,GLuint _textureObject);
        void bindVAO(const GLuint _vaoObject);
        void genAndBindTexture(GLuint _textureType,GLuint& _textureObject);
        void genAndBindVAO(GLuint& _vaoObject);
        void deleteVAO(GLuint& _vaoObject);
        void colorMask(bool r, bool g, bool b, bool a);
        void clearColor(float r, float g, float b, float a);

        void sendTexture(const char* location,Texture& texture,const int slot);
        void sendTexture(const char* location,const GLuint textureAddress,const int slot,const GLuint glTextureType);
        void sendTextureSafe(const char* location,Texture& texture,const int slot);
        void sendTextureSafe(const char* location,const GLuint textureAddress,const int slot,const GLuint glTextureType);

        void unbindFBO();
        void unbindRBO();
        void unbindReadFBO();
        void unbindDrawFBO();
        
        void renderTexture(Texture&, const glm::vec2& pos, const glm::vec4& col, float angle, const glm::vec2& scl, float depth);
        void renderText(const std::string& text, Font&, const glm::vec2& pos, const glm::vec4& color, float angle, const glm::vec2& scl, float depth);
        void renderRectangle(const glm::vec2& pos, const glm::vec4& col, float w, float h, float angle, float depth);

        #pragma region UniformSending
        //Uniform 1
        inline void sendUniform1(const char* l,double x){ glUniform1d(getUniformLocUnsafe(l),x); }
        inline void sendUniform1(const char* l,int x){ glUniform1i(getUniformLocUnsafe(l),x); }
        inline void sendUniform1(const char* l,float x){ glUniform1f(getUniformLocUnsafe(l),x); }
        inline void sendUniform1Safe(const char* l,double x){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform1d(o,x); }
        inline void sendUniform1Safe(const char* l,int x){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform1i(o,x); }
        inline void sendUniform1Safe(const char* l,float x){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform1f(o,x); }
        inline void sendUniform1Force(const char* l,double x){ glUniform1d(getUniformLoc(l),x); }
        inline void sendUniform1Force(const char* l,int x){ glUniform1i(getUniformLoc(l),x); }
        inline void sendUniform1Force(const char* l,float x){ glUniform1f(getUniformLoc(l),x); }
        
        //Uniform 2
        //vectors
        inline void sendUniform2(const char* l, const glm::dvec2& v){ glUniform2d(getUniformLocUnsafe(l),v.x,v.y); }
        inline void sendUniform2(const char* l, const glm::ivec2& v){ glUniform2i(getUniformLocUnsafe(l),v.x,v.y); }
        inline void sendUniform2(const char* l, const glm::vec2& v){ glUniform2f(getUniformLocUnsafe(l),v.x,v.y); }
        inline void sendUniform2Safe(const char* l, const glm::dvec2& v){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2d(o,v.x,v.y); }
        inline void sendUniform2Safe(const char* l, const glm::ivec2& v){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2i(o,v.x,v.y); }
        inline void sendUniform2Safe(const char* l, const glm::vec2& v){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2f(o,v.x,v.y); }
        inline void sendUniform2Force(const char* l, const glm::dvec2& v){ glUniform2d(getUniformLoc(l),v.x,v.y); }
        inline void sendUniform2Force(const char* l, const glm::ivec2& v){ glUniform2i(getUniformLoc(l),v.x,v.y); }
        inline void sendUniform2Force(const char* l, const glm::vec2& v){ glUniform2f(getUniformLoc(l),v.x,v.y); }
        //seperate
        inline void sendUniform2(const char* l,double x,double y){ glUniform2d(getUniformLocUnsafe(l),x,y); }
        inline void sendUniform2(const char* l,int x,int y){ glUniform2i(getUniformLocUnsafe(l),x,y); }
        inline void sendUniform2(const char* l,float x,float y){ glUniform2f(getUniformLocUnsafe(l),x,y); } 
        inline void sendUniform2Safe(const char* l,double x,double y){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2d(o,x,y); }
        inline void sendUniform2Safe(const char* l,int x,int y){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2i(o,x,y); }
        inline void sendUniform2Safe(const char* l,float x,float y){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2f(o,x,y); }
        inline void sendUniform2Force(const char* l,double x,double y){ glUniform2d(getUniformLoc(l),x,y); }
        inline void sendUniform2Force(const char* l,int x,int y){ glUniform2i(getUniformLoc(l),x,y); }
        inline void sendUniform2Force(const char* l,float x,float y){ glUniform2f(getUniformLoc(l),x,y); }
        //vector and array of values
        inline void sendUniform2v(const char* l, const std::vector<glm::dvec2>& d,uint i){ glUniform2dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2v(const char* l, const std::vector<glm::ivec2>& d,uint i){ glUniform2iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2v(const char* l, const std::vector<glm::vec2>& d,uint i){ glUniform2fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2v(const char* l,glm::dvec2* d,uint i){ glUniform2dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2v(const char* l,glm::ivec2* d,uint i){ glUniform2iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2v(const char* l,glm::vec2* d,uint i){ glUniform2fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2vSafe(const char* l, const std::vector<glm::vec2>& d,uint i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2fv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform2vSafe(const char* l,glm::vec2* d,uint i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2fv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform2vForce(const char* l, const std::vector<glm::vec2>& d,uint i){ glUniform2fv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2vForce(const char* l,glm::vec2* d,uint i){ glUniform2fv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2vSafe(const char* l, const std::vector<glm::dvec2>& d,uint i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2dv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform2vSafe(const char* l,glm::dvec2* d,uint i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2dv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform2vForce(const char* l, const std::vector<glm::dvec2>& d,uint i){ glUniform2dv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2vForce(const char* l,glm::dvec2* d,uint i){ glUniform2dv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2vSafe(const char* l, const std::vector<glm::ivec2>& d,uint i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2iv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform2vSafe(const char* l,glm::ivec2* d,uint i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2iv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform2vForce(const char* l, const std::vector<glm::ivec2>& d,uint i){ glUniform2iv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2vForce(const char* l,glm::ivec2* d,uint i){ glUniform2iv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        
        //Uniform 3
        //vectors
        inline void sendUniform3(const char* l, const glm::dvec3& v){ glUniform3d(getUniformLocUnsafe(l),v.x,v.y,v.z); }
        inline void sendUniform3(const char* l, const glm::ivec3& v){ glUniform3i(getUniformLocUnsafe(l),v.x,v.y,v.z); }
        inline void sendUniform3(const char* l, const glm::vec3& v){ glUniform3f(getUniformLocUnsafe(l),v.x,v.y,v.z); }
        inline void sendUniform3Safe(const char* l, const glm::dvec3& v){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3d(o,v.x,v.y,v.z); }
        inline void sendUniform3Safe(const char* l, const glm::ivec3& v){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3i(o,v.x,v.y,v.z); }
        inline void sendUniform3Safe(const char* l, const glm::vec3& v){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3f(o,v.x,v.y,v.z); }
        inline void sendUniform3Force(const char* l, const glm::dvec3& v){ glUniform3d(getUniformLoc(l),v.x,v.y,v.z); }
        inline void sendUniform3Force(const char* l, const glm::ivec3& v){ glUniform3i(getUniformLoc(l),v.x,v.y,v.z); }
        inline void sendUniform3Force(const char* l, const glm::vec3& v){ glUniform3f(getUniformLoc(l),v.x,v.y,v.z); }
        //seperate
        inline void sendUniform3(const char* l,double x,double y,double z){ glUniform3d(getUniformLocUnsafe(l),x,y,z); }
        inline void sendUniform3(const char* l,int x,int y,int z){ glUniform3i(getUniformLocUnsafe(l),x,y,z); }
        inline void sendUniform3(const char* l,float x,float y,float z){ glUniform3f(getUniformLocUnsafe(l),x,y,z); }
        inline void sendUniform3Safe(const char* l,double x,double y,double z){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3d(o,x,y,z); }
        inline void sendUniform3Safe(const char* l,int x,int y,int z){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3i(o,x,y,z); }
        inline void sendUniform3Safe(const char* l,float x,float y,float z){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3f(o,x,y,z); }
        inline void sendUniform3Force(const char* l,double x,double y,double z){ glUniform3d(getUniformLoc(l),x,y,z); }
        inline void sendUniform3Force(const char* l,int x,int y,int z){ glUniform3i(getUniformLoc(l),x,y,z); }
        inline void sendUniform3Force(const char* l,float x,float y,float z){ glUniform3f(getUniformLoc(l),x,y,z); }
        //vector and array of values
        inline void sendUniform3v(const char* l, const std::vector<glm::dvec3>& d,uint i){ glUniform3dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3v(const char* l, const std::vector<glm::ivec3>& d,uint i){ glUniform3iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3v(const char* l, const std::vector<glm::vec3>& d,uint i){ glUniform3fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3v(const char* l,glm::dvec3* d,uint i){ glUniform3dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3v(const char* l,glm::ivec3* d,uint i){ glUniform3iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3v(const char* l,glm::vec3* d,uint i){ glUniform3fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); } 
        inline void sendUniform3vSafe(const char* l, const std::vector<glm::vec3>& d,uint i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3fv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform3vSafe(const char* l,glm::vec3* d,uint i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3fv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform3vForce(const char* l, const std::vector<glm::vec3>& d,uint i){ glUniform3fv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3vForce(const char* l,glm::vec3* d,uint i){ glUniform3fv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3vSafe(const char* l, const std::vector<glm::dvec3>& d,uint i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3dv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform3vSafe(const char* l,glm::dvec3* d,uint i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3dv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform3vForce(const char* l, const std::vector<glm::dvec3>& d,uint i){ glUniform3dv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3vForce(const char* l,glm::dvec3* d,uint i){ glUniform3dv(getUniformLoc(l),i,glm::value_ptr(d[0])); } 
        inline void sendUniform3vSafe(const char* l, const std::vector<glm::ivec3>& d,uint i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3iv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform3vSafe(const char* l,glm::ivec3* d,uint i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3iv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform3vForce(const char* l, const std::vector<glm::ivec3>& d,uint i){ glUniform3iv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3vForce(const char* l,glm::ivec3* d,uint i){ glUniform3iv(getUniformLoc(l),i,glm::value_ptr(d[0])); }  
        
        //Uniform 4
        //vectors
        inline void sendUniform4(const char* l, const glm::dvec4& v){ glUniform4d(getUniformLocUnsafe(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4(const char* l, const glm::ivec4& v){ glUniform4i(getUniformLocUnsafe(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4(const char* l, const glm::vec4& v){ glUniform4f(getUniformLocUnsafe(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4Safe(const char* l, const glm::dvec4& v){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4d(o,v.x,v.y,v.z,v.w); }
        inline void sendUniform4Safe(const char* l, const glm::ivec4& v){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4i(o,v.x,v.y,v.z,v.w); }
        inline void sendUniform4Safe(const char* l, const glm::vec4& v){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4f(o,v.x,v.y,v.z,v.w); }
        inline void sendUniform4Force(const char* l, const glm::dvec4& v){ glUniform4d(getUniformLoc(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4Force(const char* l, const glm::ivec4& v){ glUniform4i(getUniformLoc(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4Force(const char* l, const glm::vec4& v){ glUniform4f(getUniformLoc(l),v.x,v.y,v.z,v.w); }
        //seperate
        inline void sendUniform4(const char* l,double x,double y,double z,double w){ glUniform4d(getUniformLocUnsafe(l),x,y,z,w); }
        inline void sendUniform4(const char* l,int x,int y,int z,int w){ glUniform4i(getUniformLocUnsafe(l),x,y,z,w); }
        inline void sendUniform4(const char* l,float x,float y,float z,float w){ glUniform4f(getUniformLocUnsafe(l),x,y,z,w); }
        inline void sendUniform4Safe(const char* l,double x,double y,double z,double w){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4d(o,x,y,z,w); }
        inline void sendUniform4Safe(const char* l,int x,int y,int z,int w){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4i(o,x,y,z,w); }
        inline void sendUniform4Safe(const char* l,float x,float y,float z,float w){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4f(o,x,y,z,w); }
        inline void sendUniform4Force(const char* l,double x,double y,double z,double w){ glUniform4d(getUniformLoc(l),x,y,z,w); }
        inline void sendUniform4Force(const char* l,int x,int y,int z,int w){ glUniform4i(getUniformLoc(l),x,y,z,w); }
        inline void sendUniform4Force(const char* l,float x,float y,float z,float w){ glUniform4f(getUniformLoc(l),x,y,z,w); }
        //vector / array of values
        inline void sendUniform4v(const char* l, const std::vector<glm::dvec4>& d,uint i){ glUniform4dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4v(const char* l, const std::vector<glm::ivec4>& d,uint i){ glUniform4iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4v(const char* l, const std::vector<glm::vec4>& d,uint i){ glUniform4fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4v(const char* l,glm::dvec4* d,uint i){ glUniform4dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4v(const char* l,glm::ivec4* d,uint i){ glUniform4iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4v(const char* l,glm::vec4* d,uint i){ glUniform4fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4vSafe(const char* l, const std::vector<glm::vec4>& d,uint i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4fv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform4vSafe(const char* l,glm::vec4* d,uint i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4fv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform4vForce(const char* l, const std::vector<glm::vec4>& d,uint i){ glUniform4fv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4vForce(const char* l,glm::vec4* d,uint i){ glUniform4fv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4vSafe(const char* l, const std::vector<glm::dvec4>& d,uint i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4dv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform4vSafe(const char* l,glm::dvec4* d,uint i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4dv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform4vForce(const char* l, const std::vector<glm::dvec4>& d,uint i){ glUniform4dv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4vForce(const char* l,glm::dvec4* d,uint i){ glUniform4dv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4vSafe(const char* l, const std::vector<glm::ivec4>& d,uint i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4iv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform4vSafe(const char* l,glm::ivec4* d,uint i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4iv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform4vForce(const char* l, const std::vector<glm::ivec4>& d,uint i){ glUniform4iv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4vForce(const char* l,glm::ivec4* d,uint i){ glUniform4iv(getUniformLoc(l),i,glm::value_ptr(d[0])); }

        //Matrix 2x2
        inline void sendUniformMatrix2(const char* l, const glm::mat2& m){ glUniformMatrix2fv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix2v(const char* l, const std::vector<glm::mat2>& m,uint count){ std::vector<float> d; d.reserve(m.size() * 4); for(auto& ma:m){const float* m = glm::value_ptr(ma);for(uint i = 0; i < 4; ++i){d.push_back(m[i]);}}glUniformMatrix2fv(getUniformLocUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix2(const char* l, const glm::dmat2& m){ glUniformMatrix2dv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix2v(const char* l, const std::vector<glm::dmat2>& m,uint count){ std::vector<double> d; d.reserve(m.size() * 4); for(auto& ma:m){ const double* m = glm::value_ptr(ma);for(uint i = 0; i < 4; ++i){d.push_back(m[i]);}}glUniformMatrix2dv(getUniformLocUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix2Safe(const char* l, const glm::mat2& m){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniformMatrix2fv(o,1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix2vSafe(const char* l, const std::vector<glm::mat2>& m, uint count){ const auto& o=getUniformLoc(l);if(o==-1)return;std::vector<float> d; d.reserve(m.size() * 4); for(auto& ma:m){ const float* m = glm::value_ptr(ma);for(uint i = 0; i < 4; ++i){d.push_back(m[i]);}}glUniformMatrix2fv(o,count,0,&d[0]); }
        inline void sendUniformMatrix2Safe(const char* l, const glm::dmat2& m){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniformMatrix2dv(o,1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix2vSafe(const char* l, const std::vector<glm::dmat2>& m, uint count){ const auto& o=getUniformLoc(l);if(o==-1)return;std::vector<double> d; d.reserve(m.size() * 4); for(auto& ma:m){ const double* m = glm::value_ptr(ma);for(uint i = 0; i < 4; ++i){d.push_back(m[i]);}}glUniformMatrix2dv(o,count,0,&d[0]); }
        inline void sendUniformMatrix2Force(const char* l, const glm::mat2& m){ glUniformMatrix2fv(getUniformLoc(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix2vForce(const char* l, const std::vector<glm::mat2>& m,uint count){ std::vector<float> d; d.reserve(m.size() * 4); for(auto& ma:m){ const float* m = glm::value_ptr(ma);for(uint i = 0; i < 4; ++i){d.push_back(m[i]);}}glUniformMatrix2fv(getUniformLoc(l),count,0,&d[0]); }
        inline void sendUniformMatrix2Force(const char* l, const glm::dmat2& m){ glUniformMatrix2dv(getUniformLoc(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix2vForce(const char* l, const std::vector<glm::dmat2>& m,uint count){ std::vector<double> d; d.reserve(m.size() * 4); for(auto& ma:m){ const double* m = glm::value_ptr(ma);for(uint i = 0; i < 4; ++i){d.push_back(m[i]);}}glUniformMatrix2dv(getUniformLoc(l),count,0,&d[0]); }
        
        //Matrix 3x3
        inline void sendUniformMatrix3(const char* l, const glm::mat3& m){ glUniformMatrix3fv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix3v(const char* l, const std::vector<glm::mat3>& m,uint count){ std::vector<float> d; d.reserve(m.size() * 9); for(auto& ma:m){ const float* m = glm::value_ptr(ma);for(uint i = 0; i < 9; ++i){d.push_back(m[i]);}}glUniformMatrix3fv(getUniformLocUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix3(const char* l, const glm::dmat3& m){ glUniformMatrix3dv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix3v(const char* l, const std::vector<glm::dmat3>& m,uint count){ std::vector<double> d; d.reserve(m.size() * 9); for(auto& ma:m){ const double* m = glm::value_ptr(ma);for(uint i = 0; i < 9; ++i){d.push_back(m[i]);}}glUniformMatrix3dv(getUniformLocUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix3Safe(const char* l, const glm::mat3& m){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniformMatrix3fv(o,1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix3vSafe(const char* l, const std::vector<glm::mat3>& m, uint count){ const auto& o=getUniformLoc(l);if(o==-1)return;std::vector<float> d; d.reserve(m.size() * 9); for(auto& ma:m){ const float* m = glm::value_ptr(ma);for(uint i = 0; i < 9; ++i){d.push_back(m[i]);}}glUniformMatrix3fv(o,count,0,&d[0]); }
        inline void sendUniformMatrix3Safe(const char* l, const glm::dmat3& m){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniformMatrix3dv(o,1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix3vSafe(const char* l, const std::vector<glm::dmat3>& m, uint count){ const auto& o=getUniformLoc(l);if(o==-1)return;std::vector<double> d; d.reserve(m.size() * 9); for(auto& ma:m){ const double* m = glm::value_ptr(ma);for(uint i = 0; i < 9; ++i){d.push_back(m[i]);}}glUniformMatrix3dv(o,count,0,&d[0]); }
        inline void sendUniformMatrix3Force(const char* l, const glm::mat3& m){ glUniformMatrix3fv(getUniformLoc(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix3vForce(const char* l, const std::vector<glm::mat3>& m,uint count){ std::vector<float> d; d.reserve(m.size() * 9); for(auto& ma:m){ const float* m = glm::value_ptr(ma);for(uint i = 0; i < 9; ++i){d.push_back(m[i]);}}glUniformMatrix3fv(getUniformLoc(l),count,0,&d[0]); }
        inline void sendUniformMatrix3Force(const char* l, const glm::dmat3& m){ glUniformMatrix3dv(getUniformLoc(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix3vForce(const char* l, const std::vector<glm::dmat3>& m, uint count) { std::vector<double> d; d.reserve(m.size() * 9); for (auto& ma : m) { const double* m = glm::value_ptr(ma); for (uint i = 0; i < 9; ++i) { d.push_back(m[i]); } }glUniformMatrix3dv(getUniformLoc(l), count, 0, &d[0]); }
        
        //Matrix4x4
        inline void sendUniformMatrix4(const char* l,const glm::mat4& m){ glUniformMatrix4fv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix4v(const char* l, const std::vector<glm::mat4>& m,uint count){ std::vector<float> d; d.reserve(m.size() * 16);for(auto& ma:m){ auto* m = glm::value_ptr(ma);for(uint i = 0; i < 16; ++i){d.push_back(m[i]);}}glUniformMatrix4fv(getUniformLocUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix4(const char* l, const glm::dmat4& m){ glUniformMatrix4dv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix4v(const char* l, const std::vector<glm::dmat4>& m,uint count){ std::vector<double> d; d.reserve(m.size() * 16);for(auto& ma:m){ auto* m = glm::value_ptr(ma);for(uint i = 0; i < 16; ++i){d.push_back(m[i]);}}glUniformMatrix4dv(getUniformLocUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix4Safe(const char* l, const glm::mat4& m){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniformMatrix4fv(o,1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix4vSafe(const char* l, const std::vector<glm::mat4>& m, uint count){ const auto& o=getUniformLoc(l);if(o==-1)return;std::vector<float> d; d.reserve(m.size() * 16);for(auto& ma:m){ auto* m = glm::value_ptr(ma);for(uint i = 0; i < 16; ++i){d.push_back(m[i]);}}glUniformMatrix4fv(o,count,0,&d[0]); }
        inline void sendUniformMatrix4Safe(const char* l, const glm::dmat4& m){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniformMatrix4dv(o,1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix4vSafe(const char* l, const std::vector<glm::dmat4>& m, uint count){ const auto& o=getUniformLoc(l);if(o==-1)return;std::vector<double> d; d.reserve(m.size() * 16);for(auto& ma:m){ auto* m = glm::value_ptr(ma);for(uint i = 0; i < 16; ++i){d.push_back(m[i]);}}glUniformMatrix4dv(o,count,0,&d[0]); }
        inline void sendUniformMatrix4Force(const char* l, const glm::mat4& m){ glUniformMatrix4fv(getUniformLoc(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix4vForce(const char* l, const std::vector<glm::mat4>& m,uint count){ std::vector<float> d; d.reserve(m.size() * 16);for(auto& ma:m){ auto* m = glm::value_ptr(ma);for(uint i = 0; i < 16; ++i){d.push_back(m[i]);}}glUniformMatrix4fv(getUniformLoc(l),count,0,&d[0]); }
        inline void sendUniformMatrix4Force(const char* l, const glm::dmat4& m){ glUniformMatrix4dv(getUniformLoc(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix4vForce(const char* l, const std::vector<glm::dmat4>& m,uint count){ std::vector<double> d; d.reserve(m.size() * 16);for(auto& ma:m){ auto* m = glm::value_ptr(ma);for(uint i = 0; i < 16; ++i){d.push_back(m[i]);}}glUniformMatrix4dv(getUniformLoc(l),count,0,&d[0]); }
        #pragma endregion
    };
};
#endif
