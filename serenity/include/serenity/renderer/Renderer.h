#pragma once
#ifndef ENGINE_ENGINE_RENDERER_H
#define ENGINE_ENGINE_RENDERER_H

class  Viewport;
class  Texture;
class  TextureCubemap;
class  Font;
class  Decal;
class  BufferObject;
struct EngineOptions;
class  ModelInstance;
namespace Engine::priv {
    class  GBuffer;
    class  FramebufferObject;
    class  RenderbufferObject;
    class  IRenderingPipeline;
    class  RenderModule;
};

#include <serenity/renderer/RendererState.h>
#include <serenity/utils/Utils.h>
#include <serenity/renderer/RendererIncludes.h>
#include <serenity/resources/font/FontIncludes.h>
#include <serenity/renderer/pipelines/IRenderingPipeline.h>
#include <serenity/resources/texture/TextureIncludes.h>
#include <serenity/lights/LightIncludes.h>
#include <serenity/types/Types.h>
#include <glm/gtc/type_ptr.hpp>

/*
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;        //enable dedicated graphics for nvidia
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1; //enable dedicated graphics for amd radeon
}
*/

namespace Engine::priv {
    class RenderModule final {
        public:
            std::unique_ptr<IRenderingPipeline>  m_Pipeline;
            AntiAliasingAlgorithm                m_AA_algorithm          = AntiAliasingAlgorithm::FXAA;
            LightingAlgorithm                    m_LightingAlgorithm     = LightingAlgorithm::PBR;
            DepthFunc                            m_Depth_Function        = DepthFunc::Less;
            float                                m_Gamma                 = 2.2f;
            float                                m_GI_Diffuse            = 1.0f;
            float                                m_GI_Specular           = 1.0f;
            float                                m_GI_Global             = 1.0f;
            float                                m_GI_Pack               = 0.0f;

            float                                m_ShadowZMultFactor     = 1.5f;
            float                                m_ShadowClipspaceOffset = 1.0f;

            bool                                 m_Lighting              = true;
            bool                                 m_DrawSkybox            = true;
            bool                                 m_DrawPhysicsDebug      = false;
        public:
            RenderModule(const EngineOptions&, uint32_t windowWidth, uint32_t windowHeight);

            void init(uint32_t windowWidth, uint32_t windowHeight);

            void _resize(uint32_t width, uint32_t height);

            static void render(RenderModule&, Viewport&, bool mainRenderFunc = true);
            static void render2DAPI(RenderModule&, Viewport&, bool mainRenderFunc = true);

            bool setShadowCaster(SunLight&,         bool isShadowCaster, uint32_t shadowMapWidth, uint32_t shadowMapSize, LightShadowFrustumType, float nearFactor, float farFactor);
            bool setShadowCaster(PointLight&,       bool isShadowCaster);
            bool setShadowCaster(DirectionalLight&, bool isShadowCaster, uint32_t shadowMapWidth, uint32_t shadowMapSize, LightShadowFrustumType, float nearFactor, float farFactor);
            bool setShadowCaster(SpotLight&,        bool isShadowCaster);
            bool setShadowCaster(RodLight&,         bool isShadowCaster);
            bool setShadowCaster(ProjectionLight&,  bool isShadowCaster);

            void _clear2DAPICommands();
            void _sort2DAPICommands();

            bool bind(ModelInstance*) const;
            bool bind(ShaderProgram*) const;
            bool bind(Mesh*) const;
            bool bind(Material*) const;

            bool unbind(Material*) const;
            bool unbind(Mesh*) const;
            bool unbind(ShaderProgram*) const;
            bool unbind(ModelInstance*) const;

            inline float _getGIPackedData() noexcept { return m_GI_Pack; }
            void _genPBREnvMapData(TextureCubemap&, Handle convolutionTexture, Handle preEnvTexture, uint32_t, uint32_t);
    };
}

namespace Engine::Renderer {
    namespace Settings {

        void setGamma(const float gamma);
        float getGamma();
        void clear(bool color = true, bool depth = true, bool stencil = true);
        void applyGlobalAnisotropicFiltering(float filtering);
        bool setAntiAliasingAlgorithm(AntiAliasingAlgorithm);
        void enableDrawPhysicsInfo(bool enabled = true);
        void disableDrawPhysicsInfo();
        void enableSkybox(bool enabled = true);
        void disableSkybox();

