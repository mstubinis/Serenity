#include <core/engine/sounds/Engine_Sounds.h>
#include <core/engine/Engine.h>
#include <core/engine/events/Engine_EventDispatcher.h>

#include <core/engine/sounds/SoundData.h>
#include <core/engine/sounds/SoundEffect.h>
#include <core/engine/sounds/SoundMusic.h>
#include <core/engine/sounds/SoundQueue.h>

using namespace Engine;
using namespace std;

epriv::SoundManager* soundManager;


epriv::SoundManager::SoundManager(const char* name,uint w,uint h){ 
    soundManager = this;
    m_SoundEffects.reserve(SoundManager::MAX_SOUND_EFFECTS);
    m_SoundMusics.reserve(SoundManager::MAX_SOUND_MUSIC);
    for (unsigned int i = 0; i < SoundManager::MAX_SOUND_EFFECTS; ++i) {
        m_SoundEffects.push_back(new SoundEffect());
        m_FreelistEffects.push(i);
    }
    for (unsigned int i = 0; i < SoundManager::MAX_SOUND_MUSIC; ++i) {
        m_SoundMusics.push_back(new SoundMusic());
        m_FreelistMusics.push(i);
    }
}
epriv::SoundManager::~SoundManager(){ 
    SAFE_DELETE_VECTOR(m_SoundQueues);
    SAFE_DELETE_VECTOR(m_SoundEffects);
    SAFE_DELETE_VECTOR(m_SoundMusics);
}
void epriv::SoundManager::_setSoundInformation(Handle& handle, SoundEffect& sound) {
    SoundData& data = *Engine::Resources::getSoundData(handle);
    auto buffer = data.getBuffer();
    if (!buffer) {
        data.buildBuffer();
    }
    sound.m_Duration = data.getDuration();
    sound.m_Sound.setBuffer(*buffer);
    sound.setVolume(data.getVolume());
}
void epriv::SoundManager::_setSoundInformation(Handle& handle, SoundMusic& sound) {
    SoundData& data = *Resources::getSoundData(handle);
    auto buffer = data.getBuffer();
    bool res = sound.m_Sound.openFromFile(data.getFilename());
    if (res) {
        sound.m_Sound.setVolume(data.getVolume());
    }
    sound.m_Duration = data.getDuration();
}

void epriv::SoundManager::_update(const double& dt){ 
    for (auto it1 = m_SoundQueues.begin(); it1 != m_SoundQueues.end();) {
        SoundQueue& queue = *(*it1);
        queue.update(dt);
        if (queue.empty()) {
            it1 = m_SoundQueues.erase(it1);
        }else{
            ++it1;
        }
    }
    for (unsigned int i = 0; i < m_SoundEffects.size(); ++i) {
        auto& effect = *m_SoundEffects[i];
        if (effect.m_Active) {
            effect.update(dt);
            if (effect.status() == SoundStatus::Stopped) {
                m_FreelistEffects.push(i);
                effect.m_Active = false;
            }
        }
    }
    for (unsigned int i = 0; i < m_SoundMusics.size(); ++i) {
        auto& music = *m_SoundMusics[i];
        if (music.m_Active) {
            music.update(dt);
            if (music.status() == SoundStatus::Stopped) {
                m_FreelistMusics.push(i);
                music.m_Active = false;
            }
        }
    }
}

SoundEffect* epriv::SoundManager::_getFreeEffect() {
    if (m_FreelistEffects.size() > 0) {
        auto& index = m_FreelistEffects.top();
        m_FreelistEffects.pop();
        return m_SoundEffects[index];
    }
    return nullptr;
}
SoundMusic* epriv::SoundManager::_getFreeMusic() {
    if (m_FreelistMusics.size() > 0) {
        auto& index = m_FreelistMusics.top();
        m_FreelistMusics.pop();
        return m_SoundMusics[index];
    }
    return nullptr;
}


SoundQueue* Engine::Sound::createQueue(const float& delay) {
    SoundQueue* queue = new SoundQueue(*soundManager, delay);
    return queue;
}

SoundEffect* Engine::Sound::playEffect(Handle& handle, const uint& loops){
    SoundEffect* effect = soundManager->_getFreeEffect();
    if (effect) {
        soundManager->_setSoundInformation(handle, *effect);
        effect->play(loops);
    }
    return effect;
}
SoundMusic* Engine::Sound::playMusic(Handle& handle, const uint& loops){
    SoundMusic* music = soundManager->_getFreeMusic();
    if (music) {
        soundManager->_setSoundInformation(handle, *music);
        music->play(loops);
    }
    return music;
}
void Engine::Sound::stop_all_music() {
    for (auto& music : soundManager->m_SoundMusics) {
        music->stop();
    }
}
void Engine::Sound::stop_all_effect() {
    for (auto& effect : soundManager->m_SoundEffects) {
        effect->stop();
    }
}