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
    class Renderer;
};

#include <core/engine/renderer/pipelines/IRenderingPipeline.h>
#include <core/engine/model/ModelInstanceIncludes.h>
#include <core/engine/renderer/opengl/State.h>
#include <core/engine/renderer/opengl/Extensions.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/FullscreenItems.h>
#include <core/engine/containers/PartialArray.h>
#include <core/engine/fonts/Font.h>

namespace Engine::priv {
    class DeferredPipeline final : public IRenderingPipeline {
        friend class Renderer;
        public: struct UBOCameraDataStruct final {
            glm::mat4 View;
            glm::mat4 Proj;
            glm::mat4 ViewProj;
            glm::mat4 InvView;
            glm::mat4 InvProj;
            glm::mat4 InvViewProj;

            glm::vec4 Info1; //renderPosX, renderPosY, renderPosZ, near
            glm::vec4 Info2; //viewVecX, viewVecY, viewVecZ, far
            glm::vec4 Info3; //realposX, realposY, realposZ, UNUSED

            glm::vec4 Info4; //mainWindowSizeX, mainWindowSizeY, viewportSizeX, viewportSizeY
        };

        private:
            struct API2DCommand final {
                std::function<void()>  func;
                float                  depth;

                API2DCommand() = default;
                API2DCommand(std::function<void()>&& func_, float depth_) 
                    : func{ std::move(func_ ) }
                    , depth { depth_ }
                {}
            };
            Engine::priv::Renderer&        m_Renderer;

            glm::vec4                      m_CurrentScissorState = glm::vec4(-1.0f);
            float                          m_CurrentScissorDepth = 0.0f;

            RendererState                  m_RendererState;
            OpenGLState                    m_OpenGLStateMachine;
            OpenGLExtensions               m_OpenGLExtensionsManager;

            FullscreenQuad                 m_FullscreenQuad;
            FullscreenTriangle             m_FullscreenTriangle;

            Engine::partial_array<glm::vec3, Font::MAX_CHARACTERS_RENDERED_PER_FRAME * 4>    m_Text_Points; //4 points per char
            Engine::partial_array<glm::vec2, Font::MAX_CHARACTERS_RENDERED_PER_FRAME * 4>    m_Text_UVs;//4 uvs per char
            Engine::partial_array<unsigned int, Font::MAX_CHARACTERS_RENDERED_PER_FRAME * 6> m_Text_Indices;//6 ind per char

            UBOCameraDataStruct            m_UBOCameraDataStruct;
            glm::mat4                      m_2DProjectionMatrix;
            GBuffer                        m_GBuffer;

            std::vector<Handle>            m_InternalShaders;
            std::vector<Handle>            m_InternalShaderPrograms;
            std::vector<API2DCommand>      m_2DAPICommands;
            std::vector<API2DCommand>      m_2DAPICommandsNonTextured;
            //particle instancing
            unsigned int                   m_Particle_Instance_VBO = 0U;

            void internal_gl_scissor_reset() noexcept;
            void internal_gl_scissor(const glm::vec4& scissor, float depth) noexcept;


            void internal_render_per_frame_preparation(Viewport& viewport, Camera& camera);
            void internal_pass_geometry(Viewport& viewport, Camera& camera);
            void internal_pass_ssao(Viewport& viewport, Camera& camera);
            void internal_pass_stencil();
            void internal_pass_lighting(Viewport& viewport, Camera& camera, bool mainRenderFunction);
            void internal_pass_forward(Viewport& viewport, Camera& camera, bool depthPrepass);
            void internal_pass_god_rays(Viewport& viewport, Camera& camera);
            void internal_pass_hdr(Viewport& viewport, Camera& camera);
            void internal_pass_bloom(Viewport& viewport);
            void internal_pass_depth_of_field(Viewport& viewport, GBufferType::Type& sceneTexture, GBufferType::Type& outTexture);
            void internal_pass_aa(bool mainRenderFunction, Viewport& viewport, Camera& camera, GBufferType::Type& sceneTexture, GBufferType::Type& outTexture);
            void internal_pass_final(GBufferType::Type sceneTexture);
            void internal_pass_depth_and_transparency(Viewport& viewport, GBufferType::Type sceneTexture); //TODO: recheck this
            bool internal_pass_depth_prepass(Viewport& viewport, Camera& camera);
            void internal_pass_blur(Viewport& viewport, GLuint texture, std::string_view type);

