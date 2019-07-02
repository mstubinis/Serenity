#pragma once
#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_SMAA_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_SMAA_H_INCLUDE_GUARD

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

struct SMAAQualityLevel {enum Level {
    Low, Medium, High, Ultra,
};};

class  ShaderP;
namespace Engine {
namespace epriv {
    class  GBuffer;
    class  Postprocess_SMAA final {
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

            Postprocess_SMAA();
            ~Postprocess_SMAA();

            void init();

            void passEdge(ShaderP&, GBuffer&, const glm::vec4& PIXEL_SIZE, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& sceneTexture, const unsigned int& outTexture);
            void passBlend(ShaderP&, GBuffer&, const glm::vec4& PIXEL_SIZE, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& outTexture);
            void passNeighbor(ShaderP&, GBuffer&, const glm::vec4& PIXEL_SIZE, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& sceneTexture);
            void passFinal(ShaderP&, GBuffer&, const unsigned int& fboWidth, const unsigned int& fboHeight); //currently unused

            static Postprocess_SMAA SMAA;
    };
};
namespace Renderer {
namespace smaa {
    void setThreshold(float f);
    void setSearchSteps(unsigned int s);
    void disableCornerDetection();
    void enableCornerDetection(unsigned int c = 25);
    void disableDiagonalDetection();
    void enableDiagonalDetection(unsigned int d = 8);
    void setQuality(SMAAQualityLevel::Level l);
    void setPredicationThreshold(float f);
    void setPredicationScale(float f);
    void setPredicationStrength(float s);
    void setReprojectionScale(float s);
    void enablePredication(bool b = true);
    void disablePredication();
    void enableReprojection(bool b = true);
    void disableReprojection();
};
};
};


#endif