#pragma once
#ifndef ENGINE_RENDERER_RENDERING_PIPELINE_H
#define ENGINE_RENDERER_RENDERING_PIPELINE_H

class Mesh;
class Material;
class ShaderProgram;
class Scene;
class Viewport;
class Camera;
class SunLight;
class PointLight;
class DirectionalLight;
class SpotLight;
class RodLight;
class ProjectionLight;
class Skybox;
class Font;
class Texture;
class Particle;
class ModelInstance;
class LightProbe;
namespace Engine::priv {
    class RenderModule;
    class ParticleSystem;
};

#include <core/engine/fonts/FontIncludes.h>
#include <core/engine/renderer/RendererIncludes.h>
#include <core/engine/resources/Handle.h>
#include <core/engine/textures/TextureIncludes.h>

namespace Engine::priv {
    class IRenderingPipeline {
        friend class RenderModule;
        public:
            struct API2DCommand final {
                std::function<void()>  func;
                float                  depth;

                API2DCommand() = default;
                API2DCommand(std::function<void()>&& func_, float depth_)
                    : func{ std::move(func_) }
                    , depth{ depth_ }
                {}
            };

            virtual ~IRenderingPipeline(){}

            virtual void init() = 0;
            virtual void onPipelineChanged() = 0;
            virtual void onFullscreen() = 0;
            virtual void onResize(uint32_t newWidth, uint32_t newHeight) = 0;
            virtual void onOpenGLContextCreation(uint32_t windowWidth, uint32_t windowHeight, uint32_t glslVersion, uint32_t openglVersion) = 0;
            virtual void restoreDefaultState() = 0;
            virtual void restoreCurrentState() = 0;
            virtual void clear2DAPI() = 0;
            virtual void sort2DAPI() = 0;

            virtual void renderPhysicsAPI(bool mainRenderFunc, Viewport& viewport, Camera& camera, Scene& scene) = 0;
            virtual void render2DAPI(const std::vector<IRenderingPipeline::API2DCommand>& commands, bool mainRenderFunc, Viewport& viewport, bool clearDepth = true) = 0;


            virtual ShaderProgram* getCurrentBoundShaderProgram() = 0;
            virtual Material* getCurrentBoundMaterial() = 0;
            virtual Mesh* getCurrentBoundMesh() = 0;


            virtual uint32_t getUniformLocation(const char* location) = 0;
            virtual uint32_t getUniformLocationUnsafe(const char* location) = 0;
            virtual uint32_t getCurrentBoundTextureOfType(uint32_t textureType) = 0;
            virtual uint32_t getMaxNumTextureUnits() = 0;

            virtual bool stencilOperation(uint32_t stencilFail, uint32_t depthFail, uint32_t depthPass) = 0;
            virtual bool stencilMask(uint32_t mask) = 0;
            virtual bool stencilFunction(uint32_t stencilFunction, uint32_t reference, uint32_t mask) = 0;
            virtual bool cullFace(uint32_t face) = 0;
            virtual bool setDepthFunction(uint32_t depthFunction) = 0;
            virtual bool setViewport(float x, float y, float width, float height) = 0;
            virtual void clear(bool color, bool depth, bool stencil) = 0;
            virtual bool colorMask(bool r, bool g, bool b, bool alpha) = 0;
            virtual bool clearColor(bool r, bool g, bool b, bool alpha) = 0;

            virtual bool bindTextureForModification(TextureType textureType, uint32_t textureObject) = 0;
            virtual bool bindVAO(uint32_t vaoObject) = 0;
            virtual bool deleteVAO(uint32_t& vaoObject) = 0;
            virtual void generateAndBindTexture(TextureType textureType, uint32_t& textureObject) = 0;
            virtual void generateAndBindVAO(uint32_t& vaoObject) = 0;

            virtual bool enableAPI(uint32_t apiEnum) = 0;
            virtual bool disableAPI(uint32_t apiEnum) = 0;
            virtual bool enableAPI_i(uint32_t apiEnum, uint32_t index) = 0;
            virtual bool disableAPI_i(uint32_t apiEnum, uint32_t index) = 0;

            virtual void clearTexture(int unit, uint32_t textureTarget) = 0;
            virtual void sendTexture(const char* location, Texture& texture, int unit) = 0;
            virtual void sendTexture(const char* location, uint32_t textureObject, int unit, uint32_t textureTarget) = 0;
            virtual void sendTextureSafe(const char* location, Texture& texture, int unit) = 0;
            virtual void sendTextureSafe(const char* location, uint32_t textureObject, int unit, uint32_t textureTarget) = 0;

            virtual bool bindReadFBO(uint32_t fbo) = 0;
            virtual bool bindDrawFBO(uint32_t fbo) = 0;
            virtual bool bindRBO(uint32_t rbo) = 0;

            virtual bool bind(ModelInstance* modelInstance) = 0;
            virtual bool bind(ShaderProgram* program) = 0;
            virtual bool bind(Material* material) = 0;
            virtual bool bind(Mesh* mesh) = 0;

            virtual bool unbind(ModelInstance* modelInstance) = 0;
            virtual bool unbind(ShaderProgram* program) = 0;
            virtual bool unbind(Material* material) = 0;
            virtual bool unbind(Mesh* mesh) = 0;
            
            virtual void generatePBRData(Texture& texture, Handle convolutionTexture, Handle preEnvTexture, uint32_t convoludeSize, uint32_t prefilterSize) = 0;

