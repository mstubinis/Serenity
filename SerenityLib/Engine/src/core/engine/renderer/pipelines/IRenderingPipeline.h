#pragma once
#ifndef ENGINE_RENDERER_RENDERING_PIPELINE_H
#define ENGINE_RENDERER_RENDERING_PIPELINE_H

class Mesh;
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

#include <string>
#include <core/engine/fonts/FontIncludes.h>
#include <core/engine/renderer/RendererIncludes.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Engine::priv {
    class IRenderingPipeline {
        public:
            virtual ~IRenderingPipeline() = 0;

            virtual void init() = 0;
            virtual void onPipelineChanged() = 0;
            virtual void onFullscreen() = 0;
            virtual void onResize(const unsigned int& newWidth, const unsigned int& newHeight) = 0;
            virtual void onOpenGLContextCreation() = 0;

            virtual void renderSkybox(Skybox*, ShaderProgram& shaderProgram, const Scene& scene, const Viewport& viewport, const Camera& camera) = 0;
            virtual void renderSunLight(const Camera& camera, const SunLight& sunLight) = 0;
            virtual void renderPointLight(const Camera& camera, const PointLight& pointLight) = 0;
            virtual void renderDirectionalLight(const Camera& camera, const DirectionalLight& directionalLight) = 0;
            virtual void renderSpotLight(const Camera& camera, const SpotLight& spotLight) = 0;
            virtual void renderRodLight(const Camera& camera, const RodLight& rodLight) = 0;
            virtual void renderMesh(const Mesh& mesh, const unsigned int mode) = 0;

            virtual void renderParticle(const Particle& particle) = 0;

            virtual void render2DText(const std::string& text, const Font& font, const glm::vec2& position, const glm::vec4& color, const float angle, const glm::vec2& scale, const float depth, const TextAlignment::Type& textAlignment, const glm::vec4& scissor = glm::vec4(-1.0f)) = 0;
            virtual void render2DTexture(const Texture* texture, const glm::vec2& position, const glm::vec4& color, const float angle, const glm::vec2& scale, const float depth, const Alignment::Type& align, const glm::vec4& scissor = glm::vec4(-1.0f)) = 0;
            virtual void render2DTriangle(const glm::vec2& pos, const glm::vec4& color, const float angle, const float width, const float height, const float depth, const Alignment::Type& align, const glm::vec4& scissor = glm::vec4(-1.0f)) = 0;

            virtual void update(const float& dt) = 0;
            virtual void render(const Viewport& viewport) = 0;

    };
};

#endif