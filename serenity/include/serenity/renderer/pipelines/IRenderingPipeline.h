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
class TextureCubemap;
class Particle;
class ModelInstance;
class LightProbe;
namespace Engine::priv {
    class RenderModule;
    class ParticleSystem;
};

#include <serenity/resources/font/FontIncludes.h>
#include <serenity/lights/LightIncludes.h>
#include <serenity/renderer/RendererIncludes.h>
#include <serenity/resources/Handle.h>
#include <serenity/resources/texture/TextureIncludes.h>
#include <functional>

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

            virtual void postConstructor(uint32_t width, uint32_t height) = 0;
            virtual void init(uint32_t width, uint32_t height) = 0;
            virtual void onPipelineChanged() = 0;
            virtual void onResize(uint32_t newWidth, uint32_t newHeight) = 0;
            //virtual void onOpenGLContextCreation(uint32_t windowWidth, uint32_t windowHeight) = 0;
            //virtual void restoreDefaultState() = 0;
            virtual void restoreCurrentState() = 0;
            virtual void clear2DAPI() = 0;
            virtual void sort2DAPI() = 0;

            virtual void renderPhysicsAPI(bool mainRenderFunc, Viewport&, Camera&, Scene&) = 0;
            virtual void render2DAPI(const std::vector<IRenderingPipeline::API2DCommand>& commands, bool mainRenderFunc, Viewport&, bool clearDepth = true) = 0;


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

            virtual bool bindTextureForModification(TextureType, uint32_t textureObject) = 0;
            virtual bool bindVAO(uint32_t vaoObject) = 0;
            virtual void generateAndBindTexture(TextureType, uint32_t& textureObject) = 0;
            virtual void generateVAO(uint32_t& vaoObject) = 0;

            virtual bool enableAPI(uint32_t apiEnum) = 0;
            virtual bool disableAPI(uint32_t apiEnum) = 0;
            virtual bool enableAPI_i(uint32_t apiEnum, uint32_t index) = 0;
            virtual bool disableAPI_i(uint32_t apiEnum, uint32_t index) = 0;

            virtual void clearTexture(int unit, uint32_t textureTarget) = 0;
            virtual void sendTexture(const char* location, Texture&, int unit) = 0;
            virtual void sendTexture(const char* location, TextureCubemap&, int unit) = 0;
            virtual void sendTexture(const char* location, uint32_t textureObject, int unit, uint32_t textureTarget) = 0;
            virtual void sendTextureSafe(const char* location, Texture&, int unit) = 0;
            virtual void sendTextureSafe(const char* location, TextureCubemap&, int unit) = 0;
            virtual void sendTextureSafe(const char* location, uint32_t textureObject, int unit, uint32_t textureTarget) = 0;

            virtual void sendTextures(const char* location, const Texture**, int slot, const int arrSize) = 0;
            virtual void sendTextures(const char* location, const TextureCubemap**, int slot, const int arrSize) = 0;
            virtual void sendTextures(const char* location, const GLuint*, int slot, GLuint glTextureType, const int arrSize) = 0;
            virtual void sendTexturesSafe(const char* location, const Texture**, int slot, const int arrSize) = 0;
            virtual void sendTexturesSafe(const char* location, const TextureCubemap**, int slot, const int arrSize) = 0;
            virtual void sendTexturesSafe(const char* location, const GLuint*, int slot, GLuint glTextureType, const int arrSize) = 0;


            virtual bool bindReadFBO(uint32_t fbo) = 0;
            virtual bool bindDrawFBO(uint32_t fbo) = 0;
            virtual bool bindRBO(uint32_t rbo) = 0;

            virtual bool bind(ModelInstance*) = 0;
            virtual bool bind(ShaderProgram*) = 0;
            virtual bool bind(Material*) = 0;
            virtual bool bind(Mesh*) = 0;

            virtual bool unbind(ModelInstance*) = 0;
            virtual bool unbind(ShaderProgram*) = 0;
            virtual bool unbind(Material*) = 0;
            virtual bool unbind(Mesh*) = 0;
            
            virtual void generatePBRData(TextureCubemap&, Handle convolutionTexture, Handle preEnvTexture, uint32_t convoludeSize, uint32_t prefilterSize) = 0;

            virtual void toggleShadowCaster(SunLight&, bool) = 0;
            virtual void toggleShadowCaster(PointLight&, bool) = 0;
            virtual void toggleShadowCaster(DirectionalLight&, bool) = 0;
            virtual void toggleShadowCaster(SpotLight&, bool) = 0;
            virtual void toggleShadowCaster(RodLight&, bool) = 0;
            virtual void toggleShadowCaster(ProjectionLight&, bool) = 0;

            virtual bool buildShadowCaster(SunLight&, uint32_t shadowMapWidth, uint32_t shadowMapSize, LightShadowFrustumType, float nearFactor, float farFactor) = 0;
            virtual bool buildShadowCaster(PointLight&) = 0;
            virtual bool buildShadowCaster(DirectionalLight&, uint32_t shadowMapWidth, uint32_t shadowMapSize, LightShadowFrustumType, float nearFactor, float farFactor) = 0;
            virtual bool buildShadowCaster(SpotLight&) = 0;
            virtual bool buildShadowCaster(RodLight&) = 0;
            virtual bool buildShadowCaster(ProjectionLight&) = 0;

            virtual void deleteAllShadowCasters(Scene&) = 0;

            virtual void sendGPUDataAllLights(const Scene&, const Camera&) = 0;
            virtual void sendGPUDataGI(Skybox*) = 0;
            virtual void sendGPUDataLight(const Camera&, const SunLight&,         const std::string& start) = 0;
            virtual int  sendGPUDataLight(const Camera&, const PointLight&,       const std::string& start) = 0;
            virtual void sendGPUDataLight(const Camera&, const DirectionalLight&, const std::string& start) = 0;
            virtual int  sendGPUDataLight(const Camera&, const SpotLight&,        const std::string& start) = 0;
            virtual int  sendGPUDataLight(const Camera&, const RodLight&,         const std::string& start) = 0;
            virtual int  sendGPUDataLight(const Camera&, const ProjectionLight&,  const std::string& start) = 0;

            virtual void renderSunLight(Camera&, SunLight&, Viewport&) = 0;
            virtual void renderPointLight(Camera&, PointLight&) = 0;
            virtual void renderDirectionalLight(Camera&, DirectionalLight&, Viewport&) = 0;
            virtual void renderSpotLight(Camera&, SpotLight&) = 0;
            virtual void renderRodLight(Camera&, RodLight&) = 0;
            virtual void renderProjectionLight(Camera&, ProjectionLight&) = 0;

            virtual void renderSkybox(Skybox*, Handle shaderProgram, Scene&, Viewport&, Camera&) = 0;
            virtual void renderMesh(Mesh&, uint32_t mode) = 0;
            virtual void renderDecal(ModelInstance&) = 0;
            virtual void renderLightProbe(LightProbe&) = 0;

            virtual void renderParticles(ParticleSystem&, Camera&) = 0;

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
            virtual void render2DTexture(
                uint32_t textureHandle,
                int textureWidth,
                int textureHeight,
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
            virtual void renderTexture(
                uint32_t textureAddress,
                int textureWidth,
                int textureHeight,
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
                Handle textureHandle, 
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
                Handle fontHandle, 
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

            virtual void renderFullscreenTriangle(float depth = 0.0f, float inNear = 0.0f, float inFar = 1.0f) = 0;
            virtual void renderFullscreenQuad(float x, float y, float width, float height, float depth = 0.0f, float inNear = 0.0f, float inFar = 1.0f) = 0;
            virtual void renderFullscreenQuadCentered(float width, float height, float depth = 0.0f, float inNear = 0.0f, float inFar = 1.0f) = 0;

            virtual void renderInitFrame(Engine::priv::RenderModule&) = 0;
            virtual void render(Engine::priv::RenderModule&, Viewport&, bool mainRenderFunction) = 0;
            virtual void render2DAPI(Engine::priv::RenderModule&, Viewport&, bool mainRenderFunction) = 0;
    };
};

#endif