#include <core/engine/sounds/SoundModule.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/scene/Camera.h>

#include <core/engine/sounds/SoundData.h>
#include <core/engine/sounds/SoundQueue.h>

using namespace Engine;
using namespace std;

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
    cleanup();
}
void priv::SoundModule::cleanup() {
    SAFE_DELETE_VECTOR(m_SoundQueues);
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
    SoundData& data = *Resources::getSoundData(handle);
    auto* buffer = data.getBuffer();
    if (!buffer) {
        data.buildBuffer();
    }
    sound.m_Duration = data.getDuration();
    sound.m_Sound.setBuffer(*buffer);
    sound.setVolume(data.getVolume());
}
void priv::SoundModule::setSoundInformation(Handle handle, SoundMusic& sound) {
    SoundData& data = *Resources::getSoundData(handle);
    //auto buffer = data.getBuffer();
    bool res = sound.m_Sound.openFromFile(data.getFilename());
    if (res) {
        sound.m_Sound.setVolume(data.getVolume());
    }
    sound.m_Duration = data.getDuration();
}
void priv::SoundModule::updateCameraPosition() {
    auto* scene = Resources::getCurrentScene();
    if (scene) {
        auto* camera = scene->getActiveCamera();
        if (camera) {
            auto camPos     = glm::vec3(camera->getPosition());
            auto camForward = glm::vec3(camera->forward());
            auto camUp      = glm::vec3(camera->up());
            sf::Listener::setPosition(camPos.x, camPos.y, camPos.z);
            sf::Listener::setDirection(camForward.x, camForward.y, camForward.z);
            sf::Listener::setUpVector(camUp.x, camUp.y, camUp.z);
        }
    }
}
void priv::SoundModule::updateSoundQueues(const float dt) {
    for (auto it1 = m_SoundQueues.begin(); it1 != m_SoundQueues.end();) {
        SoundQueue& queue = *(*it1);
        queue.update(dt);
        if (queue.empty()) {
            it1 = m_SoundQueues.erase(it1);
        }else{
            ++it1;
        }
    }
}
void priv::SoundModule::updateSoundEffects(const float dt) {
    for (unsigned int i = 0; i < MAX_SOUND_EFFECTS; ++i) {
        auto& effect = m_SoundEffects[i];
        if (effect.m_Active) {
            effect.update(dt);
            if (effect.status() == SoundStatus::Stopped || effect.status() == SoundStatus::Fresh) {
                m_FreelistEffects.push(i);
                effect.m_Active = false;
            }
        }
    }
}
void priv::SoundModule::updateSoundMusic(const float dt) {
    for (unsigned int i = 0; i < MAX_SOUND_MUSIC; ++i) {
        auto& music = m_SoundMusics[i];
        if (music.m_Active) {
            music.update(dt);
            if (music.status() == SoundStatus::Stopped || music.status() == SoundStatus::Fresh) {
                m_FreelistMusics.push(i);
                music.m_Active = false;
            }
        }
    }
}
void priv::SoundModule::update(const float dt){
    updateCameraPosition();
    updateSoundQueues(dt);
    updateSoundEffects(dt);
    updateSoundMusic(dt);
}

SoundQueue* Sound::createQueue(float delay) {
    SoundQueue* queue = NEW SoundQueue(*soundModule, delay);
    return queue;
}

SoundEffect* Sound::playEffect(Handle handle, unsigned int loops){
    SoundEffect* effect = soundModule->getNextFreeEffect();
    if (effect) {
        soundModule->setSoundInformation(handle, *effect);
        effect->play(loops);
    }
    return effect;
}
SoundMusic* Sound::playMusic(Handle handle, unsigned int loops){
    SoundMusic* music = soundModule->getNextFreeMusic();
    if (music) {
        soundModule->setSoundInformation(handle, *music);
        music->play(loops);
    }
    return music;
}
void Sound::stop_all_effects() {
    for (auto& effect : soundModule->m_SoundEffects) {
        effect.stop();
    }
}
void Sound::stop_all_music() {
    for (auto& music : soundModule->m_SoundMusics) {
        music.stop();
    }
}
std::array<SoundEffect, MAX_SOUND_EFFECTS>& Sound::getAllSoundEffects() {
    return soundModule->m_SoundEffects;
}
std::array<SoundMusic, MAX_SOUND_MUSIC>& Sound::getAllSoundMusics() {
    return soundModule->m_SoundMusics;
}