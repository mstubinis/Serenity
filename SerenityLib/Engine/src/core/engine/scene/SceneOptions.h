#pragma once
#ifndef ENGINE_SCENE_OPTIONS_H
#define ENGINE_SCENE_OPTIONS_H

struct SceneOptions final {
    size_t maxAmountOfEntities          = 20'000;
    size_t maxAmountOfParticleEmitters  = 500;
    size_t maxAmountOfParticles         = 25'000;
    
    SceneOptions() {
        #ifdef ENVIRONMENT32
            maxAmountOfEntities         = 20'000;
            maxAmountOfParticleEmitters = 500;
            maxAmountOfParticles        = 25'000;
        #else
            maxAmountOfEntities         = 40'000;
            maxAmountOfParticleEmitters = 1'000;
            maxAmountOfParticles        = 50'000;
        #endif
    }
   
    static SceneOptions DEFAULT_OPTIONS;
};

#endif