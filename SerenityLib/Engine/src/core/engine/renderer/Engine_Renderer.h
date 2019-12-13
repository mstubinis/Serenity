#pragma once
#ifndef ENGINE_ENGINE_RENDERER_H
#define ENGINE_ENGINE_RENDERER_H

#include <core/engine/renderer/RendererState.h>
#include <core/engine/utils/Utils.h>

#include <core/engine/renderer/RendererIncludes.h>

#include <core/engine/renderer/opengl/State.h>
#include <core/engine/renderer/opengl/Extensions.h>

#include <core/engine/fonts/FontIncludes.h>

#include <glm/gtc/type_ptr.hpp>
#include <SFML/Window.hpp>

class  Viewport;
class  Texture;
class  Font;
class  Decal;
struct Entity;
struct BufferObject;

namespace Engine{
namespace epriv{
    class  GBuffer;
    class  FramebufferObject;
    class  RenderbufferObject;
    class  RenderManager final{
        public:
            RendererState            RendererState;
            OpenGLState              OpenGLStateMachine;
            OpenGLExtensions         OpenGLExtensionsManager;

            class impl;
            std::unique_ptr<impl> m_i;

            RenderManager(const char* name,uint w,uint h);
            ~RenderManager();

            static uint GLSL_VERSION;
            static uint OPENGL_VERSION;

            void cleanup();

            void _init(const char* name,uint w,uint h);
            void _resize(uint width, uint height);

            void _render(const double& dt, Viewport&, const bool mainRenderFunc = true, const GLuint display_fbo = 0, const GLuint display_rbo = 0);
            void _onFullscreen(sf::Window* sfWindow, const sf::VideoMode& videoMode,const char* winName, const uint& style, const sf::ContextSettings& contextSettings);
            void _onOpenGLContextCreation(uint width,uint height,uint glslVersion,uint openglVersion);
            const bool _bindShaderProgram(ShaderProgram*);
            const bool _unbindShaderProgram();
            void _clear2DAPICommands();
            void _sort2DAPICommands();
            const bool _bindMaterial(Material*);
            const bool _unbindMaterial();
            const float _getGIPackedData();
            GBuffer& getGbuffer();
            void _genPBREnvMapData(Texture&,uint,uint);
    };
};
namespace Renderer{
    namespace Settings{
        void setGamma(const float g);
        const float getGamma();

        void clear(const bool color = true, const bool depth = true, const bool stencil = true);
        

        const bool setAntiAliasingAlgorithm(const AntiAliasingAlgorithm::Algorithm&);

        void enableDrawPhysicsInfo(const bool b = true);
        void disableDrawPhysicsInfo();

        namespace Lighting{
            void enable(bool b = true);
            void disable();
            const float getGIContributionGlobal();
            void setGIContributionGlobal(const float giGlobal);
            const float getGIContributionDiffuse();
            void setGIContributionDiffuse(const float giDiffuse);
            const float getGIContributionSpecular();
            void setGIContributionSpecular(const float giSpecular);
            void setGIContribution(const float global, const float diffuse, const float specular);
        };
    };
    void restoreDefaultOpenGLState();
    void restoreCurrentOpenGLState();

    void renderFullscreenQuad(const uint& width, const uint& height, const uint& startX = 0, const uint& startY = 0);
    void renderFullscreenTriangle(const uint& width, const uint& height, const uint& startX = 0, const uint& startY = 0);

    void renderFullscreenQuad();
    void renderFullscreenTriangle();

    inline const GLint getUniformLoc(const char* location);
    inline const GLint& getUniformLocUnsafe(const char* location);

    const bool cullFace(const GLenum& state);
    const bool setDepthFunc(const GLenum& func);
    const bool setViewport(const float& x, const float& y, const float& width, const float& height);

    const bool stencilFunc(const GLenum& func, const GLint& ref, const GLuint& mask);
    const bool stencilMask(const GLuint& mask);
    const bool stencilOp(const GLenum& sfail, const GLenum& dpfail, const GLenum& dppass);

    void bindFBO(const GLuint& fbo);
    void bindFBO(epriv::FramebufferObject& rbo);
    const bool bindRBO(const GLuint& rbo);
    const bool bindRBO(epriv::RenderbufferObject& rbo);
    const bool bindReadFBO(const GLuint& fbo);
    const bool bindDrawFBO(const GLuint& fbo);

    void unbindFBO();
    void unbindRBO();
    void unbindReadFBO();
    void unbindDrawFBO();

    const bool GLEnable(const GLenum& capability);
    const bool GLDisable(const GLenum& capability);
    const bool GLEnablei(const GLenum& capability, const GLuint& index);
    const bool GLDisablei(const GLenum& capability, const GLuint& index);

    const bool bindTextureForModification(const GLuint _textureType, const GLuint _textureObject);

    const bool bindVAO(const GLuint _vaoObject);
    void genAndBindTexture(const GLuint _textureType,GLuint& _textureObject);
    void genAndBindVAO(GLuint& _vaoObject);
    const bool deleteVAO(GLuint& _vaoObject);
    const bool colorMask(const bool& r, const bool& g, const bool& b, const bool& a);
    const bool clearColor(const float& r, const float& g, const float& b, const float& a);

    void sendTexture(const char* location,const Texture& texture,const int& slot);
    void sendTexture(const char* location,const GLuint textureAddress,const int& slot,const GLuint& glTextureType);
    void sendTextureSafe(const char* location, const Texture& texture,const int& slot);
    void sendTextureSafe(const char* location,const GLuint textureAddress,const int& slot,const GLuint& glTextureType);
    
    void alignmentOffset(const Alignment::Type& align, float& x, float& y, const float& width, const float& height);

    void renderTexture(
        const Texture&,
        const glm::vec2& position,
        const glm::vec4& color,
        const float angle,
        const glm::vec2& scale,
        const float depth,
        const Alignment::Type& = Alignment::Type::Center,
        const glm::vec4& scissor = glm::vec4(-1.0f)
    );
    void renderText(
        const std::string& text,
        const Font&,
        const glm::vec2& position,
        const glm::vec4& color,
        const float angle,
        const glm::vec2& scale,
        const float depth,
        const TextAlignment::Type& = TextAlignment::Left,
        const glm::vec4& scissor = glm::vec4(-1.0f)
    );
    void renderRectangle(
        const glm::vec2& pos,
        const glm::vec4& col,
        const float width,
        const float height,
        const float angle,
        const float depth,
        const Alignment::Type& = Alignment::Type::Center,
        const glm::vec4& scissor = glm::vec4(-1.0f)
    );
    void renderBorder(
        const float borderSize,
        const glm::vec2& position,
        const glm::vec4& color,
        const float width,
        const float height,
        const float angle,
        const float depth,
        const Alignment::Type& = Alignment::Type::Center,
        const glm::vec4& scissor = glm::vec4(-1.0f)
    );
    void renderTriangle(
        const glm::vec2& position,
        const glm::vec4& color,
        const float angle,
        const float width,
        const float height,
        const float depth,
        const Alignment::Type& = Alignment::Type::Center,
        const glm::vec4& scissor = glm::vec4(-1.0f)
    );

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
