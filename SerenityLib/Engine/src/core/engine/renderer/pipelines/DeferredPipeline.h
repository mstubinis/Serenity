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

#include <glm/mat4x4.hpp>
#include <vector>

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
            struct API2DCommand {
                std::function<void()>  func;
                float                  depth;
                glm::vec4              scissor;
            };
            Engine::priv::Renderer&        m_Renderer;

            RendererState                  m_RendererState;
            OpenGLState                    m_OpenGLStateMachine;
            OpenGLExtensions               m_OpenGLExtensionsManager;

            FullscreenQuad                 m_FullscreenQuad;
            FullscreenTriangle             m_FullscreenTriangle;

            std::vector<glm::vec3>         m_Text_Points;
            std::vector<glm::vec2>         m_Text_UVs;
            std::vector<unsigned int>      m_Text_Indices;

            UBOCameraDataStruct            m_UBOCameraDataStruct;
            glm::mat4                      m_2DProjectionMatrix;
            GBuffer                        m_GBuffer;

            std::vector<Shader*>           m_InternalShaders;
            std::vector<ShaderProgram*>    m_InternalShaderPrograms;
            std::vector<API2DCommand>      m_2DAPICommands;
            std::vector<API2DCommand>      m_2DAPICommandsNonTextured;
            //particle instancing
            unsigned int                   m_Particle_Instance_VBO;

            void internal_render_per_frame_preparation(const Viewport& viewport, const Camera& camera);
            void internal_pass_geometry(const Viewport& viewport, const Camera& camera);
            void internal_pass_ssao(const Viewport& viewport, const Camera& camera);
            void internal_pass_stencil();
            void internal_pass_lighting(const Viewport& viewport, const Camera& camera, const bool mainRenderFunction);
            void internal_pass_forward(const Viewport& viewport, const Camera& camera);
            void internal_pass_god_rays(const Viewport& viewport, const Camera& camera);
            void internal_pass_hdr(const Viewport& viewport, const Camera& camera);
            void internal_pass_bloom(const Viewport& viewport);
            void internal_pass_depth_of_field(const Viewport& viewport, GBufferType::Type& sceneTexture, GBufferType::Type& outTexture);
            void internal_pass_aa(const bool mainRenderFunction, const Viewport& viewport, const Camera& camera, GBufferType::Type& sceneTexture, GBufferType::Type& outTexture);
            void internal_pass_final(const GBufferType::Type& sceneTexture);
            void internal_pass_depth_and_transparency(const Viewport& viewport, const GBufferType::Type& sceneTexture); //TODO: recheck this
            void internal_pass_copy_depth();
            void internal_pass_blur(const Viewport& viewport, const GLuint texture, std::string_view type);

            void internal_generate_pbr_data_for_texture(ShaderProgram& covoludeShaderProgram, ShaderProgram& prefilterShaderProgram, Texture& texture, const unsigned int convoludeTextureSize, const unsigned int preEnvFilterSize);
            void internal_generate_brdf_lut(ShaderProgram& program, const unsigned int brdfSize, const int numSamples);

            void internal_render_2d_text_left(const std::string& text, const Font& font, const float newLineGlyphHeight, float& x, float& y, const float z);
            void internal_render_2d_text_center(const std::string& text, const Font& font, const float newLineGlyphHeight, float& x, float& y, const float z);
            void internal_render_2d_text_right(const std::string& text, const Font& font, const float newLineGlyphHeight, float& x, float& y, const float z);

            DeferredPipeline() = delete;
        public:
            DeferredPipeline(Engine::priv::Renderer& renderer);
            ~DeferredPipeline();

            void init() override;
            void onPipelineChanged() override;
            void onFullscreen() override;
            void onResize(const unsigned int newWidth, const unsigned int newHeight) override;
            void onOpenGLContextCreation(const unsigned int windowWidth, const unsigned int windowHeight, const unsigned int glslVersion, const unsigned int openglVersion) override;
            void restoreDefaultState() override;
            void restoreCurrentState() override;
            void clear2DAPI() override;
            void sort2DAPI() override;

            void renderPhysicsAPI(const bool mainRenderFunc, const Viewport& viewport, const Camera& camera, const Scene& scene) override;

            //non textured 2d api elements will be exposed to anti-aliasing post processing
            void render2DAPINonTextured(const bool mainRenderFunc, const Viewport& viewport) override;

            void render2DAPI(const bool mainRenderFunc, const Viewport& viewport) override;


            ShaderProgram* getCurrentBoundShaderProgram() override;
            Material* getCurrentBoundMaterial() override;
            Mesh* getCurrentBoundMesh() override;

            unsigned int getUniformLocation(const char* location) override;
            unsigned int getUniformLocationUnsafe(const char* location) override;

            unsigned int getMaxNumTextureUnits() override;

            bool stencilOperation(const unsigned int stencilFail, const unsigned int depthFail, const unsigned int depthPass) override;
            bool stencilMask(const unsigned int mask) override;
            bool stencilFunction(const unsigned int stencilFunction, const unsigned int reference, const unsigned int mask) override;
            bool cullFace(const unsigned int face) override;
            bool setDepthFunction(const unsigned int depthFunction) override;
            bool setViewport(const float x, const float y, const float width, const float height) override;
            void clear(const bool color, const bool depth, const bool stencil) override;
            bool colorMask(const bool r, const bool g, const bool b, const bool alpha) override;
            bool clearColor(const bool r, const bool g, const bool b, const bool alpha) override;

            bool bindTextureForModification(const unsigned int textureType, const unsigned int textureObject) override;
            bool bindVAO(const unsigned int vaoObject) override;
            bool deleteVAO(unsigned int& vaoObject) override;
            void generateAndBindTexture(const unsigned int textureType, unsigned int& textureObject) override;
            void generateAndBindVAO(unsigned int& vaoObject) override;

            bool enableAPI(const unsigned int apiEnum) override;
            bool disableAPI(const unsigned int apiEnum) override;
            bool enableAPI_i(const unsigned int apiEnum, const unsigned int index) override;
            bool disableAPI_i(const unsigned int apiEnum, const unsigned int index) override;

            void sendTexture(const char* location, const Texture& texture, const int slot) override;
            void sendTexture(const char* location, const unsigned int textureObject, const int slot, const unsigned int textureTarget) override;
            void sendTextureSafe(const char* location, const Texture& texture, const int slot) override;
            void sendTextureSafe(const char* location, const unsigned int textureObject, const int slot, const unsigned int textureTarget) override;

            bool bindReadFBO(const unsigned int fbo) override;
            bool bindDrawFBO(const unsigned int fbo) override;
            bool bindRBO(const unsigned int rbo) override;

            bool bind(ModelInstance* modelInstance) override;
            bool bind(ShaderProgram* program) override;
            bool bind(Material* material) override;
            bool bind(Mesh* mesh) override;

            bool unbind(ModelInstance* modelInstance) override;
            bool unbind(ShaderProgram* program) override;
            bool unbind(Material* material) override;
            bool unbind(Mesh* mesh) override;

            void generatePBRData(Texture& texture, const unsigned int convoludeSize, const unsigned int prefilterSize) override;

            void renderSkybox(Skybox*, ShaderProgram& shaderProgram, const Scene& scene, const Viewport& viewport, const Camera& camera) override;
            void renderSunLight(const Camera& c, const SunLight& s, const Viewport& viewport) override;
            void renderPointLight(const Camera& c, const PointLight& p) override;
            void renderDirectionalLight(const Camera& c, const DirectionalLight& d, const Viewport& viewport) override;
            void renderSpotLight(const Camera& c, const SpotLight& s) override;
            void renderRodLight(const Camera& c, const RodLight& r) override;
            void renderMesh(const Mesh& mesh, const unsigned int mode = ModelDrawingMode::Triangles) override;
            void renderDecal(ModelInstance& decalModelInstance) override;

            void renderParticles(ParticleSystem& particleSystem, const Camera& camera, ShaderProgram& program) override;

            void renderLightProbe(LightProbe& lightProbe) override;

            void render2DText(const std::string& text, const Font& font, const glm::vec2& position, const glm::vec4& color, const float angle, const glm::vec2& scale, const float depth, const TextAlignment::Type textAlignment, const glm::vec4& scissor = glm::vec4(-1.0f)) override;
            void render2DTexture(const Texture* texture, const glm::vec2& position, const glm::vec4& color, const float angle, const glm::vec2& scale, const float depth, const Alignment::Type align, const glm::vec4& scissor = glm::vec4(-1.0f)) override;
            void render2DTriangle(const glm::vec2& pos, const glm::vec4& color, const float angle, const float width, const float height, const float depth, const Alignment::Type align, const glm::vec4& scissor = glm::vec4(-1.0f)) override;


            void renderTexture(const Texture& tex, const glm::vec2& p, const glm::vec4& c, const float a, const glm::vec2& s, const float d, const Alignment::Type align, const glm::vec4& scissor) override;
            void renderText(const std::string& t, const Font& fnt, const glm::vec2& p, const glm::vec4& c, const float a, const glm::vec2& s, const float d, const TextAlignment::Type align, const glm::vec4& scissor) override;
            void renderBorder(const float borderSize, const glm::vec2& pos, const glm::vec4& col, const float w, const float h, const float angle, const float depth, const Alignment::Type align, const glm::vec4& scissor) override;
            void renderRectangle(const glm::vec2& pos, const glm::vec4& col, const float width, const float height, const float angle, const float depth, const Alignment::Type align, const glm::vec4& scissor) override;
            void renderTriangle(const glm::vec2& position, const glm::vec4& color, const float angle, const float width, const float height, const float depth, const Alignment::Type align, const glm::vec4& scissor) override;

            void renderFullscreenTriangle() override;
            void renderFullscreenQuad() override;

            void update(const float dt) override;
            void render(Engine::priv::Renderer& renderer, const Viewport& viewport, const bool mainRenderFunction) override;
    };
};

#endif