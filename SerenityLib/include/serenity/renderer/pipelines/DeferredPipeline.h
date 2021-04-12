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
#include <serenity/model/ModelInstanceIncludes.h>
#include <serenity/renderer/opengl/State.h>
#include <serenity/renderer/opengl/Extensions.h>
#include <serenity/renderer/GBuffer.h>
#include <serenity/renderer/FullscreenItems.h>
#include <serenity/containers/PartialArray.h>
#include <serenity/resources/font/Font.h>

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
        };
        private:
            Engine::priv::RenderModule&                        m_Renderer;

            glm::vec4                                          m_CurrentScissorState = glm::vec4(-1.0f);
            float                                              m_CurrentScissorDepth = 0.0f;

            RendererState                                      m_RendererState;
            OpenGLState                                        m_OpenGLStateMachine;
            OpenGLExtensions                                   m_OpenGLExtensionsManager;

            FullscreenQuad                                     m_FullscreenQuad;
            FullscreenTriangle                                 m_FullscreenTriangle;

            Engine::partial_array<glm::vec3, Font::MAX_CHARACTERS_RENDERED_PER_FRAME * 4>    m_Text_Points; //4 points per char
            Engine::partial_array<glm::vec2, Font::MAX_CHARACTERS_RENDERED_PER_FRAME * 4>    m_Text_UVs;//4 uvs per char
            Engine::partial_array<uint32_t,  Font::MAX_CHARACTERS_RENDERED_PER_FRAME * 6>    m_Text_Indices;//6 ind per char

            UniformBufferObject*                               m_UBOCamera = nullptr;
            UBOCameraDataStruct                                m_UBOCameraDataStruct;
            glm::mat4                                          m_2DProjectionMatrix;
            GBuffer                                            m_GBuffer;

            std::vector<Handle>                                m_InternalShaders;
            std::vector<Handle>                                m_InternalShaderPrograms;

            std::vector<IRenderingPipeline::API2DCommand>      m_Background2DAPICommands;
            std::vector<IRenderingPipeline::API2DCommand>      m_2DAPICommands;
            //particle instancing
            uint32_t                                           m_Particle_Instance_VBO = 0U;

            void internal_gl_scissor_reset() noexcept;
            void internal_gl_scissor(const glm::vec4& scissor, float depth) noexcept;


            void internal_render_per_frame_preparation(Viewport& viewport, Camera& camera);
            void internal_init_frame_gbuffer(Viewport& viewport, Camera& camera);
            void internal_pass_geometry(Viewport& viewport, Camera& camera);
            void internal_pass_ssao(Viewport& viewport, Camera& camera);
            void internal_pass_stencil();
            void internal_pass_lighting(Viewport& viewport, Camera& camera, bool mainRenderFunction);
            void internal_pass_lighting_basic(Viewport& viewport, Camera& camera, bool mainRenderFunction);
            void internal_pass_forward(Viewport& viewport, Camera& camera, bool depthPrepass);
            void internal_pass_god_rays(Viewport& viewport, Camera& camera);
            void internal_pass_hdr(Viewport& viewport, Camera& camera, GBufferType::Type outTexture, GBufferType::Type outTexture2);
            void internal_pass_bloom(Viewport& viewport, GBufferType::Type sceneTexture);
            void internal_pass_depth_of_field(Viewport& viewport, GBufferType::Type& sceneTexture, GBufferType::Type& outTexture);
            void internal_pass_aa(bool mainRenderFunction, Viewport& viewport, Camera& camera, GBufferType::Type& sceneTexture, GBufferType::Type& outTexture);
            void internal_pass_final(GBufferType::Type sceneTexture);
            void internal_pass_depth_and_transparency(Viewport& viewport, GBufferType::Type sceneTexture); //TODO: recheck this
            bool internal_pass_depth_prepass(Viewport& viewport, Camera& camera);
            void internal_pass_blur(Viewport& viewport, GLuint texture, std::string_view type);
            void internal_pass_normaless_diffuse();

            void internal_generate_pbr_data_for_texture(Handle covoludeShaderProgram, Handle prefilterShaderProgram, Texture& texture, Handle convolutionTexture, Handle preEnvTexture, uint32_t convoludeTextureSize, uint32_t preEnvFilterSize);
            void internal_generate_brdf_lut(Handle shaderProgram, uint32_t brdfSize, int numSamples);

            void internal_render_2d_text_left(std::string_view text, const Font& font, float newLineGlyphHeight, float& x, float& y, float z);
            void internal_render_2d_text_center(std::string_view text, const Font& font, float newLineGlyphHeight, float& x, float& y, float z);
            void internal_render_2d_text_right(std::string_view text, const Font& font, float newLineGlyphHeight, float& x, float& y, float z);




            void internal_renderTexture(std::vector<IRenderingPipeline::API2DCommand>& commands, Handle texture, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment align, const glm::vec4& scissor);
            void internal_renderTexture(std::vector<IRenderingPipeline::API2DCommand>& commands, uint32_t textureAddress,int textureWidth, int textureHeight, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment align, const glm::vec4& scissor);
            void internal_renderText(std::vector<IRenderingPipeline::API2DCommand>& commands, const std::string& t, Handle font, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, TextAlignment align, const glm::vec4& scissor);
            void internal_renderBorder(std::vector<IRenderingPipeline::API2DCommand>& commands, float borderSize, const glm::vec2& pos, const glm::vec4& col, float w, float h, float angle, float depth, Alignment align, const glm::vec4& scissor);
            void internal_renderRectangle(std::vector<IRenderingPipeline::API2DCommand>& commands, const glm::vec2& pos, const glm::vec4& col, float width, float height, float angle, float depth, Alignment align, const glm::vec4& scissor);
            void internal_renderTriangle(std::vector<IRenderingPipeline::API2DCommand>& commands, const glm::vec2& position, const glm::vec4& color, float angle, float width, float height, float depth, Alignment align, const glm::vec4& scissor);




            DeferredPipeline() = delete;
        public:
            DeferredPipeline(Engine::priv::RenderModule& renderer);
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

            void renderPhysicsAPI(bool mainRenderFunc, Viewport& viewport, Camera& camera, Scene& scene) override;
            void render2DAPI(const std::vector<IRenderingPipeline::API2DCommand>& commands, bool mainRenderFunc, Viewport& viewport, bool clearDepth = true) override;

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

            bool bindTextureForModification(TextureType textureType, uint32_t textureObject) override;
            bool bindVAO(uint32_t vaoObject) override;
            bool deleteVAO(uint32_t& vaoObject) override;
            void generateAndBindTexture(TextureType textureType, uint32_t& textureObject) override;
            void generateAndBindVAO(uint32_t& vaoObject) override;

            bool enableAPI(uint32_t apiEnum) override;
            bool disableAPI(uint32_t apiEnum) override;
            bool enableAPI_i(uint32_t apiEnum, uint32_t index) override;
            bool disableAPI_i(uint32_t apiEnum, uint32_t index) override;

            void clearTexture(int unit, uint32_t textureTarget) override;
            void sendTexture(const char* location, Texture& texture, int unit) override;
            void sendTexture(const char* location, uint32_t textureObject, int unit, uint32_t textureTarget) override;
            void sendTextureSafe(const char* location, Texture& texture, int unit) override;
            void sendTextureSafe(const char* location, uint32_t textureObject, int unit, uint32_t textureTarget) override;

            bool bindReadFBO(uint32_t fbo) override;
            bool bindDrawFBO(uint32_t fbo) override;
            bool bindRBO(uint32_t rbo) override;

            bool bind(ModelInstance* modelInstance) override;
            bool bind(ShaderProgram* program) override;
            bool bind(Material* material) override;
            bool bind(Mesh* mesh) override;

            bool unbind(ModelInstance* modelInstance) override;
            bool unbind(ShaderProgram* program) override;
            bool unbind(Material* material) override;
            bool unbind(Mesh* mesh) override;

            void generatePBRData(Texture& texture, Handle convolutionTexture, Handle preEnvTexture, uint32_t convoludeSize, uint32_t prefilterSize) override;


            bool buildShadowCaster(SunLight&) override;
            bool buildShadowCaster(PointLight&) override;
            bool buildShadowCaster(DirectionalLight&) override;
            bool buildShadowCaster(SpotLight&) override;
            bool buildShadowCaster(RodLight&) override;
            bool buildShadowCaster(ProjectionLight&) override;

            void sendGPUDataAllLights(Scene&, Camera&) override;
            void sendGPUDataGI(Skybox*) override;
            void sendGPUDataLight(Camera&, SunLight&,         const std::string& start) override;
            int  sendGPUDataLight(Camera&, PointLight&,       const std::string& start) override;
            void sendGPUDataLight(Camera&, DirectionalLight&, const std::string& start) override;
            int  sendGPUDataLight(Camera&, SpotLight&,        const std::string& start) override;
            int  sendGPUDataLight(Camera&, RodLight&,         const std::string& start) override;
            int  sendGPUDataLight(Camera&, ProjectionLight&,  const std::string& start) override;

            void renderSkybox(Skybox*, Handle shaderProgram, Scene& scene, Viewport& viewport, Camera& camera) override;
            void renderSunLight(Camera& c, SunLight& s, Viewport& viewport) override;
            void renderPointLight(Camera& c, PointLight& p) override;
            void renderDirectionalLight(Camera& c, DirectionalLight& d, Viewport& viewport) override;
            void renderSpotLight(Camera& c, SpotLight& s) override;
            void renderRodLight(Camera& c, RodLight& r) override;
            void renderProjectionLight(Camera& c, ProjectionLight& r) override;
            void renderMesh(Mesh& mesh, uint32_t mode = (uint32_t)ModelDrawingMode::Triangles) override;
            void renderDecal(ModelInstance& decalModelInstance) override;

            void renderParticles(ParticleSystem& particleSystem, Camera& camera, Handle program) override;

            void renderLightProbe(LightProbe& lightProbe) override;

            void render2DText(const std::string& text, Handle font, const glm::vec2& position, const glm::vec4& color, float angle, const glm::vec2& scale, float depth, TextAlignment textAlignment, const glm::vec4& scissor = NO_SCISSOR) override;
            void render2DTexture(Handle texture, const glm::vec2& position, const glm::vec4& color, float angle, const glm::vec2& scale, float depth, Alignment align, const glm::vec4& scissor = NO_SCISSOR) override;
            void render2DTexture(uint32_t textureAddress, int textureWidth, int textureHeight, const glm::vec2& position, const glm::vec4& color, float angle, const glm::vec2& scale, float depth, Alignment align, const glm::vec4& scissor = NO_SCISSOR) override;
            void render2DTriangle(const glm::vec2& pos, const glm::vec4& color, float angle, float width, float height, float depth, Alignment align, const glm::vec4& scissor = NO_SCISSOR) override;


            void renderTexture(Handle texture, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment align, const glm::vec4& scissor) override;
            void renderTexture(uint32_t textureAddress, int textureWidth, int textureHeight, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment align, const glm::vec4& scissor) override;
            void renderText(const std::string& t, Handle font, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, TextAlignment align, const glm::vec4& scissor) override;
            void renderBorder(float borderSize, const glm::vec2& pos, const glm::vec4& col, float w, float h, float angle, float depth, Alignment align, const glm::vec4& scissor) override;
            void renderRectangle(const glm::vec2& pos, const glm::vec4& col, float width, float height, float angle, float depth, Alignment align, const glm::vec4& scissor) override;
            void renderTriangle(const glm::vec2& position, const glm::vec4& color, float angle, float width, float height, float depth, Alignment align, const glm::vec4& scissor) override;


            void renderBackgroundTexture(Handle texture, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment align, const glm::vec4& scissor) override;
            void renderBackgroundText(const std::string& t, Handle font, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, TextAlignment align, const glm::vec4& scissor) override;
            void renderBackgroundBorder(float borderSize, const glm::vec2& pos, const glm::vec4& col, float w, float h, float angle, float depth, Alignment align, const glm::vec4& scissor) override;
            void renderBackgroundRectangle(const glm::vec2& pos, const glm::vec4& col, float width, float height, float angle, float depth, Alignment align, const glm::vec4& scissor) override;
            void renderBackgroundTriangle(const glm::vec2& position, const glm::vec4& color, float angle, float width, float height, float depth, Alignment align, const glm::vec4& scissor) override;


            void renderFullscreenTriangle() override;
            void renderFullscreenQuad() override;

            void render(Engine::priv::RenderModule& renderer, Viewport& viewport, bool mainRenderFunction) override;
    };
};

#endif