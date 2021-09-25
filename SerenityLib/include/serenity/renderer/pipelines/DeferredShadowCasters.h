#pragma once
#ifndef ENGINE_RENDERER_PIPELINE_DEFERRED_SHADOW_CASTERS_H
#define ENGINE_RENDERER_PIPELINE_DEFERRED_SHADOW_CASTERS_H

class  Camera;
class  DirectionalLight;

#include <serenity/lights/Lights.h>
#include <vector>

namespace Engine::priv {
    class GLDeferredDirectionalLightShadowInfo final : public DirectionalLightShadowData {
        private:
            bool initGL();
            void calculateSplits(const Camera&);
        public:
            std::array<GLuint, (size_t)DIRECTIONAL_LIGHT_NUM_CASCADING_SHADOW_MAPS>  m_DepthTexture        = { 0 };
            std::vector<glm::mat4>                                                   m_BufferLightMatrices = std::vector<glm::mat4>(DIRECTIONAL_LIGHT_NUM_CASCADING_SHADOW_MAPS);
            std::vector<float>                                                       m_BufferVClips        = std::vector<float>(DIRECTIONAL_LIGHT_NUM_CASCADING_SHADOW_MAPS);
            GLuint                                                                   m_FBO                 = 0;


            GLDeferredDirectionalLightShadowInfo() = delete;
            GLDeferredDirectionalLightShadowInfo(const Camera&, const DirectionalLight&, uint32_t shadowMapWidth, uint32_t shadowMapHeight, LightShadowFrustumType, float nearFactor, float farFactor);
            ~GLDeferredDirectionalLightShadowInfo();

            void bindUniformsReading(int textureStartSlot, const Camera&) noexcept;
            void bindUniformsWriting(int cascadeMapIndex);
            void calculateOrthographicProjections(const Camera&, const DirectionalLight&);
            void setShadowInfo(uint32_t shadowMapWidth, uint32_t shadowMapHeight, LightShadowFrustumType, float nearFactor, float farFactor);
    };
    class GLDeferredSunLightShadowInfo final : public SunLightShadowData {

    };
    class GLDeferredPointLightShadowInfo final : public PointLightShadowData {

    };
    class GLDeferredProjectionLightShadowInfo final : public ProjectionLightShadowData {

    };
    class GLDeferredRodLightShadowInfo final : public RodLightShadowData {

    };
    class GLDeferredSpotLightShadowInfo final : public SpotLightShadowData {

    };

    class GLDeferredLightShadowCasters final {
        public:
            template<class LIGHT, class SHADOW_DATA>
            class CasterContainer : public std::vector<std::tuple<LIGHT*, SHADOW_DATA*>> {};

            template<class LIGHT, class SHADOW_DATA>
            class CasterHashMap : public std::unordered_map<const LIGHT*, SHADOW_DATA*> {}; //bool is shadow casting enabled or disabled
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
            template<class CONTAINER>
            void internal_free_memory_container(CONTAINER& container) noexcept {
                for (auto& itr : container) {
                    SAFE_DELETE(std::get<1>(itr));
                }
                container.clear();
            }
            ~GLDeferredLightShadowCasters() {
                internal_free_memory_container(m_ShadowCastersSpot);
                internal_free_memory_container(m_ShadowCastersRod);
                internal_free_memory_container(m_ShadowCastersProjection);
                internal_free_memory_container(m_ShadowCastersPoint);
                internal_free_memory_container(m_ShadowCastersSun);
                internal_free_memory_container(m_ShadowCastersDirectional);
            }
    };
}

#endif