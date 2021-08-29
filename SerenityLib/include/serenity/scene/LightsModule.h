#pragma once
#ifndef ENGINE_SCENE_LIGHTS_MODULE_H
#define ENGINE_SCENE_LIGHTS_MODULE_H

#include <serenity/scene/LightContainer.h>

namespace Engine::priv {
    class LightsModule final {
        using LightContainers = std::vector<std::unique_ptr<ILightContainer>>;
        private:
            LightContainers        m_Lights;
            uint32_t               m_RegisteredLights = 0;

            template<class LIGHT> inline void internal_assert_registered_light_type() noexcept {
                ASSERT(LIGHT::TYPE_ID > 0, __FUNCTION__ << "(): type: " << typeid(LIGHT).name() << " must be registered first!");
            }
            template<class LIGHT> [[nodiscard]] inline LightContainer<LIGHT>* internal_get_light_container() noexcept {
                return static_cast<LightContainer<LIGHT>*>(m_Lights[LIGHT::TYPE_ID - 1].get());
            }
        public:
            template<class LIGHT> [[nodiscard]] inline LightContainer<LIGHT>& getLights() noexcept {
                internal_assert_registered_light_type<LIGHT>();
                return *internal_get_light_container<LIGHT>();
            }
            template<class LIGHT> uint32_t registerLightType() {
                if (LIGHT::TYPE_ID == 0) {
                    LIGHT::TYPE_ID = ++m_RegisteredLights;
                }
                internal_assert_registered_light_type<LIGHT>();
                if (LIGHT::TYPE_ID > m_Lights.size()) {
                    m_Lights.emplace_back(std::make_unique<Engine::priv::LightContainer<LIGHT>>());
                }
                return LIGHT::TYPE_ID - 1;
            }
            template<class LIGHT, class ... ARGS> [[nodiscard]] inline Engine::view_ptr<LIGHT> createLight(ARGS&&... args) {
                internal_assert_registered_light_type<LIGHT>();
                return internal_get_light_container<LIGHT>()->createLight(std::forward<ARGS>(args)...);
            }
            template<class LIGHT> inline bool deleteLight(LIGHT* light) noexcept {
                internal_assert_registered_light_type<LIGHT>();
                return internal_get_light_container<LIGHT>()->deleteLight(light);
            }
            template<class LIGHT> inline bool setShadowCaster(LIGHT* light, bool isShadowCaster) {
                internal_assert_registered_light_type<LIGHT>();
                return internal_get_light_container<LIGHT>()->setShadowCaster(light, isShadowCaster);
            }

            inline LightContainers::iterator begin() noexcept { return m_Lights.begin(); }
            inline LightContainers::const_iterator begin() const noexcept { return m_Lights.cbegin(); }
            inline LightContainers::iterator end() noexcept { return m_Lights.end(); }
            inline LightContainers::const_iterator end() const noexcept { return m_Lights.cend(); }
    };
}

#endif