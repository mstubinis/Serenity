#pragma once
#ifndef ENGINE_SCENE_LIGHTS_MODULE_H
#define ENGINE_SCENE_LIGHTS_MODULE_H

#include <serenity/scene/LightContainer.h>

namespace Engine::priv {
    class LightsModule final {
        using LightContainers = std::vector<ILightContainer*>;
        private:
            LightContainers          m_Lights;
            static inline uint32_t   m_RegisteredLights = 0;

            template<class LIGHT> inline void internal_assert_registered_light_type() noexcept {
                ASSERT(LIGHT::TYPE_ID > 0, __FUNCTION__ << "(): type: " << typeid(LIGHT).name() << " must be registered first!");
            }
            template<class LIGHT> [[nodiscard]] LightContainer<LIGHT>* internal_get_light_container() noexcept {
                const auto index = LIGHT::TYPE_ID - 1;
                if (index < m_Lights.size()) {
                    return static_cast<LightContainer<LIGHT>*>(m_Lights[index]);
                }
                return nullptr;
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
                if (LIGHT::TYPE_ID == 0) {
                    LIGHT::TYPE_ID = ++m_RegisteredLights;
                }
                internal_assert_registered_light_type<LIGHT>();
                if (LIGHT::TYPE_ID > m_Lights.size()) {
                    m_Lights.emplace_back(NEW Engine::priv::LightContainer<LIGHT>{});
                }
                return LIGHT::TYPE_ID - 1;
            }
            template<class LIGHT, class ... ARGS> [[nodiscard]] inline Engine::view_ptr<LIGHT> createLight(ARGS&&... args) {
                internal_assert_registered_light_type<LIGHT>();
                auto container = internal_get_light_container<LIGHT>();
                if (container) {
                    return container->createLight(std::forward<ARGS>(args)...);
                }
                return nullptr;
            }
            template<class LIGHT> bool deleteLight(LIGHT* light) noexcept {
                internal_assert_registered_light_type<LIGHT>();
                auto container = internal_get_light_container<LIGHT>();
                if (container) {
                    return container->deleteLight(light);
                }
                return false;
            }
            template<class LIGHT> bool setShadowCaster(LIGHT* light, bool isShadowCaster) {
                internal_assert_registered_light_type<LIGHT>();
                auto container = internal_get_light_container<LIGHT>();
                if (container) {
                    return container->setShadowCaster(light, isShadowCaster);
                }
                return false;
            }

            inline LightContainers::iterator begin() noexcept { return m_Lights.begin(); }
            inline LightContainers::const_iterator begin() const noexcept { return m_Lights.cbegin(); }
            inline LightContainers::iterator end() noexcept { return m_Lights.end(); }
            inline LightContainers::const_iterator end() const noexcept { return m_Lights.cend(); }
    };
}

#endif