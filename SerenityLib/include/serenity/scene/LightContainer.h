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
            [[nodiscard]] inline Engine::view_ptr<LIGHT> createLight(ARGS&& ... args) {
                return m_Lights.emplace_back(NEW LIGHT(std::forward<ARGS>(args)...)); 
            }

            bool deleteLight(LIGHT* light) noexcept {
                ASSERT(light != nullptr, __FUNCTION__ << "(): light parameter must not be a null pointer!");
                //if (!light) {
                //    return false;
                //}
                light->destroy();
                auto resultEraseLight           = std::erase_if(m_Lights,        [&light](auto& itr) { return itr == light; });
                auto resultEraseLightShdwCaster = std::erase_if(m_LightsShadows, [&light](auto& itr) { return itr == light; });
                SAFE_DELETE(light);
                return true;
            }

            bool setShadowCaster(LIGHT* light, bool isShadowCaster) {
                if (isShadowCaster){
                    for (const auto itr : m_LightsShadows) {
                        if (itr == light) {
                            return false;
                        }
                    }
                    m_LightsShadows.push_back(light);
                    return true;
                }else{
                    auto result = std::erase_if(m_LightsShadows, [&light](auto& itr) { return itr == light; });
                    return (bool)result;
                }
                return false;
            }

            [[nodiscard]] size_t size() const noexcept override { return m_Lights.size(); }

            [[nodiscard]] inline LIGHT* operator[](size_t idx) noexcept { return m_Lights[idx]; }
            [[nodiscard]] inline const LIGHT* operator[](size_t idx) const noexcept { return m_Lights[idx]; }

            [[nodiscard]] inline const std::vector<LIGHT*>& getShadowCasters() const noexcept { return m_LightsShadows; }

            inline typename std::vector<LIGHT*>::iterator begin() noexcept { return m_Lights.begin(); }
            inline typename std::vector<LIGHT*>::const_iterator begin() const noexcept { return m_Lights.begin(); }
            inline typename std::vector<LIGHT*>::iterator end() noexcept { return m_Lights.end(); }
            inline typename std::vector<LIGHT*>::const_iterator end() const noexcept { return m_Lights.end(); }
    };
}

#endif