            virtual void sendGPUDataLight(Camera& camera, SunLight& sunLight, const std::string& start) = 0;
            virtual int sendGPUDataLight(Camera& camera, PointLight& pointLight, const std::string& start) = 0;
            virtual void sendGPUDataLight(Camera& camera, DirectionalLight& directionalLight, const std::string& start) = 0;
            virtual int sendGPUDataLight(Camera& camera, SpotLight& spotLight, const std::string& start) = 0;
            virtual int sendGPUDataLight(Camera& camera, RodLight& rodLight, const std::string& start) = 0;
            virtual int sendGPUDataLight(Camera& camera, ProjectionLight& projectionLight, const std::string& start) = 0;

            virtual void renderSunLight(Camera& camera, SunLight& sunLight, Viewport& viewport) = 0;
            virtual void renderPointLight(Camera& camera, PointLight& pointLight) = 0;
            virtual void renderDirectionalLight(Camera& camera, DirectionalLight& directionalLight, Viewport& viewport) = 0;
            virtual void renderSpotLight(Camera& camera, SpotLight& spotLight) = 0;
            virtual void renderRodLight(Camera& camera, RodLight& rodLight) = 0;
            virtual void renderProjectionLight(Camera& camera, ProjectionLight& rodLight) = 0;

            virtual void renderSkybox(Skybox*, Handle shaderProgram, Scene& scene, Viewport& viewport, Camera& camera) = 0;
            virtual void renderMesh(Mesh& mesh, uint32_t mode) = 0;
            virtual void renderDecal(ModelInstance& decalModelInstance) = 0;
            virtual void renderLightProbe(LightProbe& lightProbe) = 0;

            virtual void renderParticles(ParticleSystem& particleSystem, Camera& camera, Handle program) = 0;

            virtual void render2DText(
                const std::string& text, 
                Handle font,
                const glm::vec2& position, 
                const glm::vec4& color, 
                float angle, 
                const glm::vec2& scale, 
                float depth, 
                TextAlignment textAlignment, 
                const glm::vec4& scissor = NO_SCISSOR
            ) = 0;
            virtual void render2DTexture(
                Handle texture,
                const glm::vec2& position, 
                const glm::vec4& color, 
                float angle, 
                const glm::vec2& scale, 
                float depth, 
                Alignment align, 
                const glm::vec4& scissor = NO_SCISSOR
            ) = 0;
            virtual void render2DTriangle(
                const glm::vec2& pos, 
                const glm::vec4& color, 
                float angle, 
                float width, 
                float height, 
                float depth, 
                Alignment align, 
                const glm::vec4& scissor = NO_SCISSOR
            ) = 0;


            virtual void renderTexture(
                Handle texture,
                const glm::vec2& p, 
                const glm::vec4& c, 
                float a, 
                const glm::vec2& s, 
                float d, 
                Alignment align, 
                const glm::vec4& scissor
            ) = 0;
            virtual void renderText(
                const std::string& t, 
                Handle font,
                const glm::vec2& p, 
                const glm::vec4& c, 
                float a, 
                const glm::vec2& s, 
                float d, 
                TextAlignment align, 
                const glm::vec4& scissor
            ) = 0;
            virtual void renderBorder(
                float borderSize, 
                const glm::vec2& pos, 
                const glm::vec4& col, 
                float w, 
                float h, 
                float angle, 
                float depth, 
                Alignment align, 
                const glm::vec4& scissor
            ) = 0;
            virtual void renderRectangle(
                const glm::vec2& pos, 
                const glm::vec4& col, 
                float width, 
                float height, 
                float angle, 
                float depth, 
                Alignment align, 
                const glm::vec4& scissor
            ) = 0;
            virtual void renderTriangle(
                const glm::vec2& position, 
                const glm::vec4& color, 
                float angle, 
                float width, 
                float height, 
                float depth, 
                Alignment align, 
                const glm::vec4& scissor
            ) = 0;





            virtual void renderBackgroundTexture(
                Handle texture,
                const glm::vec2& p,
                const glm::vec4& c,
                float a,
                const glm::vec2& s,
                float d,
                Alignment align,
                const glm::vec4& scissor
            ) = 0;
            virtual void renderBackgroundText(
                const std::string& t,
                Handle font,
                const glm::vec2& p,
                const glm::vec4& c,
                float a,
                const glm::vec2& s,
                float d,
                TextAlignment align,
                const glm::vec4& scissor
            ) = 0;
            virtual void renderBackgroundBorder(
                float borderSize,
                const glm::vec2& pos,
                const glm::vec4& col,
                float w,
                float h,
                float angle,
                float depth,
                Alignment align,
                const glm::vec4& scissor
            ) = 0;
            virtual void renderBackgroundRectangle(
                const glm::vec2& pos,
                const glm::vec4& col,
                float width,
                float height,
                float angle,
                float depth,
                Alignment align,
                const glm::vec4& scissor
            ) = 0;
            virtual void renderBackgroundTriangle(
                const glm::vec2& position,
                const glm::vec4& color,
                float angle,
                float width,
                float height,
                float depth,
                Alignment align,
                const glm::vec4& scissor
            ) = 0;

            virtual void renderFullscreenTriangle() = 0;
            virtual void renderFullscreenQuad() = 0;

            virtual void render(Engine::priv::RenderModule& renderer, Viewport& viewport, bool mainRenderFunction) = 0;
    };
};

#endif