        namespace Lighting {
            void enable(bool enabled = true);
            void disable();
            float getGIContributionGlobal();
            void setGIContributionGlobal(float giGlobal);
            float getGIContributionDiffuse();
            void setGIContributionDiffuse(float giDiffuse);
            float getGIContributionSpecular();
            void setGIContributionSpecular(float giSpecular);
            void setGIContribution(float global, float diffuse, float specular);
            LightingAlgorithm getLightingAlgorithm();
            bool setLightingAlgorithm(LightingAlgorithm);
        }
    }
    void restoreCurrentState();

    //set width and height to be <= 0.0f to use the window's dimensions automatically
    void renderFullscreenQuad(float width = 0.0f, float height = 0.0f, float depth = 0.0f, float inNear = 0.0f, float inFar = 1.0f);

    //set width and height to be <= 0.0f to use the window's dimensions automatically
    void renderFullscreenQuadCentered(float width = 0.0f, float height = 0.0f, float depth = 0.0f, float inNear = 0.0f, float inFar = 1.0f);

    void renderFullscreenTriangle(float depth = 0.0f, float inNear = 0.0f, float inFar = 1.0f);

    uint32_t getUniformLoc(const char* location);
    uint32_t getUniformLocUnsafe(const char* location);
    uint32_t getCurrentlyBoundTextureOfType(uint32_t textureType) noexcept;

    bool cullFace(GLenum state);
    bool setDepthFunc(GLenum func);
    bool setViewport(float x, float y, float width, float height);
    bool setViewport(const glm::vec4& dimensions);

    template<class X, class Y = X, class W = X, class H = X>
    inline bool setViewport(X&& x, Y&& y, W&& width, H&& height) noexcept { 
        return setViewport(
            static_cast<float>(std::forward<X>(x)), 
            static_cast<float>(std::forward<Y>(y)),
            static_cast<float>(std::forward<W>(width)),
            static_cast<float>(std::forward<H>(height))
        ); 
    }


    /*
    func - sets the stencil test function that determines whether a fragment passes or is discarded. This test function is applied to the stored stencil value
           and the glStencilFunc's ref value. Possible options are: GL_NEVER, GL_LESS, GL_LEQUAL, GL_GREATER, GL_GEQUAL, GL_EQUAL, GL_NOTEQUAL and GL_ALWAYS.
           The semantic meaning of these is similar to the depth buffer's functions.

    ref - specifies the reference value for the stencil test. The stencil buffer's content is compared to this value.

    mask - specifies a mask that is ANDed with both the reference value and the stored stencil value before the test compares them. Initially set to all 1s.
    */
    bool stencilFunc(GLenum func, GLint ref, GLuint mask);

    /*
    Allows us to set a bitmask that is ANDed with the stencil value about to be written to the buffer.
    By default this is set to a bitmask of all 1s - not affecting the output. if set to 0, then this basically
    ANDS the stencil value with zeros, resulting in zero and disabling stencil writes
    */
    bool stencilMask(GLuint mask);

    /*
    sFail - Specifies the action to take when the stencil test fails. Eight symbolic constants are accepted:
            GL_KEEP, GL_ZERO, GL_REPLACE, GL_INCR, GL_INCR_WRAP, GL_DECR, GL_DECR_WRAP, and GL_INVERT.
            The initial value is GL_KEEP.

    sPassDepthFail - Specifies the stencil action when the stencil test passes, but the depth test fails.
                     sPassDepthFail accepts the same symbolic constants as sFail. The initial value is GL_KEEP.

    sPassDepthPass - Specifies the stencil action when both the stencil test and the depth test pass,
                     or when the stencil test passes and either there is no depth buffer or depth testing is not enabled.
                     sPassDepthPass accepts the same symbolic constants as sfail. The initial value is GL_KEEP.
    */
    bool stencilOp(GLenum sFail, GLenum sPassDepthFail, GLenum sPassDepthPass);

    void bindFBO(GLuint fbo);
    void bindFBO(const priv::FramebufferObject&);
    bool bindRBO(GLuint rbo);
    bool bindRBO(priv::RenderbufferObject&);
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

