#pragma once
#ifndef ENGINE_RENDERER_DEFERRED_PIPELINE_H
#define ENGINE_RENDERER_DEFERRED_PIPELINE_H

class Camera;
class Viewport;
class UniformBufferObject;
class Shader;
class ShaderProgram;
namespace Engine::priv {
    class GBuffer;
    class FullscreenQuad;
    class FullscreenTriangle;
    class RenderModule;
};
#include <serenity/renderer/pipelines/IRenderingPipeline.h>
#include <serenity/renderer/pipelines/DeferredShadowCasters.h>
#include <serenity/model/ModelInstanceIncludes.h>
#include <serenity/renderer/opengl/State.h>
#include <serenity/renderer/opengl/Extensions.h>
#include <serenity/renderer/GBuffer.h>
#include <serenity/renderer/FullscreenItems.h>
#include <serenity/containers/PartialArray.h>
#include <serenity/resources/font/Font.h>
#include <serenity/renderer/text/TextRenderer.h>

namespace Engine::priv {
    class DeferredPipeline final : public IRenderingPipeline {
        friend class RenderModule;
        public: struct UBOCameraDataStruct final {
            glm::mat4 CameraView;
            glm::mat4 CameraProj;
            glm::mat4 CameraViewProj;
            glm::mat4 CameraInvView;
            glm::mat4 CameraInvProj;
            glm::mat4 CameraInvViewProj;
            glm::vec4 CameraInfo1;      //renderPosX, renderPosY, renderPosZ, near
            glm::vec4 CameraInfo2;      //viewVecX, viewVecY, viewVecZ, far
            glm::vec4 CameraInfo3;      //realposX, realposY, realposZ, logarithmDepthBufferFCoefficient
            glm::vec4 ScreenInfo;       //mainWindowSizeX, mainWindowSizeY, viewportSizeX, viewportSizeY
            glm::vec4 RendererInfo1;    //GIPacked, gamma, unused, unused
            glm::vec4 RendererInfo2;    //ambientR, ambientG, ambientB, unused
        };
        private:
            Engine::priv::GLDeferredLightShadowCasters         m_ShadowCasters;
            Engine::priv::RenderModule&                        m_Renderer;

            glm::vec4                                          m_CurrentScissorState = glm::vec4(-1.0f);
            float                                              m_CurrentScissorDepth = 0.0f;

            RendererState                                      m_RendererState;
            OpenGLState                                        m_OpenGLStateMachine;
            OpenGLExtensions                                   m_OpenGLExtensionsManager;

            FullscreenQuad                                     m_FullscreenQuad;
            FullscreenTriangle                                 m_FullscreenTriangle;

            TextRenderer                                       m_TextRenderer;

            UniformBufferObject*                               m_UBOCamera = nullptr;
            UBOCameraDataStruct*                               m_CameraUBODataPtr = nullptr;
            glm::mat4                                          m_2DProjectionMatrix;
            GBuffer                                            m_GBuffer;

            std::vector<Handle>                                m_InternalShaders;
            std::vector<Handle>                                m_InternalShaderPrograms;
            std::vector<int>                                   m_TextureSlotsBuffer;


            std::vector<IRenderingPipeline::API2DCommand>      m_2DAPICommands;
            std::vector<IRenderingPipeline::API2DCommand>      m_Background2DAPICommands;
            //particle instancing
            uint32_t                                           m_Particle_Instance_VBO = 0U;

            void internal_gl_scissor_reset() noexcept;
            void internal_gl_scissor(const glm::vec4& scissor, float depth) noexcept;


