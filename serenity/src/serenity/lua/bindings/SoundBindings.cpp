#include <serenity/lua/bindings/SoundBindings.h>
#include <serenity/lua/LuaIncludes.h>

#include <serenity/resources/sound/SoundModule.h>
#include <serenity/resources/Handle.h>

void Engine::priv::lua::bindings::createBindingsSounds(lua_State* L) {

    luabridge::getGlobalNamespace(L)
#pragma region SoundEffect
        .beginClass<Engine::priv::SoundEffectLUABinder>("SoundEffect")
            .addFunction("getStatus", &Engine::priv::SoundEffectLUABinder::getStatus)
            .addFunction("getLoopsLeft", &Engine::priv::SoundEffectLUABinder::getLoopsLeft)
            .addFunction("isActive", &Engine::priv::SoundEffectLUABinder::isActive)
            .addFunction("play", &Engine::priv::SoundEffectLUABinder::play)
            .addFunction("pause", &Engine::priv::SoundEffectLUABinder::pause)
            .addFunction("stop", &Engine::priv::SoundEffectLUABinder::stop)
            .addFunction("restart", &Engine::priv::SoundEffectLUABinder::restart)
            .addFunction("getDuration", &Engine::priv::SoundEffectLUABinder::getDuration)
            .addFunction("getChannelCount", &Engine::priv::SoundEffectLUABinder::getChannelCount)
            .addFunction("getMinDistance", &Engine::priv::SoundEffectLUABinder::getMinDistance)
            .addFunction("setMinDistance", &Engine::priv::SoundEffectLUABinder::setMinDistance)
            .addFunction("isRelativeToListener", &Engine::priv::SoundEffectLUABinder::isRelativeToListener)
            .addFunction("setRelativeToListener", &Engine::priv::SoundEffectLUABinder::setRelativeToListener)
            .addFunction("getAttenuation", &Engine::priv::SoundEffectLUABinder::getAttenuation)
            .addFunction("setAttenuation", &Engine::priv::SoundEffectLUABinder::setAttenuation)
            .addFunction("getPosition", &Engine::priv::SoundEffectLUABinder::getPosition)
            .addFunction("setPosition", &Engine::priv::SoundEffectLUABinder::setPosition)
            .addFunction("translate", &Engine::priv::SoundEffectLUABinder::translate)
            .addFunction("getVolume", &Engine::priv::SoundEffectLUABinder::getVolume)
            .addFunction("setVolume", &Engine::priv::SoundEffectLUABinder::setVolume)
            .addFunction("getPitch", &Engine::priv::SoundEffectLUABinder::getPitch)
            .addFunction("setPitch", &Engine::priv::SoundEffectLUABinder::setPitch)
        .endClass()
#pragma endregion

#pragma region SoundMusic
        .beginClass<Engine::priv::SoundMusicLUABinder>("SoundMusic")
            .addFunction("getStatus", &Engine::priv::SoundMusicLUABinder::getStatus)
            .addFunction("getLoopsLeft", &Engine::priv::SoundMusicLUABinder::getLoopsLeft)
            .addFunction("isActive", &Engine::priv::SoundMusicLUABinder::isActive)
            .addFunction("play", &Engine::priv::SoundMusicLUABinder::play)
            .addFunction("pause", &Engine::priv::SoundMusicLUABinder::pause)
            .addFunction("stop", &Engine::priv::SoundMusicLUABinder::stop)
            .addFunction("restart", &Engine::priv::SoundMusicLUABinder::restart)
            .addFunction("getDuration", &Engine::priv::SoundMusicLUABinder::getDuration)
            .addFunction("getChannelCount", &Engine::priv::SoundMusicLUABinder::getChannelCount)
            .addFunction("getMinDistance", &Engine::priv::SoundMusicLUABinder::getMinDistance)
            .addFunction("setMinDistance", &Engine::priv::SoundMusicLUABinder::setMinDistance)
            .addFunction("isRelativeToListener", &Engine::priv::SoundMusicLUABinder::isRelativeToListener)
            .addFunction("setRelativeToListener", &Engine::priv::SoundMusicLUABinder::setRelativeToListener)
            .addFunction("getAttenuation", &Engine::priv::SoundMusicLUABinder::getAttenuation)
            .addFunction("setAttenuation", &Engine::priv::SoundMusicLUABinder::setAttenuation)
            .addFunction("getPosition", &Engine::priv::SoundMusicLUABinder::getPosition)
            .addFunction("setPosition", &Engine::priv::SoundMusicLUABinder::setPosition)
            .addFunction("translate", &Engine::priv::SoundMusicLUABinder::translate)
            .addFunction("getVolume", &Engine::priv::SoundMusicLUABinder::getVolume)
            .addFunction("setVolume", &Engine::priv::SoundMusicLUABinder::setVolume)
            .addFunction("getPitch", &Engine::priv::SoundMusicLUABinder::getPitch)
            .addFunction("setPitch", &Engine::priv::SoundMusicLUABinder::setPitch)
        .endClass()
#pragma endregion

        .addFunction("playSoundEffect", &Engine::lua::sound::playEffect)
        .addFunction("playSoundMusic", &Engine::lua::sound::playMusic)
        .addFunction("stopAllSoundEffects", &Engine::Sound::stop_all_effects)
        .addFunction("stopAllSoundMusic", &Engine::Sound::stop_all_music)
    ;
}