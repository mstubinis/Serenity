#pragma once
#ifndef ENGINE_RENDERER_DEFERRED_PIPELINE_H
#define ENGINE_RENDERER_DEFERRED_PIPELINE_H

#include <core/engine/renderer/pipelines/IRenderingPipeline.h>

#include <glm/mat4x4.hpp>
#include <vector>

class UniformBufferObject;
namespace Engine {
    namespace epriv {
        class GBuffer;
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
                std::vector<glm::vec3>       m_Text_Points;
                std::vector<glm::vec2>       m_Text_UVs;
                std::vector<unsigned short>  m_Text_Indices;

                UBOCameraDataStruct          m_UBOCameraDataStruct;
                UniformBufferObject*         m_UBOCamera;
                glm::mat4                    m_2DProjectionMatrix;
                GBuffer*                     m_GBuffer;

                void internal_render_per_frame_preparation();
                void internal_pass_geometry();
                void internal_pass_ssao();
                void internal_pass_stencil();
                void internal_pass_lighting();
                void internal_pass_forward();
                void internal_pass_god_rays();
                void internal_pass_hdr();
                void internal_pass_bloom();
                void internal_pass_depth_of_field();
                void internal_pass_aa();
                void internal_pass_final();

                void internal_pass_depth_and_transparency(); //TODO: recheck this
                void internal_pass_copy_depth();
                void internal_pass_blur();

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

                void renderSkybox(Skybox*, ShaderProgram& shaderProgram, Scene& scene, Viewport& viewport, Camera& camera);
                void renderSunLight(Camera& c, SunLight& s);
                void renderPointLight(Camera& c, PointLight& p);
                void renderDirectionalLight(Camera& c, DirectionalLight& d);
                void renderSpotLight(Camera& c, SpotLight& s);
                void renderRodLight(Camera& c, RodLight& r);

                void renderParticle(Particle& particle);

                void render2DText(const std::string& text, const Font& font, const glm::vec2& position, const glm::vec4& color, const float angle, const glm::vec2& scale, const float depth, const TextAlignment::Type& textAlignment, const glm::vec4& scissor = glm::vec4(-1.0f));
                void render2DTexture(const Texture* texture, const glm::vec2& position, const glm::vec4& color, const float angle, const glm::vec2& scale, const float depth, const Alignment::Type& align, const glm::vec4& scissor = glm::vec4(-1.0f));
                void render2DTriangle(const glm::vec2& pos, const glm::vec4& color, const float angle, const float width, const float height, const float depth, const Alignment::Type& align, const glm::vec4& scissor = glm::vec4(-1.0f));

                void update(const double& dt);
                void render();
        };
    };
};

#endif