            void internal_render_per_frame_preparation(Viewport&, Camera&);
            void internal_pass_shadows_depth(Viewport&, Scene&, Camera&);
            void internal_init_frame_gbuffer(Viewport&, Camera&);
            void internal_pass_geometry(Viewport&, Camera&);
            void internal_pass_ssao(Viewport&, Camera&);
            void internal_pass_stencil(Viewport&);
            void internal_pass_lighting(Viewport&, Camera&, bool mainRenderFunction);
            void internal_pass_lighting_basic(Viewport&, Camera&, bool mainRenderFunction);
            void internal_pass_forward(Viewport&, Camera&, bool depthPrepass);
            void internal_pass_god_rays(Viewport&, Camera&);
            void internal_pass_hdr(Viewport&, Camera&, GBufferType::Type outTexture, GBufferType::Type outTexture2);
            void internal_pass_bloom(Viewport&, GBufferType::Type sceneTexture);
            void internal_pass_depth_of_field(Viewport&, GBufferType::Type& sceneTexture, GBufferType::Type& outTexture);
            void internal_pass_aa(bool mainRenderFunction, Viewport&, Camera&, GBufferType::Type& sceneTexture, GBufferType::Type& outTexture);
            void internal_pass_final(Viewport&, GBufferType::Type sceneTexture);
            void internal_pass_depth_and_transparency(Viewport&, GBufferType::Type sceneTexture); //TODO: recheck this
            bool internal_pass_depth_prepass(Viewport&, Camera&);
            void internal_pass_blur(Viewport&, GLuint texture, std::string_view type);
            void internal_pass_normaless_diffuse(Viewport&);

            void internal_generate_brdf_lut(Handle shaderProgram, uint32_t brdfSize, int numSamples);

            void internal_renderTexture(std::vector<IRenderingPipeline::API2DCommand>&, Handle texture, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment, const glm::vec4& scissor);
            void internal_renderTexture(std::vector<IRenderingPipeline::API2DCommand>&, uint32_t textureAddress,int textureWidth, int textureHeight, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment, const glm::vec4& scissor);
            void internal_renderText(std::vector<IRenderingPipeline::API2DCommand>&, const std::string& t, Handle font, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, TextAlignment, const glm::vec4& scissor);
            void internal_renderBorder(std::vector<IRenderingPipeline::API2DCommand>&, float borderSize, const glm::vec2& pos, const glm::vec4& col, float w, float h, float angle, float depth, Alignment, const glm::vec4& scissor);
            void internal_renderRectangle(std::vector<IRenderingPipeline::API2DCommand>&, const glm::vec2& pos, const glm::vec4& col, float width, float height, float angle, float depth, Alignment, const glm::vec4& scissor);
            void internal_renderTriangle(std::vector<IRenderingPipeline::API2DCommand>&, const glm::vec2& position, const glm::vec4& color, float angle, float width, float height, float depth, Alignment, const glm::vec4& scissor);


            void internal_render2DAPI(uint32_t diffuseBuffer, const std::vector<IRenderingPipeline::API2DCommand>& commands, bool mainRenderFunc, Viewport&, bool clearDepth = true);

            DeferredPipeline() = delete;
        public:
            DeferredPipeline(Engine::priv::RenderModule&);
            ~DeferredPipeline();

            void init() override;
            void onPipelineChanged() override;
            void onFullscreen() override;
            void onResize(uint32_t newWidth, uint32_t newHeight) override;
            void onOpenGLContextCreation(uint32_t windowWidth, uint32_t windowHeight) override;
            void restoreDefaultState() override;
            void restoreCurrentState() override;
            void clear2DAPI() override;
            void sort2DAPI() override;

            void renderPhysicsAPI(bool mainRenderFunc, Viewport&, Camera&, Scene&) override;
            void render2DAPI(const std::vector<IRenderingPipeline::API2DCommand>& commands, bool mainRenderFunc, Viewport&, bool clearDepth = true) override;

            ShaderProgram* getCurrentBoundShaderProgram() override;
            Material* getCurrentBoundMaterial() override;
            Mesh* getCurrentBoundMesh() override;

            uint32_t getUniformLocation(const char* location) override;
            uint32_t getUniformLocationUnsafe(const char* location) override;
            uint32_t getCurrentBoundTextureOfType(uint32_t textureType) override;
            uint32_t getMaxNumTextureUnits() override;

            bool stencilOperation(uint32_t stencilFail, uint32_t depthFail, uint32_t depthPass) override;
            bool stencilMask(uint32_t mask) override;
            bool stencilFunction(uint32_t stencilFunction, uint32_t reference, uint32_t mask) override;
            bool cullFace(uint32_t face) override;
            bool setDepthFunction(uint32_t depthFunction) override;
            bool setViewport(float x, float y, float width, float height) override;
            void clear(bool color, bool depth, bool stencil) override;
            bool colorMask(bool r, bool g, bool b, bool alpha) override;
            bool clearColor(bool r, bool g, bool b, bool alpha) override;

