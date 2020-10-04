#pragma once
#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_SMAA_H
#define ENGINE_RENDERER_POSTPROCESS_SMAA_H

class  ShaderProgram;
class  Shader;
class  Viewport;

enum class SMAAQualityLevel : unsigned char {
    Low, 
    Medium, 
    High, 
    Ultra,
};

#include <core/engine/resources/Handle.h>

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

            std::vector<Handle>        m_Vertex_Shaders;
            std::vector<Handle>        m_Fragment_Shaders;
            std::vector<Handle>        m_Shader_Programs;

            std::vector<std::string>   m_Vertex_Shaders_Code;
            std::vector<std::string>   m_Fragment_Shaders_Code;

            SMAA();
            ~SMAA();
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

            void init();

            bool init_shaders();

            void passEdge(GBuffer&, const glm::vec4& PIXEL_SIZE, const Viewport& viewport, unsigned int sceneTexture, unsigned int outTexture, const Engine::priv::Renderer& renderer);
            void passBlend(GBuffer&, const glm::vec4& PIXEL_SIZE, const Viewport& viewport, unsigned int outTexture, const Engine::priv::Renderer& renderer);
            void passNeighbor(GBuffer&, const glm::vec4& PIXEL_SIZE, const Viewport& viewport, unsigned int sceneTexture, const Engine::priv::Renderer& renderer);
            void passFinal(GBuffer&, const Viewport& viewport, const Engine::priv::Renderer& renderer); //currently unused

            static SMAA STATIC_SMAA;
    };
};
namespace Engine::Renderer::smaa {
    void setThreshold(float threshold);
    void setSearchSteps(unsigned int searchSteps);
    void disableCornerDetection();
    void enableCornerDetection(unsigned int detection = 25);
    void disableDiagonalDetection();
    void enableDiagonalDetection(unsigned int detection = 8);
    void setQuality(SMAAQualityLevel qualityLevel);
    void setPredicationThreshold(float predicationThreshold);
    void setPredicationScale(float predicationScale);
    void setPredicationStrength(float predicationStrength);
    void setReprojectionScale(float reprojectionScale);
    void enablePredication(bool enabledPredication = true);
    void disablePredication();
    void enableReprojection(bool enabledReprojection = true);
    void disableReprojection();
};
#endif