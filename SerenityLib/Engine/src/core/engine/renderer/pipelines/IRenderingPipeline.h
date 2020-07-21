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
    class Renderer;
    class ParticleSystem;
};

#include <core/engine/fonts/FontIncludes.h>
#include <core/engine/renderer/RendererIncludes.h>

namespace Engine::priv {
    class IRenderingPipeline {
        friend class Renderer;
        public:
            virtual ~IRenderingPipeline(){}

            virtual void init() = 0;
            virtual void onPipelineChanged() = 0;
            virtual void onFullscreen() = 0;
            virtual void onResize(unsigned int newWidth, unsigned int newHeight) = 0;
            virtual void onOpenGLContextCreation(unsigned int windowWidth, unsigned int windowHeight, unsigned int glslVersion, unsigned int openglVersion) = 0;
            virtual void restoreDefaultState() = 0;
            virtual void restoreCurrentState() = 0;
            virtual void clear2DAPI() = 0;
            virtual void sort2DAPI() = 0;

            virtual void renderPhysicsAPI(bool mainRenderFunc, Viewport& viewport, Camera& camera, Scene& scene) = 0;

            //non textured 2d api elements will be exposed to anti-aliasing post processing
            virtual void render2DAPINonTextured(bool mainRenderFunc, Viewport& viewport) = 0;

            virtual void render2DAPI(bool mainRenderFunc, Viewport& viewport) = 0;


            virtual ShaderProgram* getCurrentBoundShaderProgram() = 0;
            virtual Material* getCurrentBoundMaterial() = 0;
            virtual Mesh* getCurrentBoundMesh() = 0;


            virtual unsigned int getUniformLocation(const char* location) = 0;
            virtual unsigned int getUniformLocationUnsafe(const char* location) = 0;

            virtual unsigned int getMaxNumTextureUnits() = 0;

            virtual bool stencilOperation(unsigned int stencilFail, unsigned int depthFail, unsigned int depthPass) = 0;
            virtual bool stencilMask(unsigned int mask) = 0;
            virtual bool stencilFunction(unsigned int stencilFunction, unsigned int reference, unsigned int mask) = 0;
            virtual bool cullFace(unsigned int face) = 0;
            virtual bool setDepthFunction(unsigned int depthFunction) = 0;
            virtual bool setViewport(float x, float y, float width, float height) = 0;
            virtual void clear(bool color, bool depth, bool stencil) = 0;
            virtual bool colorMask(bool r, bool g, bool b, bool alpha) = 0;
            virtual bool clearColor(bool r, bool g, bool b, bool alpha) = 0;

            virtual bool bindTextureForModification(unsigned int textureType, unsigned int textureObject) = 0;
            virtual bool bindVAO(unsigned int vaoObject) = 0;
            virtual bool deleteVAO(unsigned int& vaoObject) = 0;
            virtual void generateAndBindTexture(unsigned int textureType, unsigned int& textureObject) = 0;
            virtual void generateAndBindVAO(unsigned int& vaoObject) = 0;

            virtual bool enableAPI(unsigned int apiEnum) = 0;
            virtual bool disableAPI(unsigned int apiEnum) = 0;
            virtual bool enableAPI_i(unsigned int apiEnum, unsigned int index) = 0;
            virtual bool disableAPI_i(unsigned int apiEnum, unsigned int index) = 0;

            virtual void sendTexture(const char* location, Texture& texture, int slot) = 0;
            virtual void sendTexture(const char* location, unsigned int textureObject, int slot, unsigned int textureTarget) = 0;
            virtual void sendTextureSafe(const char* location, Texture& texture, int slot) = 0;
            virtual void sendTextureSafe(const char* location, unsigned int textureObject, int slot, unsigned int textureTarget) = 0;

            virtual bool bindReadFBO(unsigned int fbo) = 0;
            virtual bool bindDrawFBO(unsigned int fbo) = 0;
            virtual bool bindRBO(unsigned int rbo) = 0;

            virtual bool bind(ModelInstance* modelInstance) = 0;
            virtual bool bind(ShaderProgram* program) = 0;
            virtual bool bind(Material* material) = 0;
            virtual bool bind(Mesh* mesh) = 0;

            virtual bool unbind(ModelInstance* modelInstance) = 0;
            virtual bool unbind(ShaderProgram* program) = 0;
            virtual bool unbind(Material* material) = 0;
            virtual bool unbind(Mesh* mesh) = 0;

            virtual void generatePBRData(Texture& texture, unsigned int convoludeSize, unsigned int prefilterSize) = 0;

            virtual void sendGPUDataSunLight(Camera& camera, SunLight& sunLight, const std::string& start) = 0;
            virtual int sendGPUDataPointLight(Camera& camera, PointLight& pointLight, const std::string& start) = 0;
            virtual void sendGPUDataDirectionalLight(Camera& camera, DirectionalLight& directionalLight, const std::string& start) = 0;
            virtual int sendGPUDataSpotLight(Camera& camera, SpotLight& spotLight, const std::string& start) = 0;
            virtual int sendGPUDataRodLight(Camera& camera, RodLight& rodLight, const std::string& start) = 0;
            virtual int sendGPUDataProjectionLight(Camera& camera, ProjectionLight& projectionLight, const std::string& start) = 0;

            virtual void renderSunLight(Camera& camera, SunLight& sunLight, Viewport& viewport) = 0;
            virtual void renderPointLight(Camera& camera, PointLight& pointLight) = 0;
            virtual void renderDirectionalLight(Camera& camera, DirectionalLight& directionalLight, Viewport& viewport) = 0;
            virtual void renderSpotLight(Camera& camera, SpotLight& spotLight) = 0;
            virtual void renderRodLight(Camera& camera, RodLight& rodLight) = 0;
            virtual void renderProjectionLight(Camera& camera, ProjectionLight& rodLight) = 0;

            virtual void renderSkybox(Skybox*, ShaderProgram& shaderProgram, Scene& scene, Viewport& viewport, Camera& camera) = 0;
            virtual void renderMesh(Mesh& mesh, unsigned int mode) = 0;
            virtual void renderDecal(ModelInstance& decalModelInstance) = 0;
            virtual void renderLightProbe(LightProbe& lightProbe) = 0;

            virtual void renderParticles(ParticleSystem& particleSystem, Camera& camera, ShaderProgram& program) = 0;

            virtual void render2DText(
                const std::string& text, 
                const Font& font, 
                const glm::vec2& position, 
                const glm::vec4& color, 
                float angle, 
                const glm::vec2& scale, 
                float depth, 
                TextAlignment textAlignment, 
                const glm::vec4& scissor = NO_SCISSOR
            ) = 0;
            virtual void render2DTexture(
                Texture* texture, 
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
                Texture& tex, 
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
                const Font& fnt, 
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

            virtual void renderFullscreenTriangle() = 0;
            virtual void renderFullscreenQuad() = 0;

            virtual void render(Engine::priv::Renderer& renderer, Viewport& viewport, bool mainRenderFunction) = 0;
    };
};

#endif