            bool bindTextureForModification(TextureType, uint32_t textureObject) override;
            bool bindVAO(uint32_t vaoObject) override;
            bool deleteVAO(uint32_t& vaoObject) override;
            void generateAndBindTexture(TextureType, uint32_t& textureObject) override;
            void generateAndBindVAO(uint32_t& vaoObject) override;

            bool enableAPI(uint32_t apiEnum) override;
            bool disableAPI(uint32_t apiEnum) override;
            bool enableAPI_i(uint32_t apiEnum, uint32_t index) override;
            bool disableAPI_i(uint32_t apiEnum, uint32_t index) override;

            void clearTexture(int unit, uint32_t textureTarget) override;
            void sendTexture(const char* location, Texture&, int unit) override;
            void sendTexture(const char* location, TextureCubemap&, int unit) override;
            void sendTexture(const char* location, uint32_t textureObject, int unit, uint32_t textureTarget) override;
            void sendTextureSafe(const char* location, Texture&, int unit) override;
            void sendTextureSafe(const char* location, TextureCubemap&, int unit) override;
            void sendTextureSafe(const char* location, uint32_t textureObject, int unit, uint32_t textureTarget) override;

            void sendTextures(const char* location, const Texture**, int slot, const int arrSize) override;
            void sendTextures(const char* location, const TextureCubemap**, int slot, const int arrSize) override;
            void sendTextures(const char* location, const GLuint*, int slot, GLuint glTextureType, const int arrSize) override;
            void sendTexturesSafe(const char* location, const Texture**, int slot, const int arrSize) override;
            void sendTexturesSafe(const char* location, const TextureCubemap**, int slot, const int arrSize) override;
            void sendTexturesSafe(const char* location, const GLuint*, int slot, GLuint glTextureType, const int arrSize) override;

            bool bindReadFBO(uint32_t fbo) override;
            bool bindDrawFBO(uint32_t fbo) override;
            bool bindRBO(uint32_t rbo) override;

            bool bind(ModelInstance*) override;
            bool bind(ShaderProgram*) override;
            bool bind(Material*) override;
            bool bind(Mesh*) override;

            bool unbind(ModelInstance*) override;
            bool unbind(ShaderProgram*) override;
            bool unbind(Material*) override;
            bool unbind(Mesh*) override;

            void generatePBRData(TextureCubemap&, Handle convolutionTexture, Handle preEnvTexture, uint32_t convoludeSize, uint32_t prefilterSize) override;

            void toggleShadowCaster(SunLight&, bool) override;
            void toggleShadowCaster(PointLight&, bool) override;
            void toggleShadowCaster(DirectionalLight&, bool) override;
            void toggleShadowCaster(SpotLight&, bool) override;
            void toggleShadowCaster(RodLight&, bool) override;
            void toggleShadowCaster(ProjectionLight&, bool) override;

            bool buildShadowCaster(SunLight&) override;
            bool buildShadowCaster(PointLight&) override;
            bool buildShadowCaster(DirectionalLight&, uint32_t shadowMapWidth, uint32_t shadowMapSize, LightShadowFrustumType, float nearFactor, float farFactor) override;
            bool buildShadowCaster(SpotLight&) override;
            bool buildShadowCaster(RodLight&) override;
            bool buildShadowCaster(ProjectionLight&) override;
            void setShadowDirectionalLightDirection(DirectionalLight&, const glm::vec3& direction) override;

            void sendGPUDataAllLights(const Scene&, const Camera&) override;
            void sendGPUDataGI(Skybox*) override;
            void sendGPUDataLight(const Camera&, const SunLight&,         const std::string& start) override;
            int  sendGPUDataLight(const Camera&, const PointLight&,       const std::string& start) override;
            void sendGPUDataLight(const Camera&, const DirectionalLight&, const std::string& start) override;
            int  sendGPUDataLight(const Camera&, const SpotLight&,        const std::string& start) override;
            int  sendGPUDataLight(const Camera&, const RodLight&,         const std::string& start) override;
            int  sendGPUDataLight(const Camera&, const ProjectionLight&,  const std::string& start) override;

