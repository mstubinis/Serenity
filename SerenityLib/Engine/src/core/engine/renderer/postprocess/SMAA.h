#pragma once
#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_SMAA_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_SMAA_H_INCLUDE_GUARD

class  ShaderProgram;
class  Shader;

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <string>
#include <vector>

struct SMAAQualityLevel {enum Level {
    Low, Medium, High, Ultra,
};};


namespace Engine::priv {
    class  GBuffer;
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


            unsigned int  AreaTexture;
            unsigned int  SearchTexture;
            float         THRESHOLD;
            int           MAX_SEARCH_STEPS;
            int           MAX_SEARCH_STEPS_DIAG;
            int           CORNER_ROUNDING;
            float         LOCAL_CONTRAST_ADAPTATION_FACTOR;
            float         DEPTH_THRESHOLD;
            bool          PREDICATION;
            float         PREDICATION_THRESHOLD;
            float         PREDICATION_SCALE;
            float         PREDICATION_STRENGTH;
            bool          REPROJECTION;
            float         REPROJECTION_WEIGHT_SCALE;
            int           AREATEX_MAX_DISTANCE;
            int           AREATEX_MAX_DISTANCE_DIAG;
            glm::vec2     AREATEX_PIXEL_SIZE;
            float         AREATEX_SUBTEX_SIZE;

            SMAA();
            ~SMAA();

            void init();

            const bool init_shaders();

            void passEdge(GBuffer&, const glm::vec4& PIXEL_SIZE, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& sceneTexture, const unsigned int& outTexture);
            void passBlend(GBuffer&, const glm::vec4& PIXEL_SIZE, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& outTexture);
            void passNeighbor(GBuffer&, const glm::vec4& PIXEL_SIZE, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& sceneTexture);
            void passFinal(GBuffer&, const unsigned int& fboWidth, const unsigned int& fboHeight); //currently unused

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