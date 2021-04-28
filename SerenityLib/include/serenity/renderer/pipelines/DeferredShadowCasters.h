#pragma once
#ifndef ENGINE_RENDERER_PIPELINE_DEFERRED_SHADOW_CASTERS_H
#define ENGINE_RENDERER_PIPELINE_DEFERRED_SHADOW_CASTERS_H

#include <serenity/lights/Lights.h>
#include <vector>

namespace Engine::priv {
    constexpr const float DIRECTIONAL_LIGHT_DEFAULT_SHADOW_MAP_SIZE = 1024.0f;
    class GLDeferredDirectionalLightShadowInfo final : public DirectionalLightShadowData {
        public:
            glm::vec2 m_TexelSize   = glm::vec2(1.0f / DIRECTIONAL_LIGHT_DEFAULT_SHADOW_MAP_SIZE, 1.0f / DIRECTIONAL_LIGHT_DEFAULT_SHADOW_MAP_SIZE);
            GLuint m_FBO            = 0;
            GLuint m_DepthTexture   = 0;
            uint32_t m_ShadowWidth  = static_cast<uint32_t>(DIRECTIONAL_LIGHT_DEFAULT_SHADOW_MAP_SIZE);
            uint32_t m_ShadowHeight = static_cast<uint32_t>(DIRECTIONAL_LIGHT_DEFAULT_SHADOW_MAP_SIZE);

            GLDeferredDirectionalLightShadowInfo() = delete;
            GLDeferredDirectionalLightShadowInfo(DirectionalLight& directionalLight, uint32_t shadowMapWidth, uint32_t shadowMapHeight, float orthographicRadius, float orthoNear, float orthoFar)
                : m_ShadowWidth{ shadowMapWidth }
                , m_ShadowHeight{ shadowMapHeight }
                , m_TexelSize{ glm::vec2{1.0f / static_cast<float>(shadowMapWidth), 1.0f / static_cast<float>(shadowMapHeight)} }
            {
                setOrtho(orthographicRadius, orthoNear, orthoFar);
                setLookAt(directionalLight.getComponent<ComponentTransform>()->getForward());
            }
            ~GLDeferredDirectionalLightShadowInfo() {
                glDeleteFramebuffers(1, &m_FBO);
                glDeleteTextures(1, &m_DepthTexture);
            }
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
            class CasterHashMap : public std::unordered_map<LIGHT*, SHADOW_DATA*> {}; //bool is shadow casting enabled or disabled
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