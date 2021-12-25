#pragma once
#ifndef ENGINE_SOUND_EFFECT_H
#define ENGINE_SOUND_EFFECT_H

class  Handle;
class  SoundData;
class  SoundQueue;
namespace Engine::priv {
    class SoundModule;
}

#include <serenity/resources/sound/SoundIncludes.h>
#include <SFML/Audio.hpp>

#include <serenity/lua/LuaIncludes.h>

class SoundEffect {
    friend class Engine::priv::SoundModule;
    friend class SoundQueue;
    private:
        sf::Sound   m_Sound;
        SoundStatus m_Status      = SoundStatus::Fresh;
        uint32_t    m_Loops       = 0U;
        uint32_t    m_CurrentLoop = 0U;
        float       m_Duration    = 0.0f;
        bool        m_Active      = false;
    public:
        SoundEffect();
        SoundEffect(const SoundEffect& other)                = delete;
        SoundEffect& operator=(const SoundEffect& other)     = delete;
        SoundEffect(SoundEffect&& other) noexcept            = default;
        SoundEffect& operator=(SoundEffect&& other) noexcept = default;
        virtual ~SoundEffect()                               = default;

        [[nodiscard]] inline constexpr SoundStatus getStatus() const noexcept { return m_Status; }
        [[nodiscard]] inline constexpr uint32_t getLoopsLeft() const noexcept { return m_Loops - m_CurrentLoop; }
        [[nodiscard]] inline constexpr bool isActive() const noexcept { return m_Active; }
        bool play(unsigned int numLoops = 1);
        bool pause();
        bool stop(bool stopAllLoops = false);
        bool restart();
        [[nodiscard]] inline constexpr float getDuration() const noexcept { return m_Duration; }
        [[nodiscard]] unsigned int getChannelCount() const;
        [[nodiscard]] float getMinDistance() const;
        void setMinDistance(float minDistance);
        [[nodiscard]] bool isRelativeToListener() const;
        void setRelativeToListener(bool relative = true);
        [[nodiscard]] float getAttenuation() const;


        /*
        Set the attenuation factor of the sound. The attenuation is a multiplicative factor which makes
        the sound more or less loud according to its distance from the listener.An attenuation of 0 will
        produce a non - attenuated sound, i.e. its volume will always be the same whether it is heard from
        near or from far. On the other hand, an attenuation value such as 100 will make the sound fade out
        very quickly as it gets further from the listener. The default value of the attenuation is 1.
        */
        void setAttenuation(float attenuation);

        [[nodiscard]] glm::vec3 getPosition() const;

        void setPosition(float x, float y, float z);
        void setPosition(const glm::vec3& position);
        void translate(float x, float y, float z);
        void translate(const glm::vec3& position);

        [[nodiscard]] float getVolume() const;
        void setVolume(float volume);
        [[nodiscard]] float getPitch() const;
        void setPitch(float pitch);

        void update(const float dt);
};

namespace Engine::priv {
    class SoundEffectLUABinder {
        private:
            SoundEffect* m_SoundEffect = nullptr;
        public:
            SoundEffectLUABinder() = default;
            SoundEffectLUABinder(SoundEffect&);


            SoundStatus getStatus() const;
            uint32_t getLoopsLeft() const;
            bool isActive() const;
            bool play(luabridge::LuaRef numLoops); // be aware of default parameter value
            bool pause();
            bool stop(luabridge::LuaRef stopAllLoops); // be aware of default parameter value
            bool restart();
            float getDuration() const;
            unsigned int getChannelCount() const;
            float getMinDistance() const;
            void setMinDistance(float minDistance);
            bool isRelativeToListener() const;
            void setRelativeToListener(luabridge::LuaRef relative); // be aware of default parameter value
            float getAttenuation() const;

            glm::vec3 getPosition() const;
            void setPosition(luabridge::LuaRef x, luabridge::LuaRef y, luabridge::LuaRef z) const;
            void translate(luabridge::LuaRef x, luabridge::LuaRef y, luabridge::LuaRef z) const;

            void setAttenuation(float attenuation);

            float getVolume() const;
            void setVolume(float volume) const;
            float getPitch() const;
            void setPitch(float pitch) const;
    };
}


#endif