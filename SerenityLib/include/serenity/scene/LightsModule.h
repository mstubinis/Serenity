#pragma once
#ifndef ENGINE_SCENE_LIGHTS_MODULE_H
#define ENGINE_SCENE_LIGHTS_MODULE_H

#include <serenity/scene/LightContainer.h>

namespace Engine::priv {
    class LightsModule final {
        private:
            std::vector<ILightContainer*>  m_Lights;
            static inline uint32_t         m_RegisteredLights = 0;

            template<class LIGHT> inline void internal_assert_registered_light_type() const noexcept {
                ASSERT(LIGHT::TYPE_ID != std::numeric_limits<uint32_t>().max(), __FUNCTION__ << "(): type: " << typeid(LIGHT).name() << " must be registered first!");
            }
            template<class LIGHT> [[nodiscard]] LightContainer<LIGHT>* internal_get_light_container() noexcept {
                return (LIGHT::TYPE_ID < m_Lights.size()) ? static_cast<LightContainer<LIGHT>*>(m_Lights[LIGHT::TYPE_ID]) : nullptr;
            }
        public:
            LightsModule() = default;
            ~LightsModule() {
                SAFE_DELETE_VECTOR(m_Lights);
            }

            template<class LIGHT> [[nodiscard]] LightContainer<LIGHT>& getLights() noexcept {
                internal_assert_registered_light_type<LIGHT>();
                auto container = internal_get_light_container<LIGHT>();
                ASSERT(container != nullptr, __FUNCTION__ << "(): container was nullptr!");
                return *container;
            }
            template<class LIGHT> uint32_t registerLightType() {
                if (LIGHT::TYPE_ID == std::numeric_limits<uint32_t>().max()) {
                    LIGHT::TYPE_ID = m_RegisteredLights++;
                }
                internal_assert_registered_light_type<LIGHT>();
                if (LIGHT::TYPE_ID >= m_Lights.size()) {
                    m_Lights.emplace_back(NEW Engine::priv::LightContainer<LIGHT>{});
                }
                return LIGHT::TYPE_ID;
            }
            template<class LIGHT, class ... ARGS> [[nodiscard]] inline Engine::view_ptr<LIGHT> createLight(ARGS&&... args) {
                internal_assert_registered_light_type<LIGHT>();
                auto lightContainer = internal_get_light_container<LIGHT>();
                return (lightContainer) ? lightContainer->createLight(std::forward<ARGS>(args)...) : nullptr;
            }
            template<class LIGHT> bool deleteLight(LIGHT* light) noexcept {
                internal_assert_registered_light_type<LIGHT>();
                auto lightContainer = internal_get_light_container<LIGHT>();
                return (lightContainer) ? lightContainer->deleteLight(light) : false;
            }
            template<class LIGHT> bool setShadowCaster(LIGHT* light, bool isShadowCaster) {
                internal_assert_registered_light_type<LIGHT>();
                auto lightContainer = internal_get_light_container<LIGHT>();
                return (lightContainer) ? lightContainer->setShadowCaster(light, isShadowCaster) : false;
            }

            inline std::vector<ILightContainer*>::iterator begin() noexcept { return m_Lights.begin(); }
            inline std::vector<ILightContainer*>::const_iterator begin() const noexcept { return m_Lights.cbegin(); }
            inline std::vector<ILightContainer*>::iterator end() noexcept { return m_Lights.end(); }
            inline std::vector<ILightContainer*>::const_iterator end() const noexcept { return m_Lights.cend(); }
    };
}

#endif