            void internal_generate_pbr_data_for_texture(Handle covoludeShaderProgram, Handle prefilterShaderProgram, Texture& texture, Handle convolutionTexture, Handle preEnvTexture, unsigned int convoludeTextureSize, unsigned int preEnvFilterSize);
            void internal_generate_brdf_lut(Handle shaderProgram, unsigned int brdfSize, int numSamples);

            void internal_render_2d_text_left(const std::string& text, const Font& font, float newLineGlyphHeight, float& x, float& y, float z);
            void internal_render_2d_text_center(const std::string& text, const Font& font, float newLineGlyphHeight, float& x, float& y, float z);
            void internal_render_2d_text_right(const std::string& text, const Font& font, float newLineGlyphHeight, float& x, float& y, float z);

            DeferredPipeline() = delete;
        public:
            DeferredPipeline(Engine::priv::Renderer& renderer);
            ~DeferredPipeline();

            void init() override;
            void onPipelineChanged() override;
            void onFullscreen() override;
            void onResize(unsigned int newWidth, unsigned int newHeight) override;
            void onOpenGLContextCreation(unsigned int windowWidth, unsigned int windowHeight, unsigned int glslVersion, unsigned int openglVersion) override;
            void restoreDefaultState() override;
            void restoreCurrentState() override;
            void clear2DAPI() override;
            void sort2DAPI() override;

            void renderPhysicsAPI(bool mainRenderFunc, Viewport& viewport, Camera& camera, Scene& scene) override;

            //non textured 2d api elements will be exposed to anti-aliasing post processing
            void render2DAPINonTextured(bool mainRenderFunc, Viewport& viewport) override;

            void render2DAPI(bool mainRenderFunc, Viewport& viewport) override;


            ShaderProgram* getCurrentBoundShaderProgram() override;
            Material* getCurrentBoundMaterial() override;
            Mesh* getCurrentBoundMesh() override;

            unsigned int getUniformLocation(const char* location) override;
            unsigned int getUniformLocationUnsafe(const char* location) override;
            unsigned int getCurrentBoundTextureOfType(unsigned int textureType) override;
            unsigned int getMaxNumTextureUnits() override;

            bool stencilOperation(unsigned int stencilFail, unsigned int depthFail, unsigned int depthPass) override;
            bool stencilMask(unsigned int mask) override;
            bool stencilFunction(unsigned int stencilFunction, unsigned int reference, unsigned int mask) override;
            bool cullFace(unsigned int face) override;
            bool setDepthFunction(unsigned int depthFunction) override;
            bool setViewport(float x, float y, float width, float height) override;
            void clear(bool color, bool depth, bool stencil) override;
            bool colorMask(bool r, bool g, bool b, bool alpha) override;
            bool clearColor(bool r, bool g, bool b, bool alpha) override;

            bool bindTextureForModification(TextureType textureType, unsigned int textureObject) override;
            bool bindVAO(unsigned int vaoObject) override;
            bool deleteVAO(unsigned int& vaoObject) override;
            void generateAndBindTexture(TextureType textureType, unsigned int& textureObject) override;
            void generateAndBindVAO(unsigned int& vaoObject) override;

            bool enableAPI(unsigned int apiEnum) override;
            bool disableAPI(unsigned int apiEnum) override;
            bool enableAPI_i(unsigned int apiEnum, unsigned int index) override;
            bool disableAPI_i(unsigned int apiEnum, unsigned int index) override;

            void sendTexture(const char* location, Texture& texture, int slot) override;
            void sendTexture(const char* location, unsigned int textureObject, int slot, unsigned int textureTarget) override;
            void sendTextureSafe(const char* location, Texture& texture, int slot) override;
            void sendTextureSafe(const char* location, unsigned int textureObject, int slot, unsigned int textureTarget) override;

