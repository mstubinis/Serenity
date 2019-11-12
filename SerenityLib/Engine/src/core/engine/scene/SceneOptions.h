#pragma once
#ifndef ENGINE_SCENE_OPTIONS_H
#define ENGINE_SCENE_OPTIONS_H

struct SceneOptions final {
    size_t maxAmountOfEntities;
    size_t maxAmountOfParticleEmitters;
    size_t maxAmountOfParticles;
    SceneOptions() {
        #ifdef ENVIRONMENT32
            maxAmountOfEntities = 200000;
            maxAmountOfParticleEmitters = 500;
            maxAmountOfParticles = 250000;
        #else
            maxAmountOfEntities = 400000;
            maxAmountOfParticleEmitters = 1000;
            maxAmountOfParticles = 500000;
        #endif
    }
};

#endif