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
};

#include <core/engine/renderer/pipelines/IRenderingPipeline.h>
#include <core/engine/model/ModelInstanceIncludes.h>
#include <core/engine/renderer/opengl/State.h>
#include <core/engine/renderer/opengl/Extensions.h>

#include <glm/mat4x4.hpp>
#include <vector>

namespace Engine::priv {
    class DeferredPipeline final : public IRenderingPipeline {

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
        };

        private:
            OpenGLState                  OpenGLStateMachine;
            OpenGLExtensions             OpenGLExtensionsManager;

            FullscreenQuad*              m_FullscreenQuad;
            FullscreenTriangle*          m_FullscreenTriangle;

            std::vector<glm::vec3>       m_Text_Points;
            std::vector<glm::vec2>       m_Text_UVs;
            std::vector<unsigned short>  m_Text_Indices;

            UBOCameraDataStruct          m_UBOCameraDataStruct;
            UniformBufferObject*         m_UBOCamera;
            glm::mat4                    m_2DProjectionMatrix;
            GBuffer*                     m_GBuffer;

            std::vector<Shader*>         m_InternalShaders;
            std::vector<ShaderProgram*>  m_InternalShaderPrograms;

            void internal_render_per_frame_preparation();
            void internal_pass_geometry(const Viewport& viewport, const Camera& camera);
            void internal_pass_ssao(const Viewport& viewport, const Camera& camera);
            void internal_pass_stencil();
            void internal_pass_lighting();
            void internal_pass_forward();
            void internal_pass_god_rays(const Viewport& viewport, const Camera& camera);
            void internal_pass_hdr(const Viewport& viewport, const Camera& camera);
            void internal_pass_bloom();
            void internal_pass_depth_of_field();
            void internal_pass_aa();
            void internal_pass_final();
            void internal_pass_depth_and_transparency(); //TODO: recheck this
            void internal_pass_copy_depth();
            void internal_pass_blur();

            void internal_generate_pbr_data_for_texture(ShaderProgram& covoludeShaderProgram, ShaderProgram& prefilterShaderProgram, Texture& texture, const unsigned int& convoludeTextureSize, const unsigned int& preEnvFilterSize);
            void internal_generate_brdf_lut(ShaderProgram& program, const unsigned int& brdfSize);

            void internal_render_2d_text_left(const std::string& text, const Font& font, const float& newLineGlyphHeight, float& x, float& y, const float& z);
            void internal_render_2d_text_center(const std::string& text, const Font& font, const float& newLineGlyphHeight, float& x, float& y, const float& z);
            void internal_render_2d_text_right(const std::string& text, const Font& font, const float& newLineGlyphHeight, float& x, float& y, const float& z);

        public:
            DeferredPipeline();
            ~DeferredPipeline();

            void init();
            void onPipelineChanged();
            void onFullscreen();
            void onResize(const unsigned int& newWidth, const unsigned int& newHeight);
            void onOpenGLContextCreation();

            void renderSkybox(Skybox*, ShaderProgram& shaderProgram, const Scene& scene, const Viewport& viewport, const Camera& camera);
            void renderSunLight(const Camera& c, const SunLight& s);
            void renderPointLight(const Camera& c, const PointLight& p);
            void renderDirectionalLight(const Camera& c, const DirectionalLight& d);
            void renderSpotLight(const Camera& c, const SpotLight& s);
            void renderRodLight(const Camera& c, const RodLight& r);
            void renderMesh(const Mesh& mesh, const unsigned int mode = ModelDrawingMode::Triangles);

            void renderParticle(const Particle& particle);

            void render2DText(const std::string& text, const Font& font, const glm::vec2& position, const glm::vec4& color, const float angle, const glm::vec2& scale, const float depth, const TextAlignment::Type& textAlignment, const glm::vec4& scissor = glm::vec4(-1.0f));
            void render2DTexture(const Texture* texture, const glm::vec2& position, const glm::vec4& color, const float angle, const glm::vec2& scale, const float depth, const Alignment::Type& align, const glm::vec4& scissor = glm::vec4(-1.0f));
            void render2DTriangle(const glm::vec2& pos, const glm::vec4& color, const float angle, const float width, const float height, const float depth, const Alignment::Type& align, const glm::vec4& scissor = glm::vec4(-1.0f));

            void update(const float& dt);
            void render(const Viewport& viewport);
    };
};

#endif