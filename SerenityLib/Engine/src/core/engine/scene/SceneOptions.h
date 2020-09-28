#pragma once
#ifndef ENGINE_SCENE_OPTIONS_H
#define ENGINE_SCENE_OPTIONS_H

#ifdef ENVIRONMENT64
    constexpr unsigned int NUMBER_OF_ENTITIES_LIMIT          = 40'000;
    constexpr unsigned int NUMBER_OF_PARTICLE_EMITTERS_LIMIT = 1'500U;
    constexpr unsigned int NUMBER_OF_PARTICLE_LIMIT          = 100'000U;
#else
    constexpr unsigned int NUMBER_OF_ENTITIES_LIMIT          = 20'000;
    constexpr unsigned int NUMBER_OF_PARTICLE_EMITTERS_LIMIT = 750U;
    constexpr unsigned int NUMBER_OF_PARTICLE_LIMIT          = 50'000U;
#endif

struct SceneOptions final {
    unsigned int maxAmountOfEntities         = NUMBER_OF_ENTITIES_LIMIT;
    unsigned int maxAmountOfParticleEmitters = NUMBER_OF_PARTICLE_EMITTERS_LIMIT;
    unsigned int maxAmountOfParticles        = NUMBER_OF_PARTICLE_LIMIT;
    
    SceneOptions() = default;
    SceneOptions(unsigned int maxAmountOfEntities_, unsigned int maxAmountOfParticleEmitters_, unsigned int maxAmountOfParticles_)
        : maxAmountOfEntities(maxAmountOfEntities_)
        , maxAmountOfParticleEmitters(maxAmountOfParticleEmitters_)
        , maxAmountOfParticles(maxAmountOfParticles_)
    {}
    ~SceneOptions() = default;

    static SceneOptions DEFAULT_OPTIONS;
};

#endif