            void renderSkybox(Skybox*, Handle shaderProgram, Scene&, Viewport&, Camera&) override;
            void renderSunLight(Camera&, SunLight&, Viewport&) override;
            void renderPointLight(Camera&, PointLight&) override;
            void renderDirectionalLight(Camera&, DirectionalLight&, Viewport&) override;
            void renderSpotLight(Camera&, SpotLight&) override;
            void renderRodLight(Camera&, RodLight&) override;
            void renderProjectionLight(Camera&, ProjectionLight&) override;
            void renderMesh(Mesh&, uint32_t mode = (uint32_t)ModelDrawingMode::Triangles) override;
            void renderDecal(ModelInstance&) override;

            void renderParticles(ParticleSystem&, Camera&, Handle program) override;

            void renderLightProbe(LightProbe&) override;

            void render2DText(const std::string& text, Handle font, const glm::vec2& position, const glm::vec4& color, float angle, const glm::vec2& scale, float depth, TextAlignment, const glm::vec4& scissor = NO_SCISSOR) override;
            void render2DTexture(Handle texture, const glm::vec2& position, const glm::vec4& color, float angle, const glm::vec2& scale, float depth, Alignment, const glm::vec4& scissor = NO_SCISSOR) override;
            void render2DTexture(uint32_t textureAddress, int textureWidth, int textureHeight, const glm::vec2& position, const glm::vec4& color, float angle, const glm::vec2& scale, float depth, Alignment, const glm::vec4& scissor = NO_SCISSOR) override;
            void render2DTriangle(const glm::vec2& pos, const glm::vec4& color, float angle, float width, float height, float depth, Alignment, const glm::vec4& scissor = NO_SCISSOR) override;


            void renderTexture(Handle texture, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment, const glm::vec4& scissor) override;
            void renderTexture(uint32_t textureAddress, int textureWidth, int textureHeight, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment, const glm::vec4& scissor) override;
            void renderText(const std::string& t, Handle font, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, TextAlignment, const glm::vec4& scissor) override;
            void renderBorder(float borderSize, const glm::vec2& pos, const glm::vec4& col, float w, float h, float angle, float depth, Alignment, const glm::vec4& scissor) override;
            void renderRectangle(const glm::vec2& pos, const glm::vec4& col, float width, float height, float angle, float depth, Alignment, const glm::vec4& scissor) override;
            void renderTriangle(const glm::vec2& position, const glm::vec4& color, float angle, float width, float height, float depth, Alignment, const glm::vec4& scissor) override;


            void renderBackgroundTexture(Handle textureHandle, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment align, const glm::vec4& scissor) override;
            void renderBackgroundText(const std::string& t, Handle fontHandle, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, TextAlignment align, const glm::vec4& scissor) override;
            void renderBackgroundBorder(float borderSize, const glm::vec2& pos, const glm::vec4& col, float w, float h, float angle, float depth, Alignment align, const glm::vec4& scissor) override;
            void renderBackgroundRectangle(const glm::vec2& pos, const glm::vec4& col, float width, float height, float angle, float depth, Alignment align, const glm::vec4& scissor) override;
            void renderBackgroundTriangle(const glm::vec2& position, const glm::vec4& color, float angle, float width, float height, float depth, Alignment align, const glm::vec4& scissor) override;


            void renderFullscreenTriangle(float depth = 0.0f, float inNear = 0.0f, float inFar = 1.0f) override;
            void renderFullscreenQuad(float x, float y, float width, float height, float depth = 0.0f, float inNear = 0.0f, float inFar = 1.0f) override;

            void renderInitFrame(Engine::priv::RenderModule&) override;
            void render(Engine::priv::RenderModule&, Viewport&, bool mainRenderFunction) override;
            void render2DAPI(Engine::priv::RenderModule&, Viewport&, bool mainRenderFunction) override;
    };
};

#endif