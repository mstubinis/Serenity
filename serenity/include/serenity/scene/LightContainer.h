#ifndef ENGINE_SCENE_LIGHT_CONTAINER_H
#define ENGINE_SCENE_LIGHT_CONTAINER_H

#include <serenity/system/Macros.h>

namespace Engine::priv {
    class ILightContainer {
        public:
            virtual ~ILightContainer() = default;
            virtual size_t size() const noexcept = 0;
    };

    template<class LIGHT>
    class LightContainer final : public ILightContainer {
        using ContainerType = std::vector<LIGHT*>;
        private:
            mutable ContainerType  m_Lights;
            mutable ContainerType  m_LightsShadows; //lights that have their shadow casting enabled

            [[nodiscard]] bool internal_is_light_in_container(LIGHT* light, const ContainerType& container) const noexcept {
                for (const LIGHT* itr : container) {
                    if (itr == light) {
                        return true;
                    }
                }
                return false;
            }
        public:
            LightContainer() = default;
            LightContainer(const LightContainer&)                = delete;
            LightContainer& operator=(const LightContainer&)     = delete;
            LightContainer(LightContainer&&) noexcept            = delete;
            LightContainer& operator=(LightContainer&&) noexcept = delete;
            ~LightContainer() {
                SAFE_DELETE_VECTOR(m_Lights);
            }

            template<class ... ARGS> 
            [[nodiscard]] inline Engine::view_ptr<LIGHT> createLight(ARGS&& ... args) {
                return m_Lights.emplace_back(NEW LIGHT{ std::forward<ARGS>(args)... });
            }

            bool deleteLight(LIGHT* light) noexcept {
                if (!light) {
                    return false;
                }
                light->destroy();
                Engine::swap_and_pop_single(m_Lights,        [](auto& itr, LIGHT* light) { return itr == light; }, light);
                Engine::swap_and_pop_single(m_LightsShadows, [](auto& itr, LIGHT* light) { return itr == light; }, light);
                SAFE_DELETE(light);
                return true;
            }
            bool setShadowCaster(LIGHT* light, bool isShadowCaster) {
                if (isShadowCaster) {
                    const bool isAlreadyInContainer = internal_is_light_in_container(light, m_LightsShadows);
                    assert(!isAlreadyInContainer);
                    if (!isAlreadyInContainer) {
                        m_LightsShadows.push_back(light);
                        return true;
                    }
                } else {
                    return Engine::swap_and_pop_single(m_LightsShadows, [](auto& itr, LIGHT* light) { return itr == light; }, light);
                }
                return false;
            }

            [[nodiscard]] size_t size() const noexcept override { return m_Lights.size(); }

            [[nodiscard]] inline LIGHT* operator[](size_t idx) noexcept { return m_Lights[idx]; }
            [[nodiscard]] inline const LIGHT* operator[](size_t idx) const noexcept { return m_Lights[idx]; }

            [[nodiscard]] inline const ContainerType& getShadowCasters() const noexcept { return m_LightsShadows; }

            BUILD_TEMPLATE_BEGIN_END_ITR_CLASS_MEMBERS(ContainerType, m_Lights)
    };
}

#endif