    bool bindTextureForModification(TextureType, GLuint textureObject);

    bool bindVAO(GLuint vaoObject);
    void genAndBindTexture(TextureType, GLuint& textureObject);
    void generateVAO(GLuint& vaoObject);
    bool colorMask(bool r, bool g, bool b, bool a);
    bool clearColor(float r, float g, float b, float a);

    void clearTexture(int slot, GLuint glTextureType);
    void sendTexture(const char* location, Texture&, int slot);
    void sendTexture(const char* location, TextureCubemap&, int slot);
    void sendTexture(const char* location, GLuint textureAddress, int slot, GLuint glTextureType);
    void sendTextureSafe(const char* location, Texture&, int slot);
    void sendTextureSafe(const char* location, TextureCubemap&, int slot);
    void sendTextureSafe(const char* location, GLuint textureAddress, int slot, GLuint glTextureType);

    void sendTextures(const char* location, const Texture**, int slot, const int arrSize);
    void sendTextures(const char* location, const TextureCubemap**, int slot, const int arrSize);
    void sendTextures(const char* location, const GLuint*, int slot, GLuint glTextureType, const int arrSize);
    void sendTexturesSafe(const char* location, const Texture**, int slot, const int arrSize);
    void sendTexturesSafe(const char* location, const TextureCubemap**, int slot, const int arrSize);
    void sendTexturesSafe(const char* location, const GLuint*, int slot, GLuint glTextureType, const int arrSize);


