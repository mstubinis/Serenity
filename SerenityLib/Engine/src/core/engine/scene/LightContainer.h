#pragma once
#ifndef ENGINE_SCENE_LIGHT_CONTAINER_H
#define ENGINE_SCENE_LIGHT_CONTAINER_H

namespace Engine::priv {
    class ILightContainer {
        public:
            virtual ~ILightContainer() {}
            virtual size_t size() const noexcept = 0;
    };

    template<class LIGHT>
    class LightContainer final : public ILightContainer {
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

            template<typename ... ARGS> 
            inline Engine::view_ptr<LIGHT> createLight(ARGS&& ... args) { 
                return m_Lights.emplace_back(NEW LIGHT(std::forward<ARGS>(args)...)); 
            }

            void deleteLight(LIGHT* light) noexcept {
                ASSERT(light != nullptr, __FUNCTION__ << "(): light parameter must not be a null pointer!");
                light->destroy();
                std::erase_if(m_Lights, [&light](auto& itr) {
                    return itr == light;
                });
                std::erase_if(m_LightsShadows, [&light](auto& itr) {
                    return itr == light;
                });
                SAFE_DELETE(light);
            }

            void setShadowCaster(LIGHT* light, bool isShadowCaster) {
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

            size_t size() const noexcept override { return m_Lights.size(); }

            inline LIGHT* operator[](size_t idx) noexcept { return m_Lights[idx]; }
            inline const LIGHT* operator[](size_t idx) const noexcept { return m_Lights[idx]; }

            inline const std::vector<LIGHT*>& getShadowCasters() const noexcept { return m_LightsShadows; }

            inline typename std::vector<LIGHT*>::iterator begin() noexcept { return m_Lights.begin(); }
            inline typename std::vector<LIGHT*>::const_iterator begin() const noexcept { return m_Lights.begin(); }
            inline typename std::vector<LIGHT*>::iterator end() noexcept { return m_Lights.end(); }
            inline typename std::vector<LIGHT*>::const_iterator end() const noexcept { return m_Lights.end(); }
    };
}

#endif