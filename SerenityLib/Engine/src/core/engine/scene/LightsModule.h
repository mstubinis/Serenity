#pragma once
#ifndef ENGINE_SCENE_LIGHTS_MODULE_H
#define ENGINE_SCENE_LIGHTS_MODULE_H

#include <core/engine/scene/LightContainer.h>
#include <core/engine/containers/TypeRegistry.h>

namespace Engine::priv {
    class LightsModule final {
        private:
            std::vector<std::unique_ptr<ILightContainer>>  m_Lights;
            Engine::type_registry                          m_LightsRegistry;
        public:

            template<class LIGHT>
            const LightContainer<LIGHT>& getLights() const noexcept {
                using ContainerType  = LightContainer<LIGHT>;
                const uint32_t index = m_LightsRegistry.type_slot_fast<LIGHT>();
                ASSERT(m_LightsRegistry.contains<LIGHT>(), __FUNCTION__ << "(): type: " << typeid(LIGHT).name() << " must be registered first!");
                const ContainerType& container = *static_cast<const ContainerType*>(m_Lights[index].get());
                return container;
            }


            template<class LIGHT> 
            uint32_t registerLightType() {
                const uint32_t index = m_LightsRegistry.type_slot<LIGHT>();
                if (index == m_Lights.size()) {
                    m_Lights.emplace_back(std::make_unique<Engine::priv::LightContainer<LIGHT>>());
                }
                return index;
            }

            template<class LIGHT, typename ... ARGS>
            Engine::view_ptr<LIGHT> createLight(ARGS&&... args) {
                using ContainerType      = LightContainer<LIGHT>;
                const uint32_t index     = m_LightsRegistry.type_slot_fast<LIGHT>();
                ASSERT(m_LightsRegistry.contains<LIGHT>(), __FUNCTION__ << "(): type: " << typeid(LIGHT).name() << " must be registered first!");
                ContainerType* container = static_cast<ContainerType*>(m_Lights[index].get());
                return container->createLight(std::forward<ARGS>(args)...);
            }

            template<class LIGHT>
            bool deleteLight(LIGHT* light) noexcept {
                using ContainerType      = LightContainer<LIGHT>;
                const uint32_t index     = m_LightsRegistry.type_slot_fast<LIGHT>();
                ASSERT(m_LightsRegistry.contains<LIGHT>(), __FUNCTION__ << "(): type: " << typeid(LIGHT).name() << " must be registered first!");
                ContainerType* container = static_cast<ContainerType*>(m_Lights[index].get());
                return container->deleteLight(light);
            }

            template<class LIGHT>
            bool setShadowCaster(LIGHT* light, bool isShadowCaster) {
                using ContainerType      = LightContainer<LIGHT>;
                const uint32_t index     = m_LightsRegistry.type_slot_fast<LIGHT>();
                ASSERT(m_LightsRegistry.contains<LIGHT>(), __FUNCTION__ << "(): type: " << typeid(LIGHT).name() << " must be registered first!");
                ContainerType* container = static_cast<ContainerType*>(m_Lights[index].get());
                return container->setShadowCaster(light, isShadowCaster);
            }


            inline std::vector<std::unique_ptr<ILightContainer>>::iterator begin() noexcept { return m_Lights.begin(); }
            inline std::vector<std::unique_ptr<ILightContainer>>::const_iterator begin() const noexcept { return m_Lights.cbegin(); }
            inline std::vector<std::unique_ptr<ILightContainer>>::iterator end() noexcept { return m_Lights.end(); }
            inline std::vector<std::unique_ptr<ILightContainer>>::const_iterator end() const noexcept { return m_Lights.cend(); }
    };
}

#endif