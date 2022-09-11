#pragma once
#ifndef ENGINE_RENDERER_PIPELINE_DEFERRED_SHADOW_CASTERS_H
#define ENGINE_RENDERER_PIPELINE_DEFERRED_SHADOW_CASTERS_H

class  Camera;
class  DirectionalLight;

#include <serenity/lights/Lights.h>
#include <vector>

namespace Engine::priv {
    class GLDeferredDirectionalLightShadowInfo : public DirectionalLightShadowData {
        private:
            bool initGL();
            void calculateSplits(const Camera&);
        public:
            std::array<GLuint, DIRECTIONAL_LIGHT_NUM_CASCADING_SHADOW_MAPS> m_DepthTexture = { 0 };
            GLuint                                                          m_FBO          = 0;


            GLDeferredDirectionalLightShadowInfo() = delete;
            GLDeferredDirectionalLightShadowInfo(uint32_t shadowMapWidth, uint32_t shadowMapHeight, LightShadowFrustumType, float nearFactor, float farFactor);

            GLDeferredDirectionalLightShadowInfo(const GLDeferredDirectionalLightShadowInfo&) = delete;
            GLDeferredDirectionalLightShadowInfo& operator=(const GLDeferredDirectionalLightShadowInfo&) = delete;
            GLDeferredDirectionalLightShadowInfo(GLDeferredDirectionalLightShadowInfo&&) noexcept;
            GLDeferredDirectionalLightShadowInfo& operator=(GLDeferredDirectionalLightShadowInfo&&) noexcept;

            ~GLDeferredDirectionalLightShadowInfo();

            [[nodiscard]] inline bool isReady() const noexcept { return m_FBO != 0; }

            void bindUniformsReading(int textureStartSlot) noexcept;
            void bindUniformsWriting(int cascadeMapIndex);
            void calculateOrthographicProjections(const Camera&, const glm::vec3& direction);
            void setShadowInfo(uint32_t shadowMapWidth, uint32_t shadowMapHeight, LightShadowFrustumType, float nearFactor, float farFactor);
            void setShadowInfo(uint32_t shadowMapWidth, uint32_t shadowMapHeight);
    };
    class GLDeferredSunLightShadowInfo : public GLDeferredDirectionalLightShadowInfo {
        public:
            GLDeferredSunLightShadowInfo() = delete;

            GLDeferredSunLightShadowInfo(const GLDeferredSunLightShadowInfo&) = delete;
            GLDeferredSunLightShadowInfo& operator=(const GLDeferredSunLightShadowInfo&) = delete;
            GLDeferredSunLightShadowInfo(GLDeferredSunLightShadowInfo&&) noexcept;
            GLDeferredSunLightShadowInfo& operator=(GLDeferredSunLightShadowInfo&&) noexcept;

            GLDeferredSunLightShadowInfo(uint32_t shadowMapWidth, uint32_t shadowMapHeight, LightShadowFrustumType, float nearFactor, float farFactor);
    };
    class GLDeferredPointLightShadowInfo : public PointLightShadowData {

    };
    class GLDeferredProjectionLightShadowInfo : public ProjectionLightShadowData {

    };
    class GLDeferredRodLightShadowInfo : public RodLightShadowData {

    };
    class GLDeferredSpotLightShadowInfo : public SpotLightShadowData {

    };

    //TODO: rethink this - due to invalidation when resizing consider using handles.
    class GLDeferredLightShadowCasters final {
        public:
            template<class LIGHT, class SHADOW_DATA>
            class CasterContainer : public std::vector<std::vector<std::pair<LIGHT*, SHADOW_DATA>>> {};

            template<class LIGHT, class SHADOW_DATA>
            class CasterHashMap : public std::vector<std::unordered_map<const LIGHT*, SHADOW_DATA*>> {}; //bool is shadow casting enabled or disabled
        public:
            CasterContainer<DirectionalLight, GLDeferredDirectionalLightShadowInfo>  m_ShadowCastersDirectional;
            CasterContainer<SunLight,         GLDeferredSunLightShadowInfo>          m_ShadowCastersSun;
            CasterContainer<PointLight,       GLDeferredPointLightShadowInfo>        m_ShadowCastersPoint;
            CasterContainer<ProjectionLight,  GLDeferredProjectionLightShadowInfo>   m_ShadowCastersProjection;
            CasterContainer<RodLight,         GLDeferredRodLightShadowInfo>          m_ShadowCastersRod;
            CasterContainer<SpotLight,        GLDeferredSpotLightShadowInfo>         m_ShadowCastersSpot;

            CasterHashMap<DirectionalLight, GLDeferredDirectionalLightShadowInfo>  m_ShadowCastersDirectionalHashed;
            CasterHashMap<SunLight, GLDeferredSunLightShadowInfo>                  m_ShadowCastersSunHashed;
            CasterHashMap<PointLight, GLDeferredPointLightShadowInfo>              m_ShadowCastersPointHashed;
            CasterHashMap<ProjectionLight, GLDeferredProjectionLightShadowInfo>    m_ShadowCastersProjectionHashed;
            CasterHashMap<RodLight, GLDeferredRodLightShadowInfo>                  m_ShadowCastersRodHashed;
            CasterHashMap<SpotLight, GLDeferredSpotLightShadowInfo>                m_ShadowCastersSpotHashed;
        public:
            ~GLDeferredLightShadowCasters();
            void clearSceneData(const Scene&);
            void clearSceneData(const uint32_t sceneID);
    };
}

#endif