#include <core/engine/sounds/Engine_Sounds.h>

#include <core/engine/sounds/SoundData.h>
#include <core/engine/sounds/SoundEffect.h>
#include <core/engine/sounds/SoundMusic.h>
#include <core/engine/sounds/SoundQueue.h>

using namespace Engine;
using namespace std;

epriv::SoundManager* soundManager;


epriv::SoundManager::SoundManager(const char* name,uint w,uint h){ 
    soundManager = this;
}
epriv::SoundManager::~SoundManager(){ 
    SAFE_DELETE_VECTOR(m_SoundQueues);
    SAFE_DELETE_VECTOR(m_CurrentlyPlayingSounds);
}
void epriv::SoundManager::_update(const double& dt){ 
    for (auto it = m_CurrentlyPlayingSounds.begin(); it != m_CurrentlyPlayingSounds.end();) {
        SoundBaseClass& s = *(*it);
        s.update(dt);
        if (s.status() == SoundStatus::Stopped) {
            it = m_CurrentlyPlayingSounds.erase(it);
        }
        else { ++it; }
    }
    for (auto it1 = m_SoundQueues.begin(); it1 != m_SoundQueues.end();) {
        SoundQueue& queue = *(*it1);
        queue.update(dt);
        if (queue.empty()) {
            it1 = m_SoundQueues.erase(it1);
        }
        else { ++it1; }
    }
}

void epriv::SoundManager::_play(SoundBaseClass& sound, const uint& numLoops) {
    sound.play(numLoops);
}
void epriv::SoundManager::_pause(SoundBaseClass& sound) {
    sound.pause();
}
void epriv::SoundManager::_stop(SoundBaseClass& sound) {
    sound.stop();
}
void epriv::SoundManager::_restart(SoundBaseClass& sound){
    sound.restart();
}


SoundQueue* Engine::Sound::createQueue(const float& delay) {
    SoundQueue* queue = new SoundQueue(*soundManager, delay);
    return queue;
}

SoundEffect* Engine::Sound::playEffect(Handle& handle, const uint& loops){
    SoundEffect* effect = new SoundEffect(handle,loops);
    soundManager->m_CurrentlyPlayingSounds.push_back(effect);
    effect->play();
    return effect;
}
SoundMusic* Engine::Sound::playMusic(Handle& handle, const uint& loops){
    SoundMusic* music = new SoundMusic(handle,loops);
    soundManager->m_CurrentlyPlayingSounds.push_back(music);
    music->play();
    return music;
}