    void alignmentOffset(Alignment, float& x, float& y, float width, float height);

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
    void renderTexture(
        uint32_t textureAddress,
        int textureWidth,
        int textureHeight,
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
    void renderBackgroundTexture(
        uint32_t textureAddress,
        int textureWidth,
        int textureHeight,
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
    void sendUniform1(const char* l, double x);
    void sendUniform1(const char* l, int x);
    void sendUniform1(const char* l, float x);
    void sendUniform1(const char* l, uint32_t x);
    void sendUniform1Safe(const char* l, double x);
    void sendUniform1Safe(const char* l, int x);
    void sendUniform1Safe(const char* l, float x);
    void sendUniform1Safe(const char* l, uint32_t x);
    void sendUniform1v(const char* l, const double* d, const uint32_t i);
    void sendUniform1v(const char* l, const int* d, const uint32_t i);
    void sendUniform1v(const char* l, const float* d, const uint32_t i);
    void sendUniform1vSafe(const char* l, const float* d, const uint32_t i);
    void sendUniform1vSafe(const char* l, const double* d, const uint32_t i);
    void sendUniform1vSafe(const char* l, const int* d, const uint32_t i);
    template<class CONTAINER>
    void sendUniform1v(const char* l, const CONTAINER& container) { sendUniform1v(l, container.data(), uint32_t(container.size())); }
    template<class CONTAINER>
    void sendUniform1vSafe(const char* l, const CONTAINER& container) { sendUniform1vSafe(l, container.data(), uint32_t(container.size())); }


    //Uniform 2
    //vectors
    void sendUniform2(const char* l, const glm::dvec2& v);
    void sendUniform2(const char* l, const glm::ivec2& v);
    void sendUniform2(const char* l, const glm::vec2& v);
    void sendUniform2(const char* l, const glm::uvec2& v);
    void sendUniform2Safe(const char* l, const glm::dvec2& v);
    void sendUniform2Safe(const char* l, const glm::ivec2& v);
    void sendUniform2Safe(const char* l, const glm::vec2& v);
    void sendUniform2Safe(const char* l, const glm::uvec2& v);
    //separate
    void sendUniform2(const char* l, double x, double y);
    void sendUniform2(const char* l, int x, int y);
    void sendUniform2(const char* l, float x, float y);
    void sendUniform2(const char* l, uint32_t x, uint32_t y);
    void sendUniform2Safe(const char* l, double x, double y);
    void sendUniform2Safe(const char* l, int x, int y);
    void sendUniform2Safe(const char* l, float x, float y);
    void sendUniform2Safe(const char* l, uint32_t x, uint32_t y);
    void sendUniform2v(const char* l, const glm::dvec2* d, const uint32_t i);
    void sendUniform2v(const char* l, const glm::ivec2* d, const uint32_t i);
    void sendUniform2v(const char* l, const glm::vec2* d, const uint32_t i);
    void sendUniform2vSafe(const char* l, const glm::vec2* d, const uint32_t i);
    void sendUniform2vSafe(const char* l, const glm::dvec2* d, const uint32_t i);
    void sendUniform2vSafe(const char* l, const glm::ivec2* d, const uint32_t i);
    template<class CONTAINER>
    void sendUniform2v(const char* l, const CONTAINER& container) { sendUniform2v(l, container.data(), uint32_t(container.size())); }
    template<class CONTAINER>
    void sendUniform2vSafe(const char* l, const CONTAINER& container) { sendUniform2vSafe(l, container.data(), uint32_t(container.size())); }


    //Uniform 3
    //vectors
    void sendUniform3(const char* l, const glm::dvec3& v);
    void sendUniform3(const char* l, const glm::ivec3& v);
    void sendUniform3(const char* l, const glm::vec3& v);
    void sendUniform3(const char* l, const glm::uvec3& v);
    void sendUniform3Safe(const char* l, const glm::dvec3& v);
    void sendUniform3Safe(const char* l, const glm::ivec3& v);
    void sendUniform3Safe(const char* l, const glm::vec3& v);
    void sendUniform3Safe(const char* l, const glm::uvec3& v);
    //separate
    void sendUniform3(const char* l, double x, double y, double z);
    void sendUniform3(const char* l, int x, int y, int z);
    void sendUniform3(const char* l, float x, float y, float z);
    void sendUniform3(const char* l, uint32_t x, uint32_t y, uint32_t z);
    void sendUniform3Safe(const char* l, double x, double y, double z);
    void sendUniform3Safe(const char* l, int x, int y, int z);
    void sendUniform3Safe(const char* l, float x, float y, float z);
    void sendUniform3Safe(const char* l, uint32_t x, uint32_t y, uint32_t z);
    void sendUniform3v(const char* l, const glm::dvec3* d, const uint32_t i);
    void sendUniform3v(const char* l, const glm::ivec3* d, const uint32_t i);
    void sendUniform3v(const char* l, const glm::vec3* d, const uint32_t i);
    void sendUniform3vSafe(const char* l, const glm::vec3* d, const uint32_t i);
    void sendUniform3vSafe(const char* l, const glm::dvec3* d, const uint32_t i);
    void sendUniform3vSafe(const char* l, const glm::ivec3* d, const uint32_t i);
    template<class CONTAINER>
    void sendUniform3v(const char* l, const CONTAINER& container) { sendUniform3v(l, container.data(), uint32_t(container.size())); }
    template<class CONTAINER>
    void sendUniform3vSafe(const char* l, const CONTAINER& container) { sendUniform3vSafe(l, container.data(), uint32_t(container.size())); }


    //Uniform 4
    //vectors
    void sendUniform4(const char* l, const glm::dvec4& v);
    void sendUniform4(const char* l, const glm::ivec4& v);
    void sendUniform4(const char* l, const glm::vec4& v);
    void sendUniform4(const char* l, const glm::uvec4& v);
    void sendUniform4Safe(const char* l, const glm::dvec4& v);
    void sendUniform4Safe(const char* l, const glm::ivec4& v);
    void sendUniform4Safe(const char* l, const glm::vec4& v);
    void sendUniform4Safe(const char* l, const glm::uvec4& v);
    //separate
    void sendUniform4(const char* l, double x, double y, double z, double w);
    void sendUniform4(const char* l, int x, int y, int z, int w);
    void sendUniform4(const char* l, float x, float y, float z, float w);
    void sendUniform4(const char* l, uint32_t x, uint32_t y, uint32_t z, uint32_t w);
    void sendUniform4Safe(const char* l, double x, double y, double z, double w);
    void sendUniform4Safe(const char* l, int x, int y, int z, int w);
    void sendUniform4Safe(const char* l, float x, float y, float z, float w);
    void sendUniform4Safe(const char* l, uint32_t x, uint32_t y, uint32_t z, uint32_t w);
    void sendUniform4v(const char* l, const glm::dvec4* d, const uint32_t i);
    void sendUniform4v(const char* l, const glm::ivec4* d, const uint32_t i);
    void sendUniform4v(const char* l, const glm::vec4* d, const uint32_t i);
    void sendUniform4vSafe(const char* l, const glm::vec4* d, const uint32_t i);
    void sendUniform4vSafe(const char* l, const glm::dvec4* d, const uint32_t i);
    void sendUniform4vSafe(const char* l, const glm::ivec4* d, const uint32_t i);
    template<class CONTAINER>
    void sendUniform4v(const char* l, const CONTAINER& container) { sendUniform4v(l, container.data(), uint32_t(container.size())); }
    template<class CONTAINER>
    void sendUniform4vSafe(const char* l, const CONTAINER& container) { sendUniform4vSafe(l, container.data(), uint32_t(container.size())); }

    //Matrix 2x2
    void sendUniformMatrix2(const char* l, const glm::mat2& m);
    void sendUniformMatrix2(const char* l, const glm::dmat2& m);
    void sendUniformMatrix2Safe(const char* l, const glm::mat2& m);
    void sendUniformMatrix2Safe(const char* l, const glm::dmat2& m);
    void sendUniformMatrix2v(const char* l, const glm::mat2* m, const uint32_t count);
    void sendUniformMatrix2v(const char* l, const glm::dmat2* m, const uint32_t count);
    void sendUniformMatrix2vSafe(const char* l, const glm::mat2* m, const uint32_t count);
    void sendUniformMatrix2vSafe(const char* l, const glm::dmat2* m, const uint32_t count);
    template<class CONTAINER>
    void sendUniformMatrix2v(const char* l, const CONTAINER& container) { sendUniformMatrix2v(l, container.data(), uint32_t(container.size())); }
    template<class CONTAINER>
    void sendUniformMatrix2vSafe(const char* l, const CONTAINER& container) { sendUniformMatrix2vSafe(l, container.data(), uint32_t(container.size())); }

    //Matrix 3x3
    void sendUniformMatrix3(const char* l, const glm::mat3& m);
    void sendUniformMatrix3(const char* l, const glm::dmat3& m);
    void sendUniformMatrix3Safe(const char* l, const glm::mat3& m);
    void sendUniformMatrix3Safe(const char* l, const glm::dmat3& m);
    void sendUniformMatrix3v(const char* l, const glm::mat3* m, const uint32_t count);
    void sendUniformMatrix3v(const char* l, const glm::dmat3* m, const uint32_t count);
    void sendUniformMatrix3vSafe(const char* l, const glm::mat3* m, const uint32_t count);
    void sendUniformMatrix3vSafe(const char* l, const glm::dmat3* m, const uint32_t count);
    template<class CONTAINER>
    void sendUniformMatrix3v(const char* l, const CONTAINER& container) { sendUniformMatrix3v(l, container.data(), uint32_t(container.size())); }
    template<class CONTAINER>
    void sendUniformMatrix3vSafe(const char* l, const CONTAINER& container) { sendUniformMatrix3vSafe(l, container.data(), uint32_t(container.size())); }

    //Matrix4x4
    void sendUniformMatrix4(const char* l, const glm::mat4& m);
    void sendUniformMatrix4(const char* l, const glm::dmat4& m);
    void sendUniformMatrix4Safe(const char* l, const glm::mat4& m);
    void sendUniformMatrix4Safe(const char* l, const glm::dmat4& m);
    void sendUniformMatrix4v(const char* l, const glm::mat4* m, const uint32_t count);
    void sendUniformMatrix4v(const char* l, const glm::dmat4* m, const uint32_t count);
    void sendUniformMatrix4vSafe(const char* l, const glm::mat4* m, const uint32_t count);
    void sendUniformMatrix4vSafe(const char* l, const glm::dmat4* m, const uint32_t count);
    template<class CONTAINER>
    void sendUniformMatrix4v(const char* l, const CONTAINER& container) { sendUniformMatrix4v(l, container.data(), uint32_t(container.size())); }
    template<class CONTAINER>
    void sendUniformMatrix4vSafe(const char* l, const CONTAINER& container) { sendUniformMatrix4vSafe(l, container.data(), uint32_t(container.size())); }

    #pragma endregion
}


#endif
