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
#include <string>
#include <core/engine/fonts/FontIncludes.h>
#include <core/engine/renderer/RendererIncludes.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Engine::priv {
    class IRenderingPipeline {
        friend class Renderer;
        public:
            virtual ~IRenderingPipeline(){}

            virtual void init() = 0;
            virtual void onPipelineChanged() = 0;
            virtual void onFullscreen() = 0;
            virtual void onResize(const unsigned int newWidth, const unsigned int newHeight) = 0;
            virtual void onOpenGLContextCreation(const unsigned int windowWidth, const unsigned int windowHeight, const unsigned int glslVersion, const unsigned int openglVersion) = 0;
            virtual void restoreDefaultState() = 0;
            virtual void restoreCurrentState() = 0;
            virtual void clear2DAPI() = 0;
            virtual void sort2DAPI() = 0;

            virtual void renderPhysicsAPI(const bool mainRenderFunc, const Viewport& viewport, const Camera& camera, const Scene& scene) = 0;

            //non textured 2d api elements will be exposed to anti-aliasing post processing
            virtual void render2DAPINonTextured(const bool mainRenderFunc, const Viewport& viewport) = 0;

            virtual void render2DAPI(const bool mainRenderFunc, const Viewport& viewport) = 0;


            virtual ShaderProgram* getCurrentBoundShaderProgram() = 0;
            virtual Material* getCurrentBoundMaterial() = 0;
            virtual Mesh* getCurrentBoundMesh() = 0;


            virtual const unsigned int getUniformLocation(const char* location) = 0;
            virtual const unsigned int getUniformLocationUnsafe(const char* location) = 0;

            virtual const unsigned int getMaxNumTextureUnits() = 0;

            virtual const bool stencilOperation(const unsigned int stencilFail, const unsigned int depthFail, const unsigned int depthPass) = 0;
            virtual const bool stencilMask(const unsigned int mask) = 0;
            virtual const bool stencilFunction(const unsigned int stencilFunction, const unsigned int reference, const unsigned int mask) = 0;
            virtual const bool cullFace(const unsigned int face) = 0;
            virtual const bool setDepthFunction(const unsigned int depthFunction) = 0;
            virtual const bool setViewport(const float x, const float y, const float width, const float height) = 0;
            virtual void clear(const bool color, const bool depth, const bool stencil) = 0;
            virtual const bool colorMask(const bool r, const bool g, const bool b, const bool alpha) = 0;
            virtual const bool clearColor(const bool r, const bool g, const bool b, const bool alpha) = 0;

            virtual const bool bindTextureForModification(const unsigned int textureType, const unsigned int textureObject) = 0;
            virtual const bool bindVAO(const unsigned int vaoObject) = 0;
            virtual const bool deleteVAO(unsigned int& vaoObject) = 0;
            virtual void generateAndBindTexture(const unsigned int textureType, unsigned int& textureObject) = 0;
            virtual void generateAndBindVAO(unsigned int& vaoObject) = 0;

            virtual const bool enableAPI(const unsigned int apiEnum) = 0;
            virtual const bool disableAPI(const unsigned int apiEnum) = 0;
            virtual const bool enableAPI_i(const unsigned int apiEnum, const unsigned int index) = 0;
            virtual const bool disableAPI_i(const unsigned int apiEnum, const unsigned int index) = 0;


            virtual void sendTexture(const char* location, const Texture& texture, const int slot) = 0;
            virtual void sendTexture(const char* location, const unsigned int textureObject, const int slot, const unsigned int textureTarget) = 0;
            virtual void sendTextureSafe(const char* location, const Texture& texture, const int slot) = 0;
            virtual void sendTextureSafe(const char* location, const unsigned int textureObject, const int slot, const unsigned int textureTarget) = 0;

            virtual const bool bindReadFBO(const unsigned int fbo) = 0;
            virtual const bool bindDrawFBO(const unsigned int fbo) = 0;
            virtual const bool bindRBO(const unsigned int rbo) = 0;

            virtual const bool bindShaderProgram(ShaderProgram* program) = 0;
            virtual const bool bindMaterial(Material* material) = 0;
            virtual const bool bindMesh(Mesh* mesh) = 0;

            virtual const bool unbindShaderProgram() = 0;
            virtual const bool unbindMaterial() = 0;
            virtual const bool unbindMesh(Mesh* mesh) = 0;

            virtual void generatePBRData(Texture& texture, const unsigned int convoludeSize, const unsigned int prefilterSize) = 0;

            virtual void renderSkybox(Skybox*, ShaderProgram& shaderProgram, const Scene& scene, const Viewport& viewport, const Camera& camera) = 0;
            virtual void renderSunLight(const Camera& camera, const SunLight& sunLight, const Viewport& viewport) = 0;
            virtual void renderPointLight(const Camera& camera, const PointLight& pointLight) = 0;
            virtual void renderDirectionalLight(const Camera& camera, const DirectionalLight& directionalLight, const Viewport& viewport) = 0;
            virtual void renderSpotLight(const Camera& camera, const SpotLight& spotLight) = 0;
            virtual void renderRodLight(const Camera& camera, const RodLight& rodLight) = 0;
            virtual void renderMesh(const Mesh& mesh, const unsigned int mode) = 0;
            virtual void renderDecal(ModelInstance& decalModelInstance) = 0;
            virtual void renderLightProbe(LightProbe& lightProbe) = 0;

            virtual void renderParticles(ParticleSystem& particleSystem, const Camera& camera, ShaderProgram& program) = 0;

            virtual void render2DText(
                const std::string& text, 
                const Font& font, 
                const glm::vec2& position, 
                const glm::vec4& color, 
                const float angle, 
                const glm::vec2& scale, 
                const float depth, 
                const TextAlignment::Type textAlignment, 
                const glm::vec4& scissor = glm::vec4(-1.0f)
            ) = 0;
            virtual void render2DTexture(
                const Texture* texture, 
                const glm::vec2& position, 
                const glm::vec4& color, 
                const float angle, 
                const glm::vec2& scale, 
                const float depth, 
                const Alignment::Type align, 
                const glm::vec4& scissor = glm::vec4(-1.0f)
            ) = 0;
            virtual void render2DTriangle(
                const glm::vec2& pos, 
                const glm::vec4& color, 
                const float angle, 
                const float width, 
                const float height, 
                const float depth, 
                const Alignment::Type align, 
                const glm::vec4& scissor = glm::vec4(-1.0f)
            ) = 0;


            virtual void renderTexture(
                const Texture& tex, 
                const glm::vec2& p, 
                const glm::vec4& c, 
                const float a, 
                const glm::vec2& s, 
                const float d, 
                const Alignment::Type align, 
                const glm::vec4& scissor
            ) = 0;
            virtual void renderText(
                const std::string& t, 
                const Font& fnt, 
                const glm::vec2& p, 
                const glm::vec4& c, 
                const float a, 
                const glm::vec2& s, 
                const float d, 
                const TextAlignment::Type align, 
                const glm::vec4& scissor
            ) = 0;
            virtual void renderBorder(
                const float borderSize, 
                const glm::vec2& pos, 
                const glm::vec4& col, 
                const float w, 
                const float h, 
                const float angle, 
                const float depth, 
                const Alignment::Type align, 
                const glm::vec4& scissor
            ) = 0;
            virtual void renderRectangle(
                const glm::vec2& pos, 
                const glm::vec4& col, 
                const float width, 
                const float height, 
                const float angle, 
                const float depth, 
                const Alignment::Type align, 
                const glm::vec4& scissor
            ) = 0;
            virtual void renderTriangle(
                const glm::vec2& position, 
                const glm::vec4& color, 
                const float angle, 
                const float width, 
                const float height, 
                const float depth, 
                const Alignment::Type align, 
                const glm::vec4& scissor
            ) = 0;

            virtual void renderFullscreenTriangle() = 0;
            virtual void renderFullscreenQuad() = 0;



            virtual void update(const float dt) = 0;
            virtual void render(Engine::priv::Renderer& renderer, const Viewport& viewport, const bool mainRenderFunction) = 0;

    };
};

#endif