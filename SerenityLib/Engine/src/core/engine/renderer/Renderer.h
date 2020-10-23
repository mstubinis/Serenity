#pragma once
#ifndef ENGINE_ENGINE_RENDERER_H
#define ENGINE_ENGINE_RENDERER_H

class  Viewport;
class  Texture;
class  Font;
class  Decal;
struct BufferObject;
struct EngineOptions;
class  ModelInstance;
namespace Engine::priv {
    class  GBuffer;
    class  FramebufferObject;
    class  RenderbufferObject;
    class  IRenderingPipeline;
    class  RenderModule;
};

#include <core/engine/renderer/opengl/OpenGL.h>
#include <core/engine/renderer/RendererState.h>
#include <core/engine/utils/Utils.h>
#include <core/engine/renderer/RendererIncludes.h>
#include <core/engine/renderer/opengl/State.h>
#include <core/engine/renderer/opengl/Extensions.h>
#include <core/engine/fonts/FontIncludes.h>
#include <core/engine/renderer/pipelines/IRenderingPipeline.h>
#include <core/engine/textures/TextureIncludes.h>
#include <glm/gtc/type_ptr.hpp>

namespace Engine{
namespace priv{
    class  RenderModule final{
        private:
        public:
            static Engine::view_ptr<Engine::priv::RenderModule> RENDERER;
        public:
            bool                                 m_Lighting         = true;
            bool                                 m_DrawPhysicsDebug = false;
            AntiAliasingAlgorithm                m_AA_algorithm     = AntiAliasingAlgorithm::FXAA;
            float                                m_Gamma            = 2.2f;
            float                                m_GI_Diffuse       = 1.0f;
            float                                m_GI_Specular      = 1.0f;
            float                                m_GI_Global        = 1.0f;
            float                                m_GI_Pack;
            DepthFunc                            m_Depth_Function   = DepthFunc::Less;
            std::unique_ptr<IRenderingPipeline>  m_Pipeline;
        public:
            RenderModule(const EngineOptions& options);
            ~RenderModule() = default;

            static unsigned int GLSL_VERSION;
            static unsigned int OPENGL_VERSION;

            void _init();
            void _resize(unsigned int width, unsigned int height);

            void _render(Viewport&, bool mainRenderFunc = true);

            void _onFullscreen(unsigned int width, unsigned int height);
            void _onOpenGLContextCreation(unsigned int width, unsigned int height, unsigned int glslVersion, unsigned int openglVersion);
            void _clear2DAPICommands();
            void _sort2DAPICommands();

            bool bind(ModelInstance* modelInstance) const;
            bool unbind(ModelInstance* modelInstance) const;

            bool bind(ShaderProgram* shaderProgram) const;
            bool unbind(ShaderProgram* shaderProgram) const;

            bool bind(Mesh* mesh) const;
            bool unbind(Mesh* mesh) const;

            bool bind(Material* material) const;
            bool unbind(Material* material) const;

            float _getGIPackedData();
            void _genPBREnvMapData(Texture&, Handle convolutionTexture, Handle preEnvTexture, uint, uint);
    };
};
namespace Renderer{
    namespace Settings{
        void setGamma(const float g);
        const float getGamma();

        void clear(bool color = true, bool depth = true, bool stencil = true);
        
        void applyGlobalAnisotropicFiltering(float filtering);

        bool setAntiAliasingAlgorithm(AntiAliasingAlgorithm AA_algorithm);

        void enableDrawPhysicsInfo(bool b = true);
        void disableDrawPhysicsInfo();

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
    void restoreDefaultOpenGLState();
    void restoreCurrentOpenGLState();

    void renderFullscreenQuad();
    void renderFullscreenTriangle();

    unsigned int getUniformLoc(const char* location);
    unsigned int getUniformLocUnsafe(const char* location);
    unsigned int getCurrentlyBoundTextureOfType(unsigned int textureType) noexcept;

    bool cullFace(GLenum state);
    bool setDepthFunc(GLenum func);
    bool setViewport(float x, float y, float width, float height);

    bool stencilFunc(GLenum func, GLint ref, GLuint mask);
    bool stencilMask(GLuint mask);
    bool stencilOp(GLenum sfail, GLenum dpfail, GLenum dppass);

    void bindFBO(GLuint fbo);
    void bindFBO(const priv::FramebufferObject& rbo);
    bool bindRBO(GLuint rbo);
    bool bindRBO(priv::RenderbufferObject& rbo);
    bool bindReadFBO(GLuint fbo);
    bool bindDrawFBO(GLuint fbo);

    void unbindFBO();
    void unbindRBO();
    void unbindReadFBO();
    void unbindDrawFBO();

    bool GLEnable(GLenum capability);
    bool GLDisable(GLenum capability);
    bool GLEnablei(GLenum capability, GLuint index);
    bool GLDisablei(GLenum capability, GLuint index);

    bool bindTextureForModification(TextureType textureType, GLuint textureObject);

    bool bindVAO(GLuint vaoObject);
    void genAndBindTexture(TextureType textureType, GLuint& textureObject);
    void genAndBindVAO(GLuint& vaoObject);
    bool deleteVAO(GLuint& vaoObject);
    bool colorMask(bool r, bool g, bool b, bool a);
    bool clearColor(float r, float g, float b, float a);

    void sendTexture(const char* location, Texture& texture, int slot);
    void sendTexture(const char* location, GLuint textureAddress, int slot, GLuint glTextureType);
    void sendTextureSafe(const char* location, Texture& texture, int slot);
    void sendTextureSafe(const char* location, GLuint textureAddress, int slot, GLuint glTextureType);
    
    void alignmentOffset(Alignment align, float& x, float& y, float width, float height);

    void renderTexture(
        Handle texture,
        const glm::vec2& position,
        const glm::vec4& color,
        float angle,
        const glm::vec2& scale,
        float depth,
        Alignment = Alignment::Center,
        const glm::vec4& scissor = NO_SCISSOR
    );
    void renderText(
        const std::string& text,
        Handle font,
        const glm::vec2& position,
        const glm::vec4& color,
        float angle,
        const glm::vec2& scale,
        float depth,
        TextAlignment = TextAlignment::Left,
        const glm::vec4& scissor = NO_SCISSOR
    );
    void renderRectangle(
        const glm::vec2& pos,
        const glm::vec4& col,
        float width,
        float height,
        float angle,
        float depth,
        Alignment = Alignment::Center,
        const glm::vec4& scissor = NO_SCISSOR
    );
    void renderBorder(
        float borderSize,
        const glm::vec2& position,
        const glm::vec4& color,
        float width,
        float height,
        float angle,
        float depth,
        Alignment = Alignment::Center,
        const glm::vec4& scissor = NO_SCISSOR
    );
    void renderTriangle(
        const glm::vec2& position,
        const glm::vec4& color,
        float angle,
        float width,
        float height,
        float depth,
        Alignment = Alignment::Center,
        const glm::vec4& scissor = NO_SCISSOR
    );



    void renderBackgroundTexture(
        Handle texture,
        const glm::vec2& position,
        const glm::vec4& color,
        float angle,
        const glm::vec2& scale,
        float depth,
        Alignment = Alignment::Center,
        const glm::vec4& scissor = NO_SCISSOR
    );
    void renderBackgroundText(
        const std::string& text,
        Handle font,
        const glm::vec2& position,
        const glm::vec4& color,
        float angle,
        const glm::vec2& scale,
        float depth,
        TextAlignment = TextAlignment::Left,
        const glm::vec4& scissor = NO_SCISSOR
    );
    void renderBackgroundRectangle(
        const glm::vec2& pos,
        const glm::vec4& col,
        float width,
        float height,
        float angle,
        float depth,
        Alignment = Alignment::Center,
        const glm::vec4& scissor = NO_SCISSOR
    );
    void renderBackgroundBorder(
        float borderSize,
        const glm::vec2& position,
        const glm::vec4& color,
        float width,
        float height,
        float angle,
        float depth,
        Alignment = Alignment::Center,
        const glm::vec4& scissor = NO_SCISSOR
    );
    void renderBackgroundTriangle(
        const glm::vec2& position,
        const glm::vec4& color,
        float angle,
        float width,
        float height,
        float depth,
        Alignment = Alignment::Center,
        const glm::vec4& scissor = NO_SCISSOR
    );










    #pragma region UniformSending
    //Uniform 1
    inline void sendUniform1(const char* l, double x){ glUniform1d(getUniformLocUnsafe(l),x); }
    inline void sendUniform1(const char* l, int x){ glUniform1i(getUniformLocUnsafe(l),x); }
    inline void sendUniform1(const char* l, float x){ glUniform1f(getUniformLocUnsafe(l),x); }
    inline void sendUniform1(const char* l, uint32_t x) { glUniform1ui(getUniformLocUnsafe(l), x); }
    inline void sendUniform1Safe(const char* l, double x){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform1d(o,x); }
    inline void sendUniform1Safe(const char* l, int x){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform1i(o,x); }
    inline void sendUniform1Safe(const char* l, float x){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform1f(o,x); }
    inline void sendUniform1Safe(const char* l, uint32_t x) { const auto& o=getUniformLoc(l);if(o==-1)return;glUniform1ui(o, x); }
    inline void sendUniform1Force(const char* l, double x){ glUniform1d(getUniformLoc(l),x); }
    inline void sendUniform1Force(const char* l, int x){ glUniform1i(getUniformLoc(l),x); }
    inline void sendUniform1Force(const char* l, float x){ glUniform1f(getUniformLoc(l),x); }
    inline void sendUniform1Force(const char* l, uint32_t x) { glUniform1ui(getUniformLoc(l),x); }
        
    //Uniform 2
    //vectors
    inline void sendUniform2(const char* l, const glm::dvec2& v){ glUniform2d(getUniformLocUnsafe(l),v.x,v.y); }
    inline void sendUniform2(const char* l, const glm::ivec2& v){ glUniform2i(getUniformLocUnsafe(l),v.x,v.y); }
    inline void sendUniform2(const char* l, const glm::vec2& v){ glUniform2f(getUniformLocUnsafe(l),v.x,v.y); }
    inline void sendUniform2(const char* l, const glm::uvec2& v) { glUniform2ui(getUniformLocUnsafe(l),v.x,v.y); }
    inline void sendUniform2Safe(const char* l, const glm::dvec2& v){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2d(o,v.x,v.y); }
    inline void sendUniform2Safe(const char* l, const glm::ivec2& v){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2i(o,v.x,v.y); }
    inline void sendUniform2Safe(const char* l, const glm::vec2& v){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2f(o,v.x,v.y); }
    inline void sendUniform2Safe(const char* l, const glm::uvec2& v) { const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2ui(o, v.x, v.y); }
    inline void sendUniform2Force(const char* l, const glm::dvec2& v){ glUniform2d(getUniformLoc(l),v.x,v.y); }
    inline void sendUniform2Force(const char* l, const glm::ivec2& v){ glUniform2i(getUniformLoc(l),v.x,v.y); }
    inline void sendUniform2Force(const char* l, const glm::vec2& v){ glUniform2f(getUniformLoc(l),v.x,v.y); }
    inline void sendUniform2Force(const char* l, const glm::uvec2& v) { glUniform2ui(getUniformLoc(l),v.x,v.y); }

    inline void sendUniform2(const char* l, glm::dvec2&& v) { glUniform2d(getUniformLocUnsafe(l), v.x, v.y); }
    inline void sendUniform2(const char* l, glm::ivec2&& v) { glUniform2i(getUniformLocUnsafe(l), v.x, v.y); }
    inline void sendUniform2(const char* l, glm::vec2&& v) { glUniform2f(getUniformLocUnsafe(l), v.x, v.y); }
    inline void sendUniform2(const char* l, glm::uvec2&& v) { glUniform2ui(getUniformLocUnsafe(l), v.x, v.y); }
    inline void sendUniform2Safe(const char* l, glm::dvec2&& v) { const auto& o = getUniformLoc(l); if (o == -1)return; glUniform2d(o, v.x, v.y); }
    inline void sendUniform2Safe(const char* l, glm::ivec2&& v) { const auto& o = getUniformLoc(l); if (o == -1)return; glUniform2i(o, v.x, v.y); }
    inline void sendUniform2Safe(const char* l, glm::vec2&& v) { const auto& o = getUniformLoc(l); if (o == -1)return; glUniform2f(o, v.x, v.y); }
    inline void sendUniform2Safe(const char* l, glm::uvec2&& v) { const auto& o = getUniformLoc(l); if (o == -1)return; glUniform2ui(o, v.x, v.y); }
    inline void sendUniform2Force(const char* l, glm::dvec2&& v) { glUniform2d(getUniformLoc(l), v.x, v.y); }
    inline void sendUniform2Force(const char* l, glm::ivec2&& v) { glUniform2i(getUniformLoc(l), v.x, v.y); }
    inline void sendUniform2Force(const char* l, glm::vec2&& v) { glUniform2f(getUniformLoc(l), v.x, v.y); }
    inline void sendUniform2Force(const char* l, glm::uvec2&& v) { glUniform2ui(getUniformLoc(l), v.x, v.y); }

