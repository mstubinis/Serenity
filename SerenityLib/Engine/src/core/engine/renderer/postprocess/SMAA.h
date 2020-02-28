#pragma once
#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_SMAA_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_SMAA_H_INCLUDE_GUARD

class  ShaderProgram;
class  Shader;
class  Viewport;

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <string>
#include <vector>

struct SMAAQualityLevel {enum Level {
    Low, Medium, High, Ultra,
};};

namespace Engine::priv {
    class  GBuffer;
    class  Renderer;
    class  SMAA final {
        private:
            struct PassStage final { enum Stage : unsigned int {
                Edge,
                Blend,
                Neighbor,
                Final,
                _TOTAL,
            };};

            std::vector<Shader*>        m_Vertex_Shaders;
            std::vector<Shader*>        m_Fragment_Shaders;
            std::vector<ShaderProgram*> m_Shader_Programs;

            std::vector<std::string>    m_Vertex_Shaders_Code;
            std::vector<std::string>    m_Fragment_Shaders_Code;

        public:
            unsigned int  AreaTexture                             = 0;
            unsigned int  SearchTexture                           = 0;
            unsigned char MAX_SEARCH_STEPS                        = 32;
            unsigned char MAX_SEARCH_STEPS_DIAG                   = 16;
            unsigned char CORNER_ROUNDING                         = 25;
            unsigned char AREATEX_MAX_DISTANCE                    = 16;
            unsigned char AREATEX_MAX_DISTANCE_DIAG               = 20;
            float         THRESHOLD                               = 0.05f;
            float         LOCAL_CONTRAST_ADAPTATION_FACTOR        = 2.0f;
            float         DEPTH_THRESHOLD                         = (0.1f * THRESHOLD);
            float         PREDICATION_THRESHOLD                   = 0.01f;
            float         PREDICATION_SCALE                       = 2.0f;
            float         PREDICATION_STRENGTH                    = 0.4f;
            float         REPROJECTION_WEIGHT_SCALE               = 30.0f;
            float         AREATEX_SUBTEX_SIZE                     = 0.14285714285f; //(1 / 7)
            glm::vec2     AREATEX_PIXEL_SIZE                      = glm::vec2(glm::vec2(1.0f) / glm::vec2(160.0f, 560.0f));
            bool          PREDICATION                             = false;
            bool          REPROJECTION                            = false;

            SMAA();
            ~SMAA();

            void init();

            const bool init_shaders();

            void passEdge(GBuffer&, const glm::vec4& PIXEL_SIZE, const Viewport& viewport, const unsigned int sceneTexture, const unsigned int outTexture, const Engine::priv::Renderer& renderer);
            void passBlend(GBuffer&, const glm::vec4& PIXEL_SIZE, const Viewport& viewport, const unsigned int outTexture, const Engine::priv::Renderer& renderer);
            void passNeighbor(GBuffer&, const glm::vec4& PIXEL_SIZE, const Viewport& viewport, const unsigned int sceneTexture, const Engine::priv::Renderer& renderer);
            void passFinal(GBuffer&, const Viewport& viewport, const Engine::priv::Renderer& renderer); //currently unused

            static SMAA smaa;
    };
};
namespace Engine::Renderer::smaa {
    void setThreshold(const float threshold);
    void setSearchSteps(const unsigned int steps);
    void disableCornerDetection();
    void enableCornerDetection(const unsigned int detection = 25);
    void disableDiagonalDetection();
    void enableDiagonalDetection(const unsigned int detection = 8);
    void setQuality(const SMAAQualityLevel::Level level);
    void setPredicationThreshold(const float threshold);
    void setPredicationScale(const float scale);
    void setPredicationStrength(const float strength);
    void setReprojectionScale(const float scale);
    void enablePredication(const bool b = true);
    void disablePredication();
    void enableReprojection(const bool b = true);
    void disableReprojection();
};
#endif