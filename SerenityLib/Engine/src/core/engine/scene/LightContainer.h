#pragma once
#ifndef ENGINE_SCENE_LIGHT_CONTAINER_H
#define ENGINE_SCENE_LIGHT_CONTAINER_H

namespace Engine::priv {
    template<class LIGHT>
    class LightContainer final {
        private:
            mutable std::vector<LIGHT*>  m_Lights;
            mutable std::vector<LIGHT*>  m_LightsShadows;

        public:
            LightContainer() = default;
            LightContainer(const LightContainer&)                = delete;
            LightContainer& operator=(const LightContainer&)     = delete;
            LightContainer(LightContainer&&) noexcept            = delete;
            LightContainer& operator=(LightContainer&&) noexcept = delete;
            ~LightContainer() {
                SAFE_DELETE_VECTOR(m_Lights);
            }

            template<class DERIVED, typename ... ARGS> inline Engine::view_ptr<DERIVED> createLight(ARGS&& ... args) {
                return static_cast<DERIVED*>(m_Lights.emplace_back(NEW DERIVED(std::forward<ARGS>(args)...)));
            }

            void delete_light(LIGHT* light) noexcept {
                light->destroy();
                std::erase_if(m_Lights, [&light](auto& itr) {
                    return itr == light;
                });
                std::erase_if(m_LightsShadows, [&light](auto& itr) {
                    return itr == light;
                });
                SAFE_DELETE(light);
            }

            void set_shadow(LIGHT* light, bool isShadowCaster) {
                if (isShadowCaster){
                    for (const auto itr : m_LightsShadows) {
                        if (itr == light) {
                            return;
                        }
                    }
                    m_LightsShadows.push_back(light);
                }else{
                    std::erase_if(m_LightsShadows, [&light](auto& itr) {
                        return itr == light;
                    });
                }
            }

            inline size_t size() const noexcept { return m_Lights.size(); }

            inline LIGHT* operator[](size_t idx) noexcept { return m_Lights[idx]; }

            inline const std::vector<LIGHT*>& getShadowCasters() const noexcept { return m_LightsShadows; }

            inline typename std::vector<LIGHT*>::iterator begin() noexcept { return m_Lights.begin(); }
            inline typename std::vector<LIGHT*>::const_iterator begin() const noexcept { return m_Lights.begin(); }
            inline typename std::vector<LIGHT*>::iterator end() noexcept { return m_Lights.end(); }
            inline typename std::vector<LIGHT*>::const_iterator end() const noexcept { return m_Lights.end(); }
    };
}

#endif