    //separate
    inline void sendUniform2(const char* l, double x, double y){ glUniform2d(getUniformLocUnsafe(l),x,y); }
    inline void sendUniform2(const char* l, int x, int y){ glUniform2i(getUniformLocUnsafe(l),x,y); }
    inline void sendUniform2(const char* l, float x, float y){ glUniform2f(getUniformLocUnsafe(l),x,y); } 
    inline void sendUniform2(const char* l, uint32_t x, uint32_t y) { glUniform2ui(getUniformLocUnsafe(l), x, y); }
    inline void sendUniform2Safe(const char* l, double x, double y){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2d(o,x,y); }
    inline void sendUniform2Safe(const char* l, int x, int y){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2i(o,x,y); }
    inline void sendUniform2Safe(const char* l, float x, float y){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2f(o,x,y); }
    inline void sendUniform2Safe(const char* l, uint32_t x, uint32_t y) { const auto& o = getUniformLoc(l); if (o == -1)return; glUniform2ui(o, x, y); }
    inline void sendUniform2Force(const char* l, double x, double y){ glUniform2d(getUniformLoc(l),x,y); }
    inline void sendUniform2Force(const char* l, int x, int y){ glUniform2i(getUniformLoc(l),x,y); }
    inline void sendUniform2Force(const char* l, float x, float y){ glUniform2f(getUniformLoc(l),x,y); }
    inline void sendUniform2Force(const char* l, uint32_t x, uint32_t y) { glUniform2ui(getUniformLoc(l), x, y); }
    //vector and array of values
    inline void sendUniform2v(const char* l, const std::vector<glm::dvec2>& d,const unsigned int i){ glUniform2dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform2v(const char* l, const std::vector<glm::ivec2>& d, const unsigned int i){ glUniform2iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform2v(const char* l, const std::vector<glm::vec2>& d, const unsigned int i){ glUniform2fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform2v(const char* l, glm::dvec2* d, const unsigned int i){ glUniform2dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform2v(const char* l, glm::ivec2* d, const unsigned int i){ glUniform2iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform2v(const char* l, glm::vec2* d, const unsigned int i){ glUniform2fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform2vSafe(const char* l, const std::vector<glm::vec2>& d, const unsigned int i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2fv(o,i,glm::value_ptr(d[0])); }
    inline void sendUniform2vSafe(const char* l,glm::vec2* d, const unsigned int i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2fv(o,i,glm::value_ptr(d[0])); }
    inline void sendUniform2vForce(const char* l, const std::vector<glm::vec2>& d, const unsigned int i){ glUniform2fv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform2vForce(const char* l,glm::vec2* d, const unsigned int i){ glUniform2fv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform2vSafe(const char* l, const std::vector<glm::dvec2>& d, const unsigned int i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2dv(o,i,glm::value_ptr(d[0])); }
    inline void sendUniform2vSafe(const char* l,glm::dvec2* d, const unsigned int i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2dv(o,i,glm::value_ptr(d[0])); }
    inline void sendUniform2vForce(const char* l, const std::vector<glm::dvec2>& d, const unsigned int i){ glUniform2dv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform2vForce(const char* l,glm::dvec2* d, const unsigned int i){ glUniform2dv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform2vSafe(const char* l, const std::vector<glm::ivec2>& d, const unsigned int i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2iv(o,i,glm::value_ptr(d[0])); }
    inline void sendUniform2vSafe(const char* l,glm::ivec2* d, const unsigned int i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform2iv(o,i,glm::value_ptr(d[0])); }
    inline void sendUniform2vForce(const char* l, const std::vector<glm::ivec2>& d, const unsigned int i){ glUniform2iv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform2vForce(const char* l,glm::ivec2* d, const unsigned int i){ glUniform2iv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        
    //Uniform 3
    //vectors
    inline void sendUniform3(const char* l, const glm::dvec3& v){ glUniform3d(getUniformLocUnsafe(l),v.x,v.y,v.z); }
    inline void sendUniform3(const char* l, const glm::ivec3& v){ glUniform3i(getUniformLocUnsafe(l),v.x,v.y,v.z); }
    inline void sendUniform3(const char* l, const glm::vec3& v){ glUniform3f(getUniformLocUnsafe(l),v.x,v.y,v.z); }
    inline void sendUniform3(const char* l, const glm::uvec3& v) { glUniform3ui(getUniformLocUnsafe(l), v.x, v.y, v.z); }
    inline void sendUniform3Safe(const char* l, const glm::dvec3& v){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3d(o,v.x,v.y,v.z); }
    inline void sendUniform3Safe(const char* l, const glm::ivec3& v){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3i(o,v.x,v.y,v.z); }
    inline void sendUniform3Safe(const char* l, const glm::vec3& v){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3f(o,v.x,v.y,v.z); }
    inline void sendUniform3Safe(const char* l, const glm::uvec3& v) { const auto& o = getUniformLoc(l); if (o == -1)return; glUniform3ui(o, v.x, v.y, v.z); }
    inline void sendUniform3Force(const char* l, const glm::dvec3& v){ glUniform3d(getUniformLoc(l),v.x,v.y,v.z); }
    inline void sendUniform3Force(const char* l, const glm::ivec3& v){ glUniform3i(getUniformLoc(l),v.x,v.y,v.z); }
    inline void sendUniform3Force(const char* l, const glm::vec3& v){ glUniform3f(getUniformLoc(l),v.x,v.y,v.z); }
    inline void sendUniform3Force(const char* l, const glm::uvec3& v) { glUniform3ui(getUniformLoc(l), v.x, v.y, v.z); }

    inline void sendUniform3(const char* l, glm::dvec3&& v) { glUniform3d(getUniformLocUnsafe(l), v.x, v.y, v.z); }
    inline void sendUniform3(const char* l, glm::ivec3&& v) { glUniform3i(getUniformLocUnsafe(l), v.x, v.y, v.z); }
    inline void sendUniform3(const char* l, glm::vec3&& v) { glUniform3f(getUniformLocUnsafe(l), v.x, v.y, v.z); }
    inline void sendUniform3(const char* l, glm::uvec3&& v) { glUniform3ui(getUniformLocUnsafe(l), v.x, v.y, v.z); }
    inline void sendUniform3Safe(const char* l, glm::dvec3&& v) { const auto& o = getUniformLoc(l); if (o == -1)return; glUniform3d(o, v.x, v.y, v.z); }
    inline void sendUniform3Safe(const char* l, glm::ivec3&& v) { const auto& o = getUniformLoc(l); if (o == -1)return; glUniform3i(o, v.x, v.y, v.z); }
    inline void sendUniform3Safe(const char* l, glm::vec3&& v) { const auto& o = getUniformLoc(l); if (o == -1)return; glUniform3f(o, v.x, v.y, v.z); }
    inline void sendUniform3Safe(const char* l, glm::uvec3&& v) { const auto& o = getUniformLoc(l); if (o == -1)return; glUniform3ui(o, v.x, v.y, v.z); }
    inline void sendUniform3Force(const char* l, glm::dvec3&& v) { glUniform3d(getUniformLoc(l), v.x, v.y, v.z); }
    inline void sendUniform3Force(const char* l, glm::ivec3&& v) { glUniform3i(getUniformLoc(l), v.x, v.y, v.z); }
    inline void sendUniform3Force(const char* l, glm::vec3&& v) { glUniform3f(getUniformLoc(l), v.x, v.y, v.z); }
    inline void sendUniform3Force(const char* l, glm::uvec3&& v) { glUniform3ui(getUniformLoc(l), v.x, v.y, v.z); }

    //separate
    inline void sendUniform3(const char* l,double x,double y,double z){ glUniform3d(getUniformLocUnsafe(l),x,y,z); }
    inline void sendUniform3(const char* l,int x,int y,int z){ glUniform3i(getUniformLocUnsafe(l),x,y,z); }
    inline void sendUniform3(const char* l,float x,float y,float z){ glUniform3f(getUniformLocUnsafe(l),x,y,z); }
    inline void sendUniform3(const char* l, uint32_t x, uint32_t y, uint32_t z) { glUniform3ui(getUniformLocUnsafe(l), x, y, z); }
    inline void sendUniform3Safe(const char* l,double x,double y,double z){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3d(o,x,y,z); }
    inline void sendUniform3Safe(const char* l,int x,int y,int z){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3i(o,x,y,z); }
    inline void sendUniform3Safe(const char* l,float x,float y,float z){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3f(o,x,y,z); }
    inline void sendUniform3Safe(const char* l, uint32_t x, uint32_t y, uint32_t z) { const auto& o = getUniformLoc(l); if (o == -1)return; glUniform3ui(o, x, y, z); }
    inline void sendUniform3Force(const char* l,double x,double y,double z){ glUniform3d(getUniformLoc(l),x,y,z); }
    inline void sendUniform3Force(const char* l,int x,int y,int z){ glUniform3i(getUniformLoc(l),x,y,z); }
    inline void sendUniform3Force(const char* l,float x,float y,float z){ glUniform3f(getUniformLoc(l),x,y,z); }
    inline void sendUniform3Force(const char* l, uint32_t x, uint32_t y, uint32_t z) { glUniform3ui(getUniformLoc(l), x, y, z); }
    //vector and array of values
    inline void sendUniform3v(const char* l, const std::vector<glm::dvec3>& d, const unsigned int i){ glUniform3dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform3v(const char* l, const std::vector<glm::ivec3>& d, const unsigned int i){ glUniform3iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform3v(const char* l, const std::vector<glm::vec3>& d, const unsigned int i){ glUniform3fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform3v(const char* l,glm::dvec3* d, const unsigned int i){ glUniform3dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform3v(const char* l,glm::ivec3* d, const unsigned int i){ glUniform3iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform3v(const char* l,glm::vec3* d, const unsigned int i){ glUniform3fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform3vSafe(const char* l, const std::vector<glm::vec3>& d, const unsigned int i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3fv(o,i,glm::value_ptr(d[0])); }
    inline void sendUniform3vSafe(const char* l,glm::vec3* d, const unsigned int i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3fv(o,i,glm::value_ptr(d[0])); }
    inline void sendUniform3vForce(const char* l, const std::vector<glm::vec3>& d, const unsigned int i){ glUniform3fv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform3vForce(const char* l,glm::vec3* d, const unsigned int i){ glUniform3fv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform3vSafe(const char* l, const std::vector<glm::dvec3>& d, const unsigned int i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3dv(o,i,glm::value_ptr(d[0])); }
    inline void sendUniform3vSafe(const char* l,glm::dvec3* d, const unsigned int i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3dv(o,i,glm::value_ptr(d[0])); }
    inline void sendUniform3vForce(const char* l, const std::vector<glm::dvec3>& d, const unsigned int i){ glUniform3dv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform3vForce(const char* l,glm::dvec3* d, const unsigned int i){ glUniform3dv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform3vSafe(const char* l, const std::vector<glm::ivec3>& d, const unsigned int i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3iv(o,i,glm::value_ptr(d[0])); }
    inline void sendUniform3vSafe(const char* l,glm::ivec3* d, const unsigned int i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform3iv(o,i,glm::value_ptr(d[0])); }
    inline void sendUniform3vForce(const char* l, const std::vector<glm::ivec3>& d, const unsigned int i){ glUniform3iv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform3vForce(const char* l,glm::ivec3* d, const unsigned int i){ glUniform3iv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
        
    //Uniform 4
    //vectors
    inline void sendUniform4(const char* l, const glm::dvec4& v){ glUniform4d(getUniformLocUnsafe(l),v.x,v.y,v.z,v.w); }
    inline void sendUniform4(const char* l, const glm::ivec4& v){ glUniform4i(getUniformLocUnsafe(l),v.x,v.y,v.z,v.w); }
    inline void sendUniform4(const char* l, const glm::vec4& v){ glUniform4f(getUniformLocUnsafe(l),v.x,v.y,v.z,v.w); }
    inline void sendUniform4(const char* l, const glm::uvec4& v) { glUniform4ui(getUniformLocUnsafe(l), v.x, v.y, v.z, v.w); }
    inline void sendUniform4Safe(const char* l, const glm::dvec4& v){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4d(o,v.x,v.y,v.z,v.w); }
    inline void sendUniform4Safe(const char* l, const glm::ivec4& v){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4i(o,v.x,v.y,v.z,v.w); }
    inline void sendUniform4Safe(const char* l, const glm::vec4& v){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4f(o,v.x,v.y,v.z,v.w); }
    inline void sendUniform4Safe(const char* l, const glm::uvec4& v) { const auto& o = getUniformLoc(l); if (o == -1)return; glUniform4ui(o, v.x, v.y, v.z, v.w); }
    inline void sendUniform4Force(const char* l, const glm::dvec4& v){ glUniform4d(getUniformLoc(l),v.x,v.y,v.z,v.w); }
    inline void sendUniform4Force(const char* l, const glm::ivec4& v){ glUniform4i(getUniformLoc(l),v.x,v.y,v.z,v.w); }
    inline void sendUniform4Force(const char* l, const glm::vec4& v){ glUniform4f(getUniformLoc(l),v.x,v.y,v.z,v.w); }
    inline void sendUniform4Force(const char* l, const glm::uvec4& v) { glUniform4ui(getUniformLoc(l), v.x, v.y, v.z, v.w); }

    inline void sendUniform4(const char* l, glm::dvec4&& v) { glUniform4d(getUniformLocUnsafe(l), v.x, v.y, v.z, v.w); }
    inline void sendUniform4(const char* l, glm::ivec4&& v) { glUniform4i(getUniformLocUnsafe(l), v.x, v.y, v.z, v.w); }
    inline void sendUniform4(const char* l, glm::vec4&& v) { glUniform4f(getUniformLocUnsafe(l), v.x, v.y, v.z, v.w); }
    inline void sendUniform4(const char* l, glm::uvec4&& v) { glUniform4ui(getUniformLocUnsafe(l), v.x, v.y, v.z, v.w); }
    inline void sendUniform4Safe(const char* l, glm::dvec4&& v) { const auto& o = getUniformLoc(l); if (o == -1)return; glUniform4d(o, v.x, v.y, v.z, v.w); }
    inline void sendUniform4Safe(const char* l, glm::ivec4&& v) { const auto& o = getUniformLoc(l); if (o == -1)return; glUniform4i(o, v.x, v.y, v.z, v.w); }
    inline void sendUniform4Safe(const char* l, glm::vec4&& v) { const auto& o = getUniformLoc(l); if (o == -1)return; glUniform4f(o, v.x, v.y, v.z, v.w); }
    inline void sendUniform4Safe(const char* l, glm::uvec4&& v) { const auto& o = getUniformLoc(l); if (o == -1)return; glUniform4ui(o, v.x, v.y, v.z, v.w); }
    inline void sendUniform4Force(const char* l, glm::dvec4&& v) { glUniform4d(getUniformLoc(l), v.x, v.y, v.z, v.w); }
    inline void sendUniform4Force(const char* l, glm::ivec4&& v) { glUniform4i(getUniformLoc(l), v.x, v.y, v.z, v.w); }
    inline void sendUniform4Force(const char* l, glm::vec4&& v) { glUniform4f(getUniformLoc(l), v.x, v.y, v.z, v.w); }
    inline void sendUniform4Force(const char* l, glm::uvec4&& v) { glUniform4ui(getUniformLoc(l), v.x, v.y, v.z, v.w); }

    //separate
    inline void sendUniform4(const char* l,double x,double y,double z,double w){ glUniform4d(getUniformLocUnsafe(l),x,y,z,w); }
    inline void sendUniform4(const char* l,int x,int y,int z,int w){ glUniform4i(getUniformLocUnsafe(l),x,y,z,w); }
    inline void sendUniform4(const char* l,float x,float y,float z,float w){ glUniform4f(getUniformLocUnsafe(l),x,y,z,w); }
    inline void sendUniform4(const char* l, uint32_t x, uint32_t y, uint32_t z, uint32_t w) { glUniform4ui(getUniformLocUnsafe(l), x, y, z, w); }
    inline void sendUniform4Safe(const char* l,double x,double y,double z,double w){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4d(o,x,y,z,w); }
    inline void sendUniform4Safe(const char* l,int x,int y,int z,int w){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4i(o,x,y,z,w); }
    inline void sendUniform4Safe(const char* l,float x,float y,float z,float w){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4f(o,x,y,z,w); }
    inline void sendUniform4Safe(const char* l, uint32_t x, uint32_t y, uint32_t z, uint32_t w) { const auto& o = getUniformLoc(l); if (o == -1)return; glUniform4ui(o, x, y, z, w); }
    inline void sendUniform4Force(const char* l,double x,double y,double z,double w){ glUniform4d(getUniformLoc(l),x,y,z,w); }
    inline void sendUniform4Force(const char* l,int x,int y,int z,int w){ glUniform4i(getUniformLoc(l),x,y,z,w); }
    inline void sendUniform4Force(const char* l,float x,float y,float z,float w){ glUniform4f(getUniformLoc(l),x,y,z,w); }
    inline void sendUniform4Force(const char* l, uint32_t x, uint32_t y, uint32_t z, uint32_t w) { glUniform4ui(getUniformLoc(l), x, y, z, w); }
    //vector / array of values
    inline void sendUniform4v(const char* l, const std::vector<glm::dvec4>& d, const unsigned int i){ glUniform4dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform4v(const char* l, const std::vector<glm::ivec4>& d, const unsigned int i){ glUniform4iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform4v(const char* l, const std::vector<glm::vec4>& d, const unsigned int i){ glUniform4fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform4v(const char* l, glm::dvec4* d, const unsigned int i){ glUniform4dv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform4v(const char* l, glm::ivec4* d, const unsigned int i){ glUniform4iv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform4v(const char* l, glm::vec4* d, const unsigned int i){ glUniform4fv(getUniformLocUnsafe(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform4vSafe(const char* l, const std::vector<glm::vec4>& d, const unsigned int i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4fv(o,i,glm::value_ptr(d[0])); }
    inline void sendUniform4vSafe(const char* l, glm::vec4* d, const unsigned int i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4fv(o,i,glm::value_ptr(d[0])); }
    inline void sendUniform4vForce(const char* l, const std::vector<glm::vec4>& d, const unsigned int i){ glUniform4fv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform4vForce(const char* l, glm::vec4* d, const unsigned int i){ glUniform4fv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform4vSafe(const char* l, const std::vector<glm::dvec4>& d, const unsigned int i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4dv(o,i,glm::value_ptr(d[0])); }
    inline void sendUniform4vSafe(const char* l, glm::dvec4* d, const unsigned int i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4dv(o,i,glm::value_ptr(d[0])); }
    inline void sendUniform4vForce(const char* l, const std::vector<glm::dvec4>& d, const unsigned int i){ glUniform4dv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform4vForce(const char* l, glm::dvec4* d, const unsigned int i){ glUniform4dv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform4vSafe(const char* l, const std::vector<glm::ivec4>& d, const unsigned int i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4iv(o,i,glm::value_ptr(d[0])); }
    inline void sendUniform4vSafe(const char* l, glm::ivec4* d, const unsigned int i){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniform4iv(o,i,glm::value_ptr(d[0])); }
    inline void sendUniform4vForce(const char* l, const std::vector<glm::ivec4>& d, const unsigned int i){ glUniform4iv(getUniformLoc(l),i,glm::value_ptr(d[0])); }
    inline void sendUniform4vForce(const char* l, glm::ivec4* d, const unsigned int i){ glUniform4iv(getUniformLoc(l),i,glm::value_ptr(d[0])); }

    //Matrix 2x2
    inline void sendUniformMatrix2(const char* l, const glm::mat2& m){ glUniformMatrix2fv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
    inline void sendUniformMatrix2v(const char* l, const std::vector<glm::mat2>& m, const unsigned int count){ std::vector<float> d; d.reserve(m.size() * 4); for(auto& ma:m){const float* m = glm::value_ptr(ma);for(uint i = 0; i < 4; ++i){d.push_back(m[i]);}}glUniformMatrix2fv(getUniformLocUnsafe(l),count,0,&d[0]); }
    inline void sendUniformMatrix2(const char* l, const glm::dmat2& m){ glUniformMatrix2dv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
    inline void sendUniformMatrix2v(const char* l, const std::vector<glm::dmat2>& m, const unsigned int count){ std::vector<double> d; d.reserve(m.size() * 4); for(auto& ma:m){ const double* m = glm::value_ptr(ma);for(uint i = 0; i < 4; ++i){d.push_back(m[i]);}}glUniformMatrix2dv(getUniformLocUnsafe(l),count,0,&d[0]); }
    inline void sendUniformMatrix2Safe(const char* l, const glm::mat2& m){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniformMatrix2fv(o,1,0,glm::value_ptr(m)); }
    inline void sendUniformMatrix2vSafe(const char* l, const std::vector<glm::mat2>& m, const unsigned int count){ const auto& o=getUniformLoc(l);if(o==-1)return;std::vector<float> d; d.reserve(m.size() * 4); for(auto& ma:m){ const float* m = glm::value_ptr(ma);for(uint i = 0; i < 4; ++i){d.push_back(m[i]);}}glUniformMatrix2fv(o,count,0,&d[0]); }
    inline void sendUniformMatrix2Safe(const char* l, const glm::dmat2& m){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniformMatrix2dv(o,1,0,glm::value_ptr(m)); }
    inline void sendUniformMatrix2vSafe(const char* l, const std::vector<glm::dmat2>& m, const unsigned int count){ const auto& o=getUniformLoc(l);if(o==-1)return;std::vector<double> d; d.reserve(m.size() * 4); for(auto& ma:m){ const double* m = glm::value_ptr(ma);for(uint i = 0; i < 4; ++i){d.push_back(m[i]);}}glUniformMatrix2dv(o,count,0,&d[0]); }
    inline void sendUniformMatrix2Force(const char* l, const glm::mat2& m){ glUniformMatrix2fv(getUniformLoc(l),1,0,glm::value_ptr(m)); }
    inline void sendUniformMatrix2vForce(const char* l, const std::vector<glm::mat2>& m, const unsigned int count){ std::vector<float> d; d.reserve(m.size() * 4); for(auto& ma:m){ const float* m = glm::value_ptr(ma);for(uint i = 0; i < 4; ++i){d.push_back(m[i]);}}glUniformMatrix2fv(getUniformLoc(l),count,0,&d[0]); }
    inline void sendUniformMatrix2Force(const char* l, const glm::dmat2& m){ glUniformMatrix2dv(getUniformLoc(l),1,0,glm::value_ptr(m)); }
    inline void sendUniformMatrix2vForce(const char* l, const std::vector<glm::dmat2>& m, const unsigned int count){ std::vector<double> d; d.reserve(m.size() * 4); for(auto& ma:m){ const double* m = glm::value_ptr(ma);for(uint i = 0; i < 4; ++i){d.push_back(m[i]);}}glUniformMatrix2dv(getUniformLoc(l),count,0,&d[0]); }
      
    inline void sendUniformMatrix2(const char* l, glm::mat2&& m) { glUniformMatrix2fv(getUniformLocUnsafe(l), 1, 0, glm::value_ptr(m)); }
    inline void sendUniformMatrix2v(const char* l, std::vector<glm::mat2>&& m, const unsigned int count) { std::vector<float> d; d.reserve(m.size() * 4); for (auto& ma : m) { const float* m = glm::value_ptr(ma); for (uint i = 0; i < 4; ++i) { d.push_back(m[i]); } }glUniformMatrix2fv(getUniformLocUnsafe(l), count, 0, &d[0]); }
    inline void sendUniformMatrix2(const char* l, glm::dmat2&& m) { glUniformMatrix2dv(getUniformLocUnsafe(l), 1, 0, glm::value_ptr(m)); }
    inline void sendUniformMatrix2v(const char* l, std::vector<glm::dmat2>&& m, const unsigned int count) { std::vector<double> d; d.reserve(m.size() * 4); for (auto& ma : m) { const double* m = glm::value_ptr(ma); for (uint i = 0; i < 4; ++i) { d.push_back(m[i]); } }glUniformMatrix2dv(getUniformLocUnsafe(l), count, 0, &d[0]); }
    inline void sendUniformMatrix2Safe(const char* l, glm::mat2&& m) { const auto& o = getUniformLoc(l); if (o == -1)return; glUniformMatrix2fv(o, 1, 0, glm::value_ptr(m)); }
    inline void sendUniformMatrix2vSafe(const char* l, std::vector<glm::mat2>&& m, const unsigned int count) { const auto& o = getUniformLoc(l); if (o == -1)return; std::vector<float> d; d.reserve(m.size() * 4); for (auto& ma : m) { const float* m = glm::value_ptr(ma); for (uint i = 0; i < 4; ++i) { d.push_back(m[i]); } }glUniformMatrix2fv(o, count, 0, &d[0]); }
    inline void sendUniformMatrix2Safe(const char* l, glm::dmat2&& m) { const auto& o = getUniformLoc(l); if (o == -1)return; glUniformMatrix2dv(o, 1, 0, glm::value_ptr(m)); }
    inline void sendUniformMatrix2vSafe(const char* l, std::vector<glm::dmat2>&& m, const unsigned int count) { const auto& o = getUniformLoc(l); if (o == -1)return; std::vector<double> d; d.reserve(m.size() * 4); for (auto& ma : m) { const double* m = glm::value_ptr(ma); for (uint i = 0; i < 4; ++i) { d.push_back(m[i]); } }glUniformMatrix2dv(o, count, 0, &d[0]); }
    inline void sendUniformMatrix2Force(const char* l, glm::mat2&& m) { glUniformMatrix2fv(getUniformLoc(l), 1, 0, glm::value_ptr(m)); }
    inline void sendUniformMatrix2vForce(const char* l, std::vector<glm::mat2>&& m, const unsigned int count) { std::vector<float> d; d.reserve(m.size() * 4); for (auto& ma : m) { const float* m = glm::value_ptr(ma); for (uint i = 0; i < 4; ++i) { d.push_back(m[i]); } }glUniformMatrix2fv(getUniformLoc(l), count, 0, &d[0]); }
    inline void sendUniformMatrix2Force(const char* l, glm::dmat2&& m) { glUniformMatrix2dv(getUniformLoc(l), 1, 0, glm::value_ptr(m)); }
    inline void sendUniformMatrix2vForce(const char* l, std::vector<glm::dmat2>&& m, const unsigned int count) { std::vector<double> d; d.reserve(m.size() * 4); for (auto& ma : m) { const double* m = glm::value_ptr(ma); for (uint i = 0; i < 4; ++i) { d.push_back(m[i]); } }glUniformMatrix2dv(getUniformLoc(l), count, 0, &d[0]); }

    //Matrix 3x3
    inline void sendUniformMatrix3(const char* l, const glm::mat3& m){ glUniformMatrix3fv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
    inline void sendUniformMatrix3v(const char* l, const std::vector<glm::mat3>& m, const unsigned int count){ std::vector<float> d; d.reserve(m.size() * 9); for(auto& ma:m){ const float* m = glm::value_ptr(ma);for(uint i = 0; i < 9; ++i){d.push_back(m[i]);}}glUniformMatrix3fv(getUniformLocUnsafe(l),count,0,&d[0]); }
    inline void sendUniformMatrix3(const char* l, const glm::dmat3& m){ glUniformMatrix3dv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
    inline void sendUniformMatrix3v(const char* l, const std::vector<glm::dmat3>& m, const unsigned int count){ std::vector<double> d; d.reserve(m.size() * 9); for(auto& ma:m){ const double* m = glm::value_ptr(ma);for(uint i = 0; i < 9; ++i){d.push_back(m[i]);}}glUniformMatrix3dv(getUniformLocUnsafe(l),count,0,&d[0]); }
    inline void sendUniformMatrix3Safe(const char* l, const glm::mat3& m){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniformMatrix3fv(o,1,0,glm::value_ptr(m)); }
    inline void sendUniformMatrix3vSafe(const char* l, const std::vector<glm::mat3>& m, const unsigned int count){ const auto& o=getUniformLoc(l);if(o==-1)return;std::vector<float> d; d.reserve(m.size() * 9); for(auto& ma:m){ const float* m = glm::value_ptr(ma);for(uint i = 0; i < 9; ++i){d.push_back(m[i]);}}glUniformMatrix3fv(o,count,0,&d[0]); }
    inline void sendUniformMatrix3Safe(const char* l, const glm::dmat3& m){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniformMatrix3dv(o,1,0,glm::value_ptr(m)); }
    inline void sendUniformMatrix3vSafe(const char* l, const std::vector<glm::dmat3>& m, const unsigned int count){ const auto& o=getUniformLoc(l);if(o==-1)return;std::vector<double> d; d.reserve(m.size() * 9); for(auto& ma:m){ const double* m = glm::value_ptr(ma);for(uint i = 0; i < 9; ++i){d.push_back(m[i]);}}glUniformMatrix3dv(o,count,0,&d[0]); }
    inline void sendUniformMatrix3Force(const char* l, const glm::mat3& m){ glUniformMatrix3fv(getUniformLoc(l),1,0,glm::value_ptr(m)); }
    inline void sendUniformMatrix3vForce(const char* l, const std::vector<glm::mat3>& m, const unsigned int count){ std::vector<float> d; d.reserve(m.size() * 9); for(auto& ma:m){ const float* m = glm::value_ptr(ma);for(uint i = 0; i < 9; ++i){d.push_back(m[i]);}}glUniformMatrix3fv(getUniformLoc(l),count,0,&d[0]); }
    inline void sendUniformMatrix3Force(const char* l, const glm::dmat3& m){ glUniformMatrix3dv(getUniformLoc(l),1,0,glm::value_ptr(m)); }
    inline void sendUniformMatrix3vForce(const char* l, const std::vector<glm::dmat3>& m, const unsigned int count) { std::vector<double> d; d.reserve(m.size() * 9); for (auto& ma : m) { const double* m = glm::value_ptr(ma); for (uint i = 0; i < 9; ++i) { d.push_back(m[i]); } }glUniformMatrix3dv(getUniformLoc(l), count, 0, &d[0]); }
     
    inline void sendUniformMatrix3(const char* l, glm::mat3&& m) { glUniformMatrix3fv(getUniformLocUnsafe(l), 1, 0, glm::value_ptr(m)); }
    inline void sendUniformMatrix3v(const char* l, std::vector<glm::mat3>&& m, const unsigned int count) { std::vector<float> d; d.reserve(m.size() * 9); for (auto& ma : m) { const float* m = glm::value_ptr(ma); for (uint i = 0; i < 9; ++i) { d.push_back(m[i]); } }glUniformMatrix3fv(getUniformLocUnsafe(l), count, 0, &d[0]); }
    inline void sendUniformMatrix3(const char* l, glm::dmat3&& m) { glUniformMatrix3dv(getUniformLocUnsafe(l), 1, 0, glm::value_ptr(m)); }
    inline void sendUniformMatrix3v(const char* l, std::vector<glm::dmat3>&& m, const unsigned int count) { std::vector<double> d; d.reserve(m.size() * 9); for (auto& ma : m) { const double* m = glm::value_ptr(ma); for (uint i = 0; i < 9; ++i) { d.push_back(m[i]); } }glUniformMatrix3dv(getUniformLocUnsafe(l), count, 0, &d[0]); }
    inline void sendUniformMatrix3Safe(const char* l, glm::mat3&& m) { const auto& o = getUniformLoc(l); if (o == -1)return; glUniformMatrix3fv(o, 1, 0, glm::value_ptr(m)); }
    inline void sendUniformMatrix3vSafe(const char* l, std::vector<glm::mat3>&& m, const unsigned int count) { const auto& o = getUniformLoc(l); if (o == -1)return; std::vector<float> d; d.reserve(m.size() * 9); for (auto& ma : m) { const float* m = glm::value_ptr(ma); for (uint i = 0; i < 9; ++i) { d.push_back(m[i]); } }glUniformMatrix3fv(o, count, 0, &d[0]); }
    inline void sendUniformMatrix3Safe(const char* l, glm::dmat3&& m) { const auto& o = getUniformLoc(l); if (o == -1)return; glUniformMatrix3dv(o, 1, 0, glm::value_ptr(m)); }
    inline void sendUniformMatrix3vSafe(const char* l, std::vector<glm::dmat3>&& m, const unsigned int count) { const auto& o = getUniformLoc(l); if (o == -1)return; std::vector<double> d; d.reserve(m.size() * 9); for (auto& ma : m) { const double* m = glm::value_ptr(ma); for (uint i = 0; i < 9; ++i) { d.push_back(m[i]); } }glUniformMatrix3dv(o, count, 0, &d[0]); }
    inline void sendUniformMatrix3Force(const char* l, glm::mat3&& m) { glUniformMatrix3fv(getUniformLoc(l), 1, 0, glm::value_ptr(m)); }
    inline void sendUniformMatrix3vForce(const char* l, std::vector<glm::mat3>&& m, const unsigned int count) { std::vector<float> d; d.reserve(m.size() * 9); for (auto& ma : m) { const float* m = glm::value_ptr(ma); for (uint i = 0; i < 9; ++i) { d.push_back(m[i]); } }glUniformMatrix3fv(getUniformLoc(l), count, 0, &d[0]); }
    inline void sendUniformMatrix3Force(const char* l, glm::dmat3&& m) { glUniformMatrix3dv(getUniformLoc(l), 1, 0, glm::value_ptr(m)); }
    inline void sendUniformMatrix3vForce(const char* l, std::vector<glm::dmat3>&& m, const unsigned int count) { std::vector<double> d; d.reserve(m.size() * 9); for (auto& ma : m) { const double* m = glm::value_ptr(ma); for (uint i = 0; i < 9; ++i) { d.push_back(m[i]); } }glUniformMatrix3dv(getUniformLoc(l), count, 0, &d[0]); }

    //Matrix4x4
    inline void sendUniformMatrix4(const char* l,const glm::mat4& m){ glUniformMatrix4fv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
    inline void sendUniformMatrix4v(const char* l, const std::vector<glm::mat4>& m, const unsigned int count){ std::vector<float> d; d.reserve(m.size() * 16);for(auto& ma:m){ auto* m = glm::value_ptr(ma);for(uint i = 0; i < 16; ++i){d.push_back(m[i]);}}glUniformMatrix4fv(getUniformLocUnsafe(l),count,0,&d[0]); }
    inline void sendUniformMatrix4(const char* l, const glm::dmat4& m){ glUniformMatrix4dv(getUniformLocUnsafe(l),1,0,glm::value_ptr(m)); }
    inline void sendUniformMatrix4v(const char* l, const std::vector<glm::dmat4>& m, const unsigned int count){ std::vector<double> d; d.reserve(m.size() * 16);for(auto& ma:m){ auto* m = glm::value_ptr(ma);for(uint i = 0; i < 16; ++i){d.push_back(m[i]);}}glUniformMatrix4dv(getUniformLocUnsafe(l),count,0,&d[0]); }
    inline void sendUniformMatrix4Safe(const char* l, const glm::mat4& m){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniformMatrix4fv(o,1,0,glm::value_ptr(m)); }
    inline void sendUniformMatrix4vSafe(const char* l, const std::vector<glm::mat4>& m, const unsigned int count){ const auto& o=getUniformLoc(l);if(o==-1)return;std::vector<float> d; d.reserve(m.size() * 16);for(auto& ma:m){ auto* m = glm::value_ptr(ma);for(uint i = 0; i < 16; ++i){d.push_back(m[i]);}}glUniformMatrix4fv(o,count,0,&d[0]); }
    inline void sendUniformMatrix4Safe(const char* l, const glm::dmat4& m){ const auto& o=getUniformLoc(l);if(o==-1)return;glUniformMatrix4dv(o,1,0,glm::value_ptr(m)); }
    inline void sendUniformMatrix4vSafe(const char* l, const std::vector<glm::dmat4>& m, const unsigned int count){ const auto& o=getUniformLoc(l);if(o==-1)return;std::vector<double> d; d.reserve(m.size() * 16);for(auto& ma:m){ auto* m = glm::value_ptr(ma);for(uint i = 0; i < 16; ++i){d.push_back(m[i]);}}glUniformMatrix4dv(o,count,0,&d[0]); }
    inline void sendUniformMatrix4Force(const char* l, const glm::mat4& m){ glUniformMatrix4fv(getUniformLoc(l),1,0,glm::value_ptr(m)); }
    inline void sendUniformMatrix4vForce(const char* l, const std::vector<glm::mat4>& m, const unsigned int count){ std::vector<float> d; d.reserve(m.size() * 16);for(auto& ma:m){ auto* m = glm::value_ptr(ma);for(uint i = 0; i < 16; ++i){d.push_back(m[i]);}}glUniformMatrix4fv(getUniformLoc(l),count,0,&d[0]); }
    inline void sendUniformMatrix4Force(const char* l, const glm::dmat4& m){ glUniformMatrix4dv(getUniformLoc(l),1,0,glm::value_ptr(m)); }
    inline void sendUniformMatrix4vForce(const char* l, const std::vector<glm::dmat4>& m, const unsigned int count){ std::vector<double> d; d.reserve(m.size() * 16);for(auto& ma:m){ auto* m = glm::value_ptr(ma);for(uint i = 0; i < 16; ++i){d.push_back(m[i]);}}glUniformMatrix4dv(getUniformLoc(l),count,0,&d[0]); }
    
    inline void sendUniformMatrix4(const char* l, glm::mat4&& m) { glUniformMatrix4fv(getUniformLocUnsafe(l), 1, 0, glm::value_ptr(m)); }
    inline void sendUniformMatrix4v(const char* l, std::vector<glm::mat4>&& m, const unsigned int count) { std::vector<float> d; d.reserve(m.size() * 16); for (auto& ma : m) { auto* m = glm::value_ptr(ma); for (uint i = 0; i < 16; ++i) { d.push_back(m[i]); } }glUniformMatrix4fv(getUniformLocUnsafe(l), count, 0, &d[0]); }
    inline void sendUniformMatrix4(const char* l, glm::dmat4&& m) { glUniformMatrix4dv(getUniformLocUnsafe(l), 1, 0, glm::value_ptr(m)); }
    inline void sendUniformMatrix4v(const char* l, std::vector<glm::dmat4>&& m, const unsigned int count) { std::vector<double> d; d.reserve(m.size() * 16); for (auto& ma : m) { auto* m = glm::value_ptr(ma); for (uint i = 0; i < 16; ++i) { d.push_back(m[i]); } }glUniformMatrix4dv(getUniformLocUnsafe(l), count, 0, &d[0]); }
    inline void sendUniformMatrix4Safe(const char* l, glm::mat4&& m) { const auto& o = getUniformLoc(l); if (o == -1)return; glUniformMatrix4fv(o, 1, 0, glm::value_ptr(m)); }
    inline void sendUniformMatrix4vSafe(const char* l, std::vector<glm::mat4>&& m, const unsigned int count) { const auto& o = getUniformLoc(l); if (o == -1)return; std::vector<float> d; d.reserve(m.size() * 16); for (auto& ma : m) { auto* m = glm::value_ptr(ma); for (uint i = 0; i < 16; ++i) { d.push_back(m[i]); } }glUniformMatrix4fv(o, count, 0, &d[0]); }
    inline void sendUniformMatrix4Safe(const char* l, glm::dmat4&& m) { const auto& o = getUniformLoc(l); if (o == -1)return; glUniformMatrix4dv(o, 1, 0, glm::value_ptr(m)); }
    inline void sendUniformMatrix4vSafe(const char* l, std::vector<glm::dmat4>&& m, const unsigned int count) { const auto& o = getUniformLoc(l); if (o == -1)return; std::vector<double> d; d.reserve(m.size() * 16); for (auto& ma : m) { auto* m = glm::value_ptr(ma); for (uint i = 0; i < 16; ++i) { d.push_back(m[i]); } }glUniformMatrix4dv(o, count, 0, &d[0]); }
    inline void sendUniformMatrix4Force(const char* l, glm::mat4&& m) { glUniformMatrix4fv(getUniformLoc(l), 1, 0, glm::value_ptr(m)); }
    inline void sendUniformMatrix4vForce(const char* l, std::vector<glm::mat4>&& m, const unsigned int count) { std::vector<float> d; d.reserve(m.size() * 16); for (auto& ma : m) { auto* m = glm::value_ptr(ma); for (uint i = 0; i < 16; ++i) { d.push_back(m[i]); } }glUniformMatrix4fv(getUniformLoc(l), count, 0, &d[0]); }
    inline void sendUniformMatrix4Force(const char* l, glm::dmat4&& m) { glUniformMatrix4dv(getUniformLoc(l), 1, 0, glm::value_ptr(m)); }
    inline void sendUniformMatrix4vForce(const char* l, std::vector<glm::dmat4>&& m, const unsigned int count) { std::vector<double> d; d.reserve(m.size() * 16); for (auto& ma : m) { auto* m = glm::value_ptr(ma); for (uint i = 0; i < 16; ++i) { d.push_back(m[i]); } }glUniformMatrix4dv(getUniformLoc(l), count, 0, &d[0]); }

    #pragma endregion
};
};
#endif
