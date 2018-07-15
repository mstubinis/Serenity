#pragma once
#ifndef ENGINE_ENGINE_RENDERER_H
#define ENGINE_ENGINE_RENDERER_H

#include "Engine_Math.h"
#include "Engine_GLStateMachine.h"
#include "ShaderProgram.h"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <SFML/Window.hpp>

typedef unsigned int uint;

class Texture;
class Font;
class Camera;
class Entity;

class SMAAQualityLevel{public: enum Level{
    Low,Medium,High,Ultra
};};
class HDRAlgorithm{public: enum Algorithm{
    REINHARD,FILMIC,EXPOSURE,UNCHARTED
};};
class AntiAliasingAlgorithm{public: enum Algorithm{
    None,FXAA,SMAA
};};
namespace Engine{
    namespace epriv{
        class GBuffer;
        class FramebufferObject;
        class RenderbufferObject;

        class RenderManager final{
            private:
                class impl;
            public:
                std::unique_ptr<impl> m_i;

                RenderManager(const char* name,uint w,uint h);
                ~RenderManager();

                void _init(const char* name,uint w,uint h);
                void _resize(uint width, uint height);
                void _resizeGbuffer(uint width,uint height);

                void _render(
                    epriv::GBuffer*,Camera*,uint fboWidth,uint fboHeight,
                    bool ssao=true,bool godRays=true,bool AA=true,bool HUD=true,
                    Entity* ignore=nullptr,bool mainRenderFunc=true,GLuint display_fbo=0,GLuint display_rbo=0
                );
                void _render(
                    Camera*,uint fboWidth,uint fboHeight,
                    bool ssao=true,bool godRays=true,bool AA=true,bool HUD=true,
                    Entity* ignore=nullptr,bool mainRenderFunc=true,GLuint display_fbo=0,GLuint display_rbo=0
                );
                void _onFullscreen(sf::Window* sfWindow,sf::VideoMode videoMode,const char* winName,uint style,sf::ContextSettings&);
                void _onOpenGLContextCreation(uint width,uint height);
                void _renderText(Font*,std::string text,glm::vec2 pos,glm::vec4 color,glm::vec2 scl,float angle,float depth);
                void _renderTexture(Texture*,glm::vec2 pos,glm::vec4 color,glm::vec2 scl,float angle,float depth);
                void _addShaderToStage(ShaderP*,uint stage);
                void _bindShaderProgram(ShaderP*);
                bool _bindMaterial(Material*);
                bool _unbindMaterial();
                void _genPBREnvMapData(Texture*,uint,uint);
        };
    };
    namespace Renderer{
        namespace Settings{

            void setGamma(float g);  float getGamma();

            void clear(bool color = true, bool depth = true, bool stencil = true);
            void cullFace(uint state);

            void setAntiAliasingAlgorithm(AntiAliasingAlgorithm::Algorithm);

            void enableDrawPhysicsInfo(bool b = true);   void disableDrawPhysicsInfo();

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
                void setReduceMin(float r);   float getReduceMin();
                void setReduceMul(float r);   float getReduceMul();
                void setSpanMax(float r);     float getSpanMax();
            };
            namespace HDR{
                bool enabled();
                void enable(bool b = true);   void disable();
                float getExposure();          void setExposure(float e);
                void setAlgorithm(HDRAlgorithm::Algorithm a);
            };
            namespace Bloom{
                void enable(bool b = true);   void disable();
                float getRadius();            void setRadius(float r);
                float getStrength();          void setStrength(float r);
            };
            namespace GodRays{
                bool enabled();
                void enable(bool b);      void disable();
                float getExposure();      void setExposure(float e);
                float getDecay();         void setDecay(float d);
                float getDensity();       void setDensity(float d);
                float getWeight();        void setWeight(float w);
                uint getSamples();        void setSamples(uint s);
                float getFOVDegrees();    void setFOVDegrees(float d);
                float getAlphaFalloff();  void setAlphaFalloff(float a);
				void setObject(uint&);
				void setObject(Entity*);
				Entity* getObject();
            };
            namespace SSAO{
                bool enabled();
                void enable(bool b = true);       void disable();
                void enableBlur(bool b = true);   void disableBlur();
                float getBlurStrength();          void setBlurStrength(float s);
                float getIntensity();             void setIntensity(float i);
                float getRadius();                void setRadius(float r);
                float getScale();                 void setScale(float s);
                float getBias();                  void setBias(float b);
                uint getSamples();                void setSamples(uint s);
            };
            namespace Lighting{
                void enable(bool b = true);    void disable();
            };
        };
        void renderFullscreenQuad(uint width, uint height);
        void renderFullscreenTriangle(uint width,uint height);

        inline const GLint getUniformLoc(const char* location);
        inline const GLint& getUniformLocUnsafe(const char* location);

        void setViewport(uint x, uint y, uint width, uint height);
        void bindFBO(GLuint);
        void bindFBO(epriv::FramebufferObject*);
        void bindRBO(GLuint);
        void bindRBO(epriv::RenderbufferObject*);
        void bindReadFBO(GLuint);
        void bindDrawFBO(GLuint);
        void bindTexture(const char* location,Texture*,uint slot);
        void bindTexture(const char* location,GLuint textureAddress,uint slot,GLuint type);
        void bindTextureSafe(const char* location,Texture*,uint slot);
        void bindTextureSafe(const char* location,GLuint textureAddress,uint slot,GLuint type);
        void unbindTexture(uint slot,Texture*);
        void unbindTexture2D(uint slot);
        void unbindTextureCubemap(uint slot);
        void unbindFBO();
        void unbindRBO();
        void unbindReadFBO();
        void unbindDrawFBO();
        
        //Uniform 1
        //seperate (dont need vectors for 1 component ;) )
        inline void sendUniform1(const char* l,double x){ glUniform1d(getUniformLocUnsafe(l),x); }
        inline void sendUniform1(const char* l,int x){ glUniform1i(getUniformLocUnsafe(l),x); }
        inline void sendUniform1(const char* l,float x){ glUniform1f(getUniformLocUnsafe(l),x); }
        inline void sendUniform1d(const char* l,double x){ glUniform1d(getUniformLocUnsafe(l),x); }
        inline void sendUniform1i(const char* l,int x){ glUniform1i(getUniformLocUnsafe(l),x); }
        inline void sendUniform1f(const char* l,float x){ glUniform1f(getUniformLocUnsafe(l),x); }
        inline void sendUniform1Safe(const char* l,double x){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform1d(o,x); }
        inline void sendUniform1Safe(const char* l,int x){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform1i(o,x); }
        inline void sendUniform1Safe(const char* l,float x){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform1f(o,x); }
        inline void sendUniform1dSafe(const char* l,double x){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform1d(o,x); }
        inline void sendUniform1iSafe(const char* l,int x){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform1i(o,x); }
        inline void sendUniform1fSafe(const char* l,float x){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform1f(o,x); }
        inline void sendUniform1Force(const char* l,double x){ glUniform1d(getUniformLoc(l),x); }
        inline void sendUniform1Force(const char* l,int x){ glUniform1i(getUniformLoc(l),x); }
        inline void sendUniform1Force(const char* l,float x){ glUniform1f(getUniformLoc(l),x); }
        inline void sendUniform1dForce(const char* l,double x){ glUniform1d(getUniformLoc(l),x); }
        inline void sendUniform1iForce(const char* l,int x){ glUniform1i(getUniformLoc(l),x); }
        inline void sendUniform1fForce(const char* l,float x){ glUniform1f(getUniformLoc(l),x); }
        
        //Uniform 2
        //vectors
        inline void sendUniform2(const char* l,glm::dvec2& v){ glUniform2d(getUniformLocUnsafe(l),v.x,v.y); }
        inline void sendUniform2(const char* l,glm::ivec2& v){ glUniform2i(getUniformLocUnsafe(l),v.x,v.y); }
        inline void sendUniform2(const char* l,glm::vec2& v){ glUniform2f(getUniformLocUnsafe(l),v.x,v.y); }
        inline void sendUniform2d(const char* l,glm::dvec2& v){ glUniform2d(getUniformLocUnsafe(l),v.x,v.y); }
        inline void sendUniform2i(const char* l,glm::ivec2& v){ glUniform2i(getUniformLocUnsafe(l),v.x,v.y); }
        inline void sendUniform2f(const char* l,glm::vec2& v){ glUniform2f(getUniformLocUnsafe(l),v.x,v.y); }
        inline void sendUniform2Safe(const char* l,glm::dvec2& v){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform2d(o,v.x,v.y); }
        inline void sendUniform2Safe(const char* l,glm::ivec2& v){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform2i(o,v.x,v.y); }
        inline void sendUniform2Safe(const char* l,glm::vec2& v){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform2f(o,v.x,v.y); }
        inline void sendUniform2dSafe(const char* l,glm::dvec2& v){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform2d(o,v.x,v.y); }
        inline void sendUniform2iSafe(const char* l,glm::ivec2& v){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform2i(o,v.x,v.y); }
        inline void sendUniform2fSafe(const char* l,glm::vec2& v){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform2f(o,v.x,v.y); }
        inline void sendUniform2Force(const char* l,glm::dvec2& v){ glUniform2d(getUniformLoc(l),v.x,v.y); }
        inline void sendUniform2Force(const char* l,glm::ivec2& v){ glUniform2i(getUniformLoc(l),v.x,v.y); }
        inline void sendUniform2Force(const char* l,glm::vec2& v){ glUniform2f(getUniformLoc(l),v.x,v.y); }
        inline void sendUniform2dForce(const char* l,glm::dvec2& v){ glUniform2d(getUniformLoc(l),v.x,v.y); }
        inline void sendUniform2iForce(const char* l,glm::ivec2& v){ glUniform2i(getUniformLoc(l),v.x,v.y); }
        inline void sendUniform2fForce(const char* l,glm::vec2& v){ glUniform2f(getUniformLoc(l),v.x,v.y); } 
        //seperate
        inline void sendUniform2(const char* l,double x,double y){ glUniform2d(getUniformLocUnsafe(l),x,y); }
        inline void sendUniform2(const char* l,int x,int y){ glUniform2i(getUniformLocUnsafe(l),x,y); }
        inline void sendUniform2(const char* l,float x,float y){ glUniform2f(getUniformLocUnsafe(l),x,y); } 
        inline void sendUniform2d(const char* l,double x,double y){ glUniform2d(getUniformLocUnsafe(l),x,y); }
        inline void sendUniform2i(const char* l,int x,int y){ glUniform2i(getUniformLocUnsafe(l),x,y); }
        inline void sendUniform2f(const char* l,float x,float y){ glUniform2f(getUniformLocUnsafe(l),x,y); }
        inline void sendUniform2Safe(const char* l,double x,double y){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform2d(o,x,y); }
        inline void sendUniform2Safe(const char* l,int x,int y){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform2i(o,x,y); }
        inline void sendUniform2Safe(const char* l,float x,float y){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform2f(o,x,y); }
        inline void sendUniform2dSafe(const char* l,double x,double y){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform2d(o,x,y); }
        inline void sendUniform2iSafe(const char* l,int x,int y){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform2i(o,x,y); }
        inline void sendUniform2fSafe(const char* l,float x,float y){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform2f(o,x,y); }
        inline void sendUniform2Force(const char* l,double x,double y){ glUniform2d(getUniformLoc(l),x,y); }
        inline void sendUniform2Force(const char* l,int x,int y){ glUniform2i(getUniformLoc(l),x,y); }
        inline void sendUniform2Force(const char* l,float x,float y){ glUniform2f(getUniformLoc(l),x,y); }
        inline void sendUniform2dForce(const char* l,double x,double y){ glUniform2d(getUniformLoc(l),x,y); }
        inline void sendUniform2iForce(const char* l,int x,int y){ glUniform2i(getUniformLoc(l),x,y); }
        inline void sendUniform2fForce(const char* l,float x,float y){ glUniform2f(getUniformLoc(l),x,y); }
        //vector and array of values
        inline void sendUniform2v(const char* l,std::vector<glm::dvec2>& d,uint i){ glUniform2dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2v(const char* l,std::vector<glm::ivec2>& d,uint i){ glUniform2iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2v(const char* l,std::vector<glm::vec2>& d,uint i){ glUniform2fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2v(const char* l,glm::dvec2* d,uint i){ glUniform2dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2v(const char* l,glm::ivec2* d,uint i){ glUniform2iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2v(const char* l,glm::vec2* d,uint i){ glUniform2fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2fv(const char* l,std::vector<glm::vec2>& d,uint i){ glUniform2fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2fv(const char* l,glm::vec2* d,uint i){ glUniform2fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2fvSafe(const char* l,std::vector<glm::vec2>& d,uint i){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform2fv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform2fvSafe(const char* l,glm::vec2* d,uint i){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform2fv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform2fvForce(const char* l,std::vector<glm::vec2>& d,uint i){ glUniform2fv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2fvForce(const char* l,glm::vec2* d,uint i){ glUniform2fv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2dv(const char* l,std::vector<glm::dvec2>& d,uint i){ glUniform2dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2dv(const char* l,glm::dvec2* d,uint i){ glUniform2dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2dvSafe(const char* l,std::vector<glm::dvec2>& d,uint i){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform2dv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform2dvSafe(const char* l,glm::dvec2* d,uint i){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform2dv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform2dvForce(const char* l,std::vector<glm::dvec2>& d,uint i){ glUniform2dv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2dvForce(const char* l,glm::dvec2* d,uint i){ glUniform2dv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2iv(const char* l,std::vector<glm::ivec2>& d,uint i){ glUniform2iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2iv(const char* l,glm::ivec2* d,uint i){ glUniform2iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2ivSafe(const char* l,std::vector<glm::ivec2>& d,uint i){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform2iv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform2ivSafe(const char* l,glm::ivec2* d,uint i){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform2iv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform2ivForce(const char* l,std::vector<glm::ivec2>& d,uint i){ glUniform2iv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform2ivForce(const char* l,glm::ivec2* d,uint i){ glUniform2iv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        
        //Uniform 3
        //vectors
        inline void sendUniform3(const char* l,glm::dvec3& v){ glUniform3d(getUniformLocUnsafe(l),v.x,v.y,v.z); }
        inline void sendUniform3(const char* l,glm::ivec3& v){ glUniform3i(getUniformLocUnsafe(l),v.x,v.y,v.z); }
        inline void sendUniform3(const char* l,glm::vec3& v){ glUniform3f(getUniformLocUnsafe(l),v.x,v.y,v.z); }
        inline void sendUniform3d(const char* l,glm::dvec3& v){ glUniform3d(getUniformLocUnsafe(l),v.x,v.y,v.z); }
        inline void sendUniform3i(const char* l,glm::ivec3& v){ glUniform3i(getUniformLocUnsafe(l),v.x,v.y,v.z); }
        inline void sendUniform3f(const char* l,glm::vec3& v){ glUniform3f(getUniformLocUnsafe(l),v.x,v.y,v.z); }
        inline void sendUniform3Safe(const char* l,glm::dvec3& v){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform3d(o,v.x,v.y,v.z); }
        inline void sendUniform3Safe(const char* l,glm::ivec3& v){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform3i(o,v.x,v.y,v.z); }
        inline void sendUniform3Safe(const char* l,glm::vec3& v){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform3f(o,v.x,v.y,v.z); } 
        inline void sendUniform3dSafe(const char* l,glm::dvec3& v){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform3d(o,v.x,v.y,v.z); }
        inline void sendUniform3iSafe(const char* l,glm::ivec3& v){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform3i(o,v.x,v.y,v.z); }
        inline void sendUniform3fSafe(const char* l,glm::vec3& v){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform3f(o,v.x,v.y,v.z); }
        inline void sendUniform3Force(const char* l,glm::dvec3& v){ glUniform3d(getUniformLoc(l),v.x,v.y,v.z); }
        inline void sendUniform3Force(const char* l,glm::ivec3& v){ glUniform3i(getUniformLoc(l),v.x,v.y,v.z); }
        inline void sendUniform3Force(const char* l,glm::vec3& v){ glUniform3f(getUniformLoc(l),v.x,v.y,v.z); }
        inline void sendUniform3dForce(const char* l,glm::dvec3& v){ glUniform3d(getUniformLoc(l),v.x,v.y,v.z); }
        inline void sendUniform3iForce(const char* l,glm::ivec3& v){ glUniform3i(getUniformLoc(l),v.x,v.y,v.z); }
        inline void sendUniform3fForce(const char* l,glm::vec3& v){ glUniform3f(getUniformLoc(l),v.x,v.y,v.z); }
        //seperate
        inline void sendUniform3(const char* l,double x,double y,double z){ glUniform3d(getUniformLocUnsafe(l),x,y,z); }
        inline void sendUniform3(const char* l,int x,int y,int z){ glUniform3i(getUniformLocUnsafe(l),x,y,z); }
        inline void sendUniform3(const char* l,float x,float y,float z){ glUniform3f(getUniformLocUnsafe(l),x,y,z); }
        inline void sendUniform3d(const char* l,double x,double y,double z){ glUniform3d(getUniformLocUnsafe(l),x,y,z); }
        inline void sendUniform3i(const char* l,int x,int y,int z){ glUniform3i(getUniformLocUnsafe(l),x,y,z); }
        inline void sendUniform3f(const char* l,float x,float y,float z){ glUniform3f(getUniformLocUnsafe(l),x,y,z); }
        inline void sendUniform3Safe(const char* l,double x,double y,double z){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform3d(o,x,y,z); }
        inline void sendUniform3Safe(const char* l,int x,int y,int z){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform3i(o,x,y,z); }
        inline void sendUniform3Safe(const char* l,float x,float y,float z){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform3f(o,x,y,z); }
        inline void sendUniform3dSafe(const char* l,double x,double y,double z){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform3d(o,x,y,z); }
        inline void sendUniform3iSafe(const char* l,int x,int y,int z){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform3i(o,x,y,z); }
        inline void sendUniform3fSafe(const char* l,float x,float y,float z){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform3f(o,x,y,z); }
        inline void sendUniform3Force(const char* l,double x,double y,double z){ glUniform3d(getUniformLoc(l),x,y,z); }
        inline void sendUniform3Force(const char* l,int x,int y,int z){ glUniform3i(getUniformLoc(l),x,y,z); }
        inline void sendUniform3Force(const char* l,float x,float y,float z){ glUniform3f(getUniformLoc(l),x,y,z); }
        inline void sendUniform3dForce(const char* l,double x,double y,double z){ glUniform3d(getUniformLoc(l),x,y,z); }
        inline void sendUniform3iForce(const char* l,int x,int y,int z){ glUniform3i(getUniformLoc(l),x,y,z); }
        inline void sendUniform3fForce(const char* l,float x,float y,float z){ glUniform3f(getUniformLoc(l),x,y,z); }
        //vector and array of values
        inline void sendUniform3v(const char* l,std::vector<glm::dvec3>& d,uint i){ glUniform3dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3v(const char* l,std::vector<glm::ivec3>& d,uint i){ glUniform3iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3v(const char* l,std::vector<glm::vec3>& d,uint i){ glUniform3fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3v(const char* l,glm::dvec3* d,uint i){ glUniform3dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3v(const char* l,glm::ivec3* d,uint i){ glUniform3iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3v(const char* l,glm::vec3* d,uint i){ glUniform3fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); } 
        inline void sendUniform3fv(const char* l,std::vector<glm::vec3>& d,uint i){ glUniform3fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3fv(const char* l,glm::vec3* d,uint i){ glUniform3fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3fvSafe(const char* l,std::vector<glm::vec3>& d,uint i){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform3fv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform3fvSafe(const char* l,glm::vec3* d,uint i){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform3fv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform3fvForce(const char* l,std::vector<glm::vec3>& d,uint i){ glUniform3fv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3fvForce(const char* l,glm::vec3* d,uint i){ glUniform3fv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3dv(const char* l,std::vector<glm::dvec3>& d,uint i){ glUniform3dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3dv(const char* l,glm::dvec3* d,uint i){ glUniform3dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3dvSafe(const char* l,std::vector<glm::dvec3>& d,uint i){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform3dv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform3dvSafe(const char* l,glm::dvec3* d,uint i){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform3dv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform3dvForce(const char* l,std::vector<glm::dvec3>& d,uint i){ glUniform3dv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3dvForce(const char* l,glm::dvec3* d,uint i){ glUniform3dv(getUniformLoc(l),i,glm::value_ptr(d[0])); } 
        inline void sendUniform3iv(const char* l,std::vector<glm::ivec3>& d,uint i){ glUniform3iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3iv(const char* l,glm::ivec3* d,uint i){ glUniform3iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3ivSafe(const char* l,std::vector<glm::ivec3>& d,uint i){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform3iv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform3ivSafe(const char* l,glm::ivec3* d,uint i){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform3iv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform3ivForce(const char* l,std::vector<glm::ivec3>& d,uint i){ glUniform3iv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform3ivForce(const char* l,glm::ivec3* d,uint i){ glUniform3iv(getUniformLoc(l),i,glm::value_ptr(d[0])); }  
        
        //Uniform 4
        //vectors
        inline void sendUniform4(const char* l,glm::dvec4& v){ glUniform4d(getUniformLocUnsafe(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4(const char* l,glm::ivec4& v){ glUniform4i(getUniformLocUnsafe(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4(const char* l,glm::vec4& v){ glUniform4f(getUniformLocUnsafe(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4d(const char* l,glm::dvec4& v){ glUniform4d(getUniformLocUnsafe(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4i(const char* l,glm::ivec4& v){ glUniform4i(getUniformLocUnsafe(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4f(const char* l,glm::vec4& v){ glUniform4f(getUniformLocUnsafe(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4Safe(const char* l,glm::dvec4& v){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform4d(o,v.x,v.y,v.z,v.w); }
        inline void sendUniform4Safe(const char* l,glm::ivec4& v){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform4i(o,v.x,v.y,v.z,v.w); }
        inline void sendUniform4Safe(const char* l,glm::vec4& v){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform4f(o,v.x,v.y,v.z,v.w); }
        inline void sendUniform4dSafe(const char* l,glm::dvec4& v){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform4d(o,v.x,v.y,v.z,v.w); }
        inline void sendUniform4iSafe(const char* l,glm::ivec4& v){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform4i(o,v.x,v.y,v.z,v.w); }
        inline void sendUniform4fSafe(const char* l,glm::vec4& v){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform4f(o,v.x,v.y,v.z,v.w); }
        inline void sendUniform4Force(const char* l,glm::dvec4& v){ glUniform4d(getUniformLoc(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4Force(const char* l,glm::ivec4& v){ glUniform4i(getUniformLoc(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4Force(const char* l,glm::vec4& v){ glUniform4f(getUniformLoc(l),v.x,v.y,v.z,v.w); } 
        inline void sendUniform4dForce(const char* l,glm::dvec4& v){ glUniform4d(getUniformLoc(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4iForce(const char* l,glm::ivec4& v){ glUniform4i(getUniformLoc(l),v.x,v.y,v.z,v.w); }
        inline void sendUniform4fForce(const char* l,glm::vec4& v){ glUniform4f(getUniformLoc(l),v.x,v.y,v.z,v.w); }
        //seperate
        inline void sendUniform4(const char* l,double x,double y,double z,double w){ glUniform4d(getUniformLocUnsafe(l),x,y,z,w); }
        inline void sendUniform4(const char* l,int x,int y,int z,int w){ glUniform4i(getUniformLocUnsafe(l),x,y,z,w); }
        inline void sendUniform4(const char* l,float x,float y,float z,float w){ glUniform4f(getUniformLocUnsafe(l),x,y,z,w); }
        inline void sendUniform4d(const char* l,double x,double y,double z,double w){ glUniform4d(getUniformLocUnsafe(l),x,y,z,w); }
        inline void sendUniform4i(const char* l,int x,int y,int z,int w){ glUniform4i(getUniformLocUnsafe(l),x,y,z,w); }
        inline void sendUniform4f(const char* l,float x,float y,float z,float w){ glUniform4f(getUniformLocUnsafe(l),x,y,z,w); }
        inline void sendUniform4Safe(const char* l,double x,double y,double z,double w){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform4d(o,x,y,z,w); }
        inline void sendUniform4Safe(const char* l,int x,int y,int z,int w){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform4i(o,x,y,z,w); }
        inline void sendUniform4Safe(const char* l,float x,float y,float z,float w){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform4f(o,x,y,z,w); }
        inline void sendUniform4dSafe(const char* l,double x,double y,double z,double w){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform4d(o,x,y,z,w); }
        inline void sendUniform4iSafe(const char* l,int x,int y,int z,int w){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform4i(o,x,y,z,w); }
        inline void sendUniform4fSafe(const char* l,float x,float y,float z,float w){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform4f(o,x,y,z,w); }
        inline void sendUniform4Force(const char* l,double x,double y,double z,double w){ glUniform4d(getUniformLoc(l),x,y,z,w); }
        inline void sendUniform4Force(const char* l,int x,int y,int z,int w){ glUniform4i(getUniformLoc(l),x,y,z,w); }
        inline void sendUniform4Force(const char* l,float x,float y,float z,float w){ glUniform4f(getUniformLoc(l),x,y,z,w); }
        inline void sendUniform4dForce(const char* l,double x,double y,double z,double w){ glUniform4d(getUniformLoc(l),x,y,z,w); }
        inline void sendUniform4iForce(const char* l,int x,int y,int z,int w){ glUniform4i(getUniformLoc(l),x,y,z,w); }
        inline void sendUniform4fForce(const char* l,float x,float y,float z,float w){ glUniform4f(getUniformLoc(l),x,y,z,w); }
        //vector / array of values
        inline void sendUniform4v(const char* l,std::vector<glm::dvec4>& d,uint i){ glUniform4dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4v(const char* l,std::vector<glm::ivec4>& d,uint i){ glUniform4iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4v(const char* l,std::vector<glm::vec4>& d,uint i){ glUniform4fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); } 
        inline void sendUniform4v(const char* l,glm::dvec4* d,uint i){ glUniform4dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4v(const char* l,glm::ivec4* d,uint i){ glUniform4iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4v(const char* l,glm::vec4* d,uint i){ glUniform4fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4fv(const char* l,std::vector<glm::vec4>& d,uint i){ glUniform4fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4fv(const char* l,glm::vec4* d,uint i){ glUniform4fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4fvSafe(const char* l,std::vector<glm::vec4>& d,uint i){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform4fv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform4fvSafe(const char* l,glm::vec4* d,uint i){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform4fv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform4fvForce(const char* l,std::vector<glm::vec4>& d,uint i){ glUniform4fv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4fvForce(const char* l,glm::vec4* d,uint i){ glUniform4fv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4dv(const char* l,std::vector<glm::dvec4>& d,uint i){ glUniform4dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4dv(const char* l,glm::dvec4* d,uint i){ glUniform4dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4dvSafe(const char* l,std::vector<glm::dvec4>& d,uint i){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform4dv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform4dvSafe(const char* l,glm::dvec4* d,uint i){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform4dv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform4dvForce(const char* l,std::vector<glm::dvec4>& d,uint i){ glUniform4dv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4dvForce(const char* l,glm::dvec4* d,uint i){ glUniform4dv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4iv(const char* l,std::vector<glm::ivec4>& d,uint i){ glUniform4iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4iv(const char* l,glm::ivec4* d,uint i){ glUniform4iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4ivSafe(const char* l,std::vector<glm::ivec4>& d,uint i){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform4iv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform4ivSafe(const char* l,glm::ivec4* d,uint i){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniform4iv(o,i,glm::value_ptr(d[0])); }
        inline void sendUniform4ivForce(const char* l,std::vector<glm::ivec4>& d,uint i){ glUniform4iv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        inline void sendUniform4ivForce(const char* l,glm::ivec4* d,uint i){ glUniform4iv(getUniformLoc(l),i,glm::value_ptr(d[0])); }

        //Matrix 2x2
        inline void sendUniformMatrix2(const char* l,glm::mat2& m){ glUniformMatrix2fv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix2v(const char* l,std::vector<glm::mat2>& m,uint count){ std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 4; ++i){d.push_back(m[i]);}}glUniformMatrix2fv(getUniformLocUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix2(const char* l,glm::dmat2& m){ glUniformMatrix2dv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix2v(const char* l,std::vector<glm::dmat2>& m,uint count){ std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 4; ++i){d.push_back(m[i]);}}glUniformMatrix2dv(getUniformLocUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix2f(const char* l,glm::mat2& m){ glUniformMatrix2fv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix2fv(const char* l,std::vector<glm::mat2>& m,uint count){ std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 4; ++i){d.push_back(m[i]);}}glUniformMatrix2fv(getUniformLocUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix2d(const char* l,glm::dmat2& m){ glUniformMatrix2dv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix2dv(const char* l,std::vector<glm::dmat2>& m,uint count){ std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 4; ++i){d.push_back(m[i]);}}glUniformMatrix2dv(getUniformLocUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix2fSafe(const char* l,glm::mat2& m){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniformMatrix2fv(o,1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix2fvSafe(const char* l,std::vector<glm::mat2>& m, uint count){ const GLint& o=getUniformLoc(l);if(o==-1)return;std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 4; ++i){d.push_back(m[i]);}}glUniformMatrix2fv(o,count,0,&d[0]); }  
        inline void sendUniformMatrix2dSafe(const char* l,glm::dmat2& m){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniformMatrix2dv(o,1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix2dvSafe(const char* l,std::vector<glm::dmat2>& m, uint count){ const GLint& o=getUniformLoc(l);if(o==-1)return;std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 4; ++i){d.push_back(m[i]);}}glUniformMatrix2dv(o,count,0,&d[0]); }   
        inline void sendUniformMatrix2fForce(const char* l,glm::mat2& m){ glUniformMatrix2fv(getUniformLoc(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix2fvForce(const char* l,std::vector<glm::mat2>& m,uint count){ std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 4; ++i){d.push_back(m[i]);}}glUniformMatrix2fv(getUniformLoc(l),count,0,&d[0]); }  
        inline void sendUniformMatrix2dForce(const char* l,glm::dmat2& m){ glUniformMatrix2dv(getUniformLoc(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix2dvForce(const char* l,std::vector<glm::dmat2>& m,uint count){ std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 4; ++i){d.push_back(m[i]);}}glUniformMatrix2dv(getUniformLoc(l),count,0,&d[0]); }   
        
        //Matrix 3x3
        inline void sendUniformMatrix3(const char* l,glm::mat3& m){ glUniformMatrix3fv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix3v(const char* l,std::vector<glm::mat3>& m,uint count){ std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 9; ++i){d.push_back(m[i]);}}glUniformMatrix3fv(getUniformLocUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix3(const char* l,glm::dmat3& m){ glUniformMatrix3dv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix3v(const char* l,std::vector<glm::dmat3>& m,uint count){ std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 9; ++i){d.push_back(m[i]);}}glUniformMatrix3dv(getUniformLocUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix3f(const char* l,glm::mat3& m){ glUniformMatrix3fv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix3fv(const char* l,std::vector<glm::mat3>& m,uint count){ std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 9; ++i){d.push_back(m[i]);}}glUniformMatrix3fv(getUniformLocUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix3d(const char* l,glm::dmat3& m){ glUniformMatrix3dv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix3dv(const char* l,std::vector<glm::dmat3>& m,uint count){ std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 9; ++i){d.push_back(m[i]);}}glUniformMatrix3dv(getUniformLocUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix3fSafe(const char* l,glm::mat3& m){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniformMatrix3fv(o,1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix3fvSafe(const char* l,std::vector<glm::mat3>& m, uint count){ const GLint& o=getUniformLoc(l);if(o==-1)return;std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 9; ++i){d.push_back(m[i]);}}glUniformMatrix3fv(o,count,0,&d[0]); }  
        inline void sendUniformMatrix3dSafe(const char* l,glm::dmat3& m){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniformMatrix3dv(o,1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix3dvSafe(const char* l,std::vector<glm::dmat3>& m, uint count){ const GLint& o=getUniformLoc(l);if(o==-1)return;std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 9; ++i){d.push_back(m[i]);}}glUniformMatrix3dv(o,count,0,&d[0]); }  
        inline void sendUniformMatrix3fForce(const char* l,glm::mat3& m){ glUniformMatrix3fv(getUniformLoc(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix3fvForce(const char* l,std::vector<glm::mat3>& m,uint count){ std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 9; ++i){d.push_back(m[i]);}}glUniformMatrix3fv(getUniformLoc(l),count,0,&d[0]); }  
        inline void sendUniformMatrix3dForce(const char* l,glm::dmat3& m){ glUniformMatrix3dv(getUniformLoc(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix3dvForce(const char* l,std::vector<glm::dmat3>& m,uint count){ std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 9; ++i){d.push_back(m[i]);}}glUniformMatrix3dv(getUniformLoc(l),count,0,&d[0]); }   
        
        //Matrix4x4
        inline void sendUniformMatrix4(const char* l,glm::mat4& m){ glUniformMatrix4fv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix4v(const char* l,std::vector<glm::mat4>& m,uint count){ std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 16; ++i){d.push_back(m[i]);}}glUniformMatrix4fv(getUniformLocUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix4(const char* l,glm::dmat4& m){ glUniformMatrix4dv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix4v(const char* l,std::vector<glm::dmat4>& m,uint count){ std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 16; ++i){d.push_back(m[i]);}}glUniformMatrix4dv(getUniformLocUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix4f(const char* l,glm::mat4& m){ glUniformMatrix4fv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix4fv(const char* l,std::vector<glm::mat4>& m,uint count){ std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 16; ++i){d.push_back(m[i]);}}glUniformMatrix4fv(getUniformLocUnsafe(l),count,0,&d[0]); }
        inline void sendUniformMatrix4d(const char* l,glm::dmat4& m){ glUniformMatrix4dv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix4dv(const char* l,std::vector<glm::dmat4>& m,uint count){ std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 16; ++i){d.push_back(m[i]);}}glUniformMatrix4dv(getUniformLocUnsafe(l),count,0,&d[0]); } 
        inline void sendUniformMatrix4fSafe(const char* l,glm::mat4& m){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniformMatrix4fv(o,1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix4fvSafe(const char* l,std::vector<glm::mat4>& m, uint count){ const GLint& o=getUniformLoc(l);if(o==-1)return;std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 16; ++i){d.push_back(m[i]);}}glUniformMatrix4fv(o,count,0,&d[0]); }  
        inline void sendUniformMatrix4dSafe(const char* l,glm::dmat4& m){ const GLint& o=getUniformLoc(l);if(o==-1)return;glUniformMatrix4dv(o,1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix4dvSafe(const char* l,std::vector<glm::dmat4>& m, uint count){ const GLint& o=getUniformLoc(l);if(o==-1)return;std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 16; ++i){d.push_back(m[i]);}}glUniformMatrix4dv(o,count,0,&d[0]); }  
        inline void sendUniformMatrix4fForce(const char* l,glm::mat4& m){ glUniformMatrix4fv(getUniformLoc(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix4fvForce(const char* l,std::vector<glm::mat4>& m,uint count){ std::vector<float> d;for(auto ma:m){float* m = glm::value_ptr(ma);for(uint i = 0; i < 16; ++i){d.push_back(m[i]);}}glUniformMatrix4fv(getUniformLoc(l),count,0,&d[0]); }  
        inline void sendUniformMatrix4dForce(const char* l,glm::dmat4& m){ glUniformMatrix4dv(getUniformLoc(l),1,0,glm::value_ptr(m)); }
        inline void sendUniformMatrix4dvForce(const char* l,std::vector<glm::dmat4>& m,uint count){ std::vector<double> d;for(auto ma:m){double* m = glm::value_ptr(ma);for(uint i = 0; i < 16; ++i){d.push_back(m[i]);}}glUniformMatrix4dv(getUniformLoc(l),count,0,&d[0]); }   

        void renderTexture(Texture*,glm::vec2& pos, glm::vec4& col,float angle, glm::vec2& scl, float depth);
        void renderText(std::string& text,Font*, glm::vec2& pos,glm::vec4& color, float angle, glm::vec2& scl, float depth);
        void renderRectangle(glm::vec2& pos, glm::vec4& col, float w, float h, float angle, float depth);
    };
};
#endif