            bool bindReadFBO(unsigned int fbo) override;
            bool bindDrawFBO(unsigned int fbo) override;
            bool bindRBO(unsigned int rbo) override;

            bool bind(ModelInstance* modelInstance) override;
            bool bind(ShaderProgram* program) override;
            bool bind(Material* material) override;
            bool bind(Mesh* mesh) override;

            bool unbind(ModelInstance* modelInstance) override;
            bool unbind(ShaderProgram* program) override;
            bool unbind(Material* material) override;
            bool unbind(Mesh* mesh) override;

            void generatePBRData(Texture& texture, Handle convolutionTexture, Handle preEnvTexture, unsigned int convoludeSize, unsigned int prefilterSize) override;

            void sendGPUDataSunLight(Camera& camera, SunLight& sunLight, const std::string& start) override;
            int sendGPUDataPointLight(Camera& camera, PointLight& pointLight, const std::string& start) override;
            void sendGPUDataDirectionalLight(Camera& camera, DirectionalLight& directionalLight, const std::string& start) override;
            int sendGPUDataSpotLight(Camera& camera, SpotLight& spotLight, const std::string& start) override;
            int sendGPUDataRodLight(Camera& camera, RodLight& rodLight, const std::string& start) override;
            int sendGPUDataProjectionLight(Camera& camera, ProjectionLight& projectionLight, const std::string& start) override;

            void renderSkybox(Skybox*, Handle shaderProgram, Scene& scene, Viewport& viewport, Camera& camera) override;
            void renderSunLight(Camera& c, SunLight& s, Viewport& viewport) override;
            void renderPointLight(Camera& c, PointLight& p) override;
            void renderDirectionalLight(Camera& c, DirectionalLight& d, Viewport& viewport) override;
            void renderSpotLight(Camera& c, SpotLight& s) override;
            void renderRodLight(Camera& c, RodLight& r) override;
            void renderProjectionLight(Camera& c, ProjectionLight& r) override;
            void renderMesh(Mesh& mesh, unsigned int mode = (unsigned int)ModelDrawingMode::Triangles) override;
            void renderDecal(ModelInstance& decalModelInstance) override;

            void renderParticles(ParticleSystem& particleSystem, Camera& camera, Handle program) override;

            void renderLightProbe(LightProbe& lightProbe) override;

            void render2DText(const std::string& text, Handle font, const glm::vec2& position, const glm::vec4& color, float angle, const glm::vec2& scale, float depth, TextAlignment textAlignment, const glm::vec4& scissor = NO_SCISSOR) override;
            void render2DTexture(Handle texture, const glm::vec2& position, const glm::vec4& color, float angle, const glm::vec2& scale, float depth, Alignment align, const glm::vec4& scissor = NO_SCISSOR) override;
            void render2DTriangle(const glm::vec2& pos, const glm::vec4& color, float angle, float width, float height, float depth, Alignment align, const glm::vec4& scissor = NO_SCISSOR) override;


            void renderTexture(Handle texture, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment align, const glm::vec4& scissor) override;
            void renderText(const std::string& t, Handle font, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, TextAlignment align, const glm::vec4& scissor) override;
            void renderBorder(float borderSize, const glm::vec2& pos, const glm::vec4& col, float w, float h, float angle, float depth, Alignment align, const glm::vec4& scissor) override;
            void renderRectangle(const glm::vec2& pos, const glm::vec4& col, float width, float height, float angle, float depth, Alignment align, const glm::vec4& scissor) override;
            void renderTriangle(const glm::vec2& position, const glm::vec4& color, float angle, float width, float height, float depth, Alignment align, const glm::vec4& scissor) override;

            void renderFullscreenTriangle() override;
            void renderFullscreenQuad() override;

            void render(Engine::priv::Renderer& renderer, Viewport& viewport, bool mainRenderFunction) override;
    };
};

#endif