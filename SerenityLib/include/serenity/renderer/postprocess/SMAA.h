#pragma once
#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_SMAA_H
#define ENGINE_RENDERER_POSTPROCESS_SMAA_H

class  ShaderProgram;
class  Shader;
class  Viewport;

#include <serenity/system/TypeDefs.h>

enum class SMAAQualityLevel : uint8_t {
    Low, 
    Medium, 
    High, 
    Ultra,
};

#include <serenity/resources/Handle.h>
#include <serenity/dependencies/glm.h>
#include <vector>
#include <string>

namespace Engine::priv {
    class  GBuffer;
    class  RenderModule;
    class  SMAA final {
        private:
            struct PassStage final { enum Stage : uint32_t {
                Edge,
                Blend,
                Neighbor,
                Final,
                _TOTAL,
            };};

            std::vector<Handle>        m_Vertex_Shaders;
            std::vector<Handle>        m_Fragment_Shaders;
            std::vector<Handle>        m_Shader_Programs;

            std::vector<std::string>   m_Vertex_Shaders_Code;
            std::vector<std::string>   m_Fragment_Shaders_Code;

            void internal_init_edge_vertex_code(const std::string& common);
            void internal_init_edge_fragment_code(const std::string& common);
            void internal_init_blend_vertex_code(const std::string& common);
            void internal_init_blend_fragment_code(const std::string& common);
            void internal_init_neighbor_vertex_code(const std::string& common);
            void internal_init_neighbor_fragment_code(const std::string& common);

            SMAA();
            ~SMAA();
        public:
            glm::vec2     AREATEX_PIXEL_SIZE                      = glm::vec2(glm::vec2(1.0f) / glm::vec2(160.0f, 560.0f));
            uint32_t      AreaTexture                             = 0;
            uint32_t      SearchTexture                           = 0;
            float         THRESHOLD                               = 0.05f;
            float         LOCAL_CONTRAST_ADAPTATION_FACTOR        = 2.0f;
            float         DEPTH_THRESHOLD                         = (0.1f * THRESHOLD);
            float         PREDICATION_THRESHOLD                   = 0.01f;
            float         PREDICATION_SCALE                       = 2.0f;
            float         PREDICATION_STRENGTH                    = 0.4f;
            float         REPROJECTION_WEIGHT_SCALE               = 30.0f;
            float         AREATEX_SUBTEX_SIZE                     = 0.14285714285f; //(1 / 7)
            uint8_t       MAX_SEARCH_STEPS                        = 32;
            uint8_t       MAX_SEARCH_STEPS_DIAG                   = 16;
            uint8_t       CORNER_ROUNDING                         = 25;
            uint8_t       AREATEX_MAX_DISTANCE                    = 16;
            uint8_t       AREATEX_MAX_DISTANCE_DIAG               = 20;
            bool          PREDICATION                             = false;
            bool          REPROJECTION                            = false;

            static bool init();

            void passEdge(GBuffer&, const glm::vec4& PIXEL_SIZE, const Viewport& viewport, uint32_t sceneTexture, uint32_t outTexture, const Engine::priv::RenderModule& renderer);
            void passBlend(GBuffer&, const glm::vec4& PIXEL_SIZE, const Viewport& viewport, uint32_t outTexture, const Engine::priv::RenderModule& renderer);
            void passNeighbor(GBuffer&, const glm::vec4& PIXEL_SIZE, const Viewport& viewport, uint32_t sceneTexture, const Engine::priv::RenderModule& renderer);
            void passFinal(GBuffer&, const Viewport& viewport, const Engine::priv::RenderModule& renderer); //currently unused

            static SMAA STATIC_SMAA;
    };
};
namespace Engine::Renderer::smaa {
    void setQuality(SMAAQualityLevel qualityLevel);

    inline void setThreshold(float threshold) noexcept { Engine::priv::SMAA::STATIC_SMAA.THRESHOLD = threshold; }
    inline void setSearchSteps(uint32_t searchSteps) noexcept { Engine::priv::SMAA::STATIC_SMAA.MAX_SEARCH_STEPS = searchSteps; }
    inline void disableCornerDetection() noexcept { Engine::priv::SMAA::STATIC_SMAA.CORNER_ROUNDING = 0; }
    inline void enableCornerDetection(uint32_t detection = 25) noexcept { Engine::priv::SMAA::STATIC_SMAA.CORNER_ROUNDING = detection; }
    inline void disableDiagonalDetection() noexcept { Engine::priv::SMAA::STATIC_SMAA.MAX_SEARCH_STEPS_DIAG = 0; }
    inline void enableDiagonalDetection(uint32_t detection = 8) noexcept { Engine::priv::SMAA::STATIC_SMAA.MAX_SEARCH_STEPS_DIAG = detection; }
    inline void setPredicationThreshold(float predicationThreshold) noexcept { Engine::priv::SMAA::STATIC_SMAA.PREDICATION_THRESHOLD = predicationThreshold; }
    inline void setPredicationScale(float predicationScale) noexcept { Engine::priv::SMAA::STATIC_SMAA.PREDICATION_SCALE = predicationScale; }
    inline void setPredicationStrength(float predicationStrength) noexcept { Engine::priv::SMAA::STATIC_SMAA.PREDICATION_STRENGTH = predicationStrength; }
    inline void setReprojectionScale(float reprojectionScale) noexcept { Engine::priv::SMAA::STATIC_SMAA.REPROJECTION_WEIGHT_SCALE = reprojectionScale; }
    inline void enablePredication(bool enabledPredication = true) noexcept { Engine::priv::SMAA::STATIC_SMAA.PREDICATION = enabledPredication; }
    inline void disablePredication() noexcept { Engine::priv::SMAA::STATIC_SMAA.PREDICATION = false; }
    inline void enableReprojection(bool enabledReprojection = true) noexcept { Engine::priv::SMAA::STATIC_SMAA.REPROJECTION = enabledReprojection; }
    inline void disableReprojection() noexcept { Engine::priv::SMAA::STATIC_SMAA.REPROJECTION = false; }
};
#endif