
#include <serenity/resources/sound/SoundModule.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/scene/Scene.h>
#include <serenity/scene/Camera.h>

#include <serenity/resources/sound/SoundData.h>
#include <serenity/resources/sound/SoundQueue.h>

namespace {
    Engine::priv::SoundModule* SOUND_MODULE = nullptr;
}

Engine::priv::SoundModule::SoundModule() {
    m_FreelistEffects.reserve(MAX_SOUND_EFFECTS);
    m_FreelistMusics.reserve(MAX_SOUND_MUSIC);
    for (uint32_t i = 0; i < MAX_SOUND_EFFECTS; ++i) {
        m_FreelistEffects.push_back(i);
    }
    for (uint32_t i = 0; i < MAX_SOUND_MUSIC; ++i) {
        m_FreelistMusics.push_back(i);
    }
    SOUND_MODULE = this;
}
Engine::priv::SoundModule::~SoundModule() {

}
SoundEffect* Engine::priv::SoundModule::getNextFreeEffect() {
    if (m_FreelistEffects.size() > 0){
        const auto index = m_FreelistEffects.back();
        m_FreelistEffects.pop_back();
        return &m_SoundEffects[index];
    }
    return nullptr;
}
SoundMusic* Engine::priv::SoundModule::getNextFreeMusic() {
    if (m_FreelistMusics.size() > 0) {
        const auto index = m_FreelistMusics.back();
        m_FreelistMusics.pop_back();
        return &m_SoundMusics[index];
    }
    return nullptr;
}
void Engine::priv::SoundModule::setSoundInformation(Handle handle, SoundEffect& sound) {
    SoundData& data = *handle.get<SoundData>();
    auto* buffer = data.getBuffer();
    if (!buffer) {
        data.buildBuffer();
    }
    sound.m_Duration = data.getDuration();
    sound.m_Sound.setBuffer(*buffer);
    sound.setVolume(data.getVolume());
}
void Engine::priv::SoundModule::setSoundInformation(Handle handle, SoundMusic& sound) {
    SoundData& data = *handle.get<SoundData>();
    bool res = sound.m_Sound.openFromFile(data.getFilename());
    if (res) {
        sound.m_Sound.setVolume(data.getVolume());
    }
    sound.m_Duration = data.getDuration();
}
void Engine::priv::SoundModule::updateCameraPosition(Scene& scene) {
    auto* camera = scene.getActiveCamera();
    if (camera && !camera->isDestroyed()) {
        auto cameraTransform = camera->getComponent<ComponentTransform>();
        auto camPos     = glm::vec3{ camera->getPosition() };
        auto camForward = camera->getViewVector();
        auto camUp      = cameraTransform->getUp();
        sf::Listener::setPosition(camPos.x, camPos.y, camPos.z);
        sf::Listener::setDirection(camForward.x, camForward.y, camForward.z);
        sf::Listener::setUpVector(camUp.x, camUp.y, camUp.z);
    }
}
void Engine::priv::SoundModule::updateSoundQueues(Scene& scene, const float dt) {
    for (auto& queue : m_SoundQueues) {
        queue->update(dt);
    }
    std::erase_if(m_SoundQueues, [](auto& queue) {
        return queue->empty();
    });
}
void Engine::priv::SoundModule::updateSoundEffects(Scene& scene, const float dt) {
    for (uint32_t i = 0; i < MAX_SOUND_EFFECTS; ++i) {
        auto& soundEffect = m_SoundEffects[i];
        if (soundEffect.m_Active) {
            soundEffect.update(dt);
            if (soundEffect.getStatus() == SoundStatus::Stopped || soundEffect.getStatus() == SoundStatus::Fresh) {
                m_FreelistEffects.push_back(i);
                soundEffect.m_Active = false;
            }
        }
    }
}
void Engine::priv::SoundModule::updateSoundMusic(Scene& scene, const float dt) {
    for (uint32_t i = 0; i < MAX_SOUND_MUSIC; ++i) {
        auto& soundMusic = m_SoundMusics[i];
        if (soundMusic.m_Active) {
            soundMusic.update(dt);
            if (soundMusic.getStatus() == SoundStatus::Stopped || soundMusic.getStatus() == SoundStatus::Fresh) {
                m_FreelistMusics.push_back(i);
                soundMusic.m_Active = false;
            }
        }
    }
}
void Engine::priv::SoundModule::update(Scene& scene, const float dt){
    updateCameraPosition(scene);
    updateSoundQueues(scene, dt);
    updateSoundEffects(scene, dt);
    updateSoundMusic(scene, dt);
}

SoundQueue* Engine::Sound::createQueue(float delay) {
    return NEW SoundQueue(*SOUND_MODULE, delay);
}

SoundEffect* Engine::Sound::playEffect(Handle handle, uint32_t loops){
    SoundEffect* effect = SOUND_MODULE->getNextFreeEffect();
    if (effect) {
        SOUND_MODULE->setSoundInformation(handle, *effect);
        effect->play(loops);
    } else {
        ENGINE_PRODUCTION_LOG(__FUNCTION__ << " - returned a null sound effect for handle: " << handle.toString())
    }
    return effect;
}
SoundMusic* Engine::Sound::playMusic(Handle handle, uint32_t loops){
    SoundMusic* music = SOUND_MODULE->getNextFreeMusic();
    if (music) {
        SOUND_MODULE->setSoundInformation(handle, *music);
        music->play(loops);
    } else {
        ENGINE_PRODUCTION_LOG(__FUNCTION__ << " - returned a null sound music for handle: " << handle.toString())
    }
    return music;
}
void Engine::Sound::stop_all_effects() {
    for (auto& soundEffect : SOUND_MODULE->m_SoundEffects) {
        soundEffect.stop();
    }
}
void Engine::Sound::stop_all_music() {
    for (auto& soundMusic : SOUND_MODULE->m_SoundMusics) {
        soundMusic.stop();
    }
}
std::array<SoundEffect, MAX_SOUND_EFFECTS>& Engine::Sound::getAllSoundEffects() {
    return SOUND_MODULE->m_SoundEffects;
}
std::array<SoundMusic, MAX_SOUND_MUSIC>& Engine::Sound::getAllSoundMusics() {
    return SOUND_MODULE->m_SoundMusics;
}

Engine::priv::SoundEffectLUABinder Engine::lua::sound::playEffect(Handle soundHandle, luabridge::LuaRef numLoops) {
    return Engine::priv::SoundEffectLUABinder{ *Engine::Sound::playEffect(
        soundHandle, 
        numLoops.isNil() ? 1U : numLoops.cast<uint32_t>()
    ) };
}
Engine::priv::SoundMusicLUABinder Engine::lua::sound::playMusic(Handle soundHandle, luabridge::LuaRef numLoops) {
    return Engine::priv::SoundMusicLUABinder{ *Engine::Sound::playMusic(
        soundHandle,
        numLoops.isNil() ? 1U : numLoops.cast<uint32_t>()
    ) };
}