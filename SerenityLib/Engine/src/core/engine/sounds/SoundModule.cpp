#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/sounds/SoundModule.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/scene/Camera.h>

#include <core/engine/sounds/SoundData.h>
#include <core/engine/sounds/SoundQueue.h>

using namespace Engine;

priv::SoundModule* soundModule = nullptr;

priv::SoundModule::SoundModule(){
    soundModule = this;
    for (unsigned int i = 0; i < MAX_SOUND_EFFECTS; ++i) {
        m_FreelistEffects.push(i);
    }
    for (unsigned int i = 0; i < MAX_SOUND_MUSIC; ++i) {
        m_FreelistMusics.push(i);
    }
}
priv::SoundModule::~SoundModule(){

}
SoundEffect* priv::SoundModule::getNextFreeEffect() {
    if (m_FreelistEffects.size() > 0){
        auto index = m_FreelistEffects.top();
        m_FreelistEffects.pop();
        return &m_SoundEffects[index];
    }
    return nullptr;
}
SoundMusic* priv::SoundModule::getNextFreeMusic() {
    if (m_FreelistMusics.size() > 0) {
        auto index = m_FreelistMusics.top();
        m_FreelistMusics.pop();
        return &m_SoundMusics[index];
    }
    return nullptr;
}
void priv::SoundModule::setSoundInformation(Handle handle, SoundEffect& sound) {
    SoundData& data = *handle.get<SoundData>();
    auto* buffer = data.getBuffer();
    if (!buffer) {
        data.buildBuffer();
    }
    sound.m_Duration = data.getDuration();
    sound.m_Sound.setBuffer(*buffer);
    sound.setVolume(data.getVolume());
}
void priv::SoundModule::setSoundInformation(Handle handle, SoundMusic& sound) {
    SoundData& data = *handle.get<SoundData>();
    bool res = sound.m_Sound.openFromFile(data.getFilename());
    if (res) {
        sound.m_Sound.setVolume(data.getVolume());
    }
    sound.m_Duration = data.getDuration();
}
void priv::SoundModule::updateCameraPosition(Scene& scene) {
    auto* camera = scene.getActiveCamera();
    if (camera) {
        auto camPos     = glm::vec3(camera->getPosition());
        auto camForward = glm::vec3(camera->forward());
        auto camUp      = glm::vec3(camera->up());
        sf::Listener::setPosition(camPos.x, camPos.y, camPos.z);
        sf::Listener::setDirection(camForward.x, camForward.y, camForward.z);
        sf::Listener::setUpVector(camUp.x, camUp.y, camUp.z);
    }
}
void priv::SoundModule::updateSoundQueues(Scene& scene, const float dt) {
    for (auto& queue : m_SoundQueues) {
        queue->update(dt);
    }
    std::erase_if(m_SoundQueues, [](auto& queue) {
        return (queue->empty());
    });
}
void priv::SoundModule::updateSoundEffects(Scene& scene, const float dt) {
    for (unsigned int i = 0; i < MAX_SOUND_EFFECTS; ++i) {
        auto& soundEffect = m_SoundEffects[i];
        if (soundEffect.m_Active) {
            soundEffect.update(dt);
            if (soundEffect.status() == SoundStatus::Stopped || soundEffect.status() == SoundStatus::Fresh) {
                m_FreelistEffects.push(i);
                soundEffect.m_Active = false;
            }
        }
    }
}
void priv::SoundModule::updateSoundMusic(Scene& scene, const float dt) {
    for (unsigned int i = 0; i < MAX_SOUND_MUSIC; ++i) {
        auto& soundMusic = m_SoundMusics[i];
        if (soundMusic.m_Active) {
            soundMusic.update(dt);
            if (soundMusic.status() == SoundStatus::Stopped || soundMusic.status() == SoundStatus::Fresh) {
                m_FreelistMusics.push(i);
                soundMusic.m_Active = false;
            }
        }
    }
}
void priv::SoundModule::update(Scene& scene, const float dt){
    updateCameraPosition(scene);
    updateSoundQueues(scene, dt);
    updateSoundEffects(scene, dt);
    updateSoundMusic(scene, dt);
}

SoundQueue* Sound::createQueue(float delay) {
    return NEW SoundQueue(*soundModule, delay);
}

SoundEffect* Sound::playEffect(Handle handle, unsigned int loops){
    SoundEffect* effect = soundModule->getNextFreeEffect();
    if (effect) {
        soundModule->setSoundInformation(handle, *effect);
        effect->play(loops);
    }else{
        ENGINE_PRODUCTION_LOG("Sound::playEffect returned a null sound effect for handle: " << handle.index() << ", " << handle.type() << ", " << handle.version())
    }
    return effect;
}
SoundMusic* Sound::playMusic(Handle handle, unsigned int loops){
    SoundMusic* music = soundModule->getNextFreeMusic();
    if (music) {
        soundModule->setSoundInformation(handle, *music);
        music->play(loops);
    }else{
        ENGINE_PRODUCTION_LOG("Sound::playMusic returned a null sound music for handle: " << handle.index() << ", " << handle.type() << ", " << handle.version())
    }
    return music;
}
void Sound::stop_all_effects() {
    for (auto& soundEffect : soundModule->m_SoundEffects) {
        soundEffect.stop();
    }
}
void Sound::stop_all_music() {
    for (auto& soundMusic : soundModule->m_SoundMusics) {
        soundMusic.stop();
    }
}
std::array<SoundEffect, MAX_SOUND_EFFECTS>& Sound::getAllSoundEffects() {
    return soundModule->m_SoundEffects;
}
std::array<SoundMusic, MAX_SOUND_MUSIC>& Sound::getAllSoundMusics() {
    return soundModule->m_SoundMusics;
}