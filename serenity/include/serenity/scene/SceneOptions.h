#pragma once
#ifndef ENGINE_SCENE_OPTIONS_H
#define ENGINE_SCENE_OPTIONS_H

#include <cstdint>

#ifdef ENVIRONMENT64
    constexpr uint32_t NUMBER_OF_ENTITIES_LIMIT          = 40'000;
    constexpr uint32_t NUMBER_OF_PARTICLE_EMITTERS_LIMIT = 1'500U;
    constexpr uint32_t NUMBER_OF_PARTICLE_LIMIT          = 100'000U;
#else
    constexpr uint32_t NUMBER_OF_ENTITIES_LIMIT          = 20'000;
    constexpr uint32_t NUMBER_OF_PARTICLE_EMITTERS_LIMIT = 750U;
    constexpr uint32_t NUMBER_OF_PARTICLE_LIMIT          = 50'000U;
#endif

struct SceneOptions final {
    uint32_t maxAmountOfEntities         = NUMBER_OF_ENTITIES_LIMIT;
    uint32_t maxAmountOfParticleEmitters = NUMBER_OF_PARTICLE_EMITTERS_LIMIT;
    uint32_t maxAmountOfParticles        = NUMBER_OF_PARTICLE_LIMIT;
    
    SceneOptions() = default;
    SceneOptions(uint32_t maxAmountOfEntities_, uint32_t maxAmountOfParticleEmitters_, uint32_t maxAmountOfParticles_)
        : maxAmountOfEntities{ maxAmountOfEntities_ }
        , maxAmountOfParticleEmitters{ maxAmountOfParticleEmitters_ }
        , maxAmountOfParticles{ maxAmountOfParticles_ }
    {}
    ~SceneOptions() = default;

    static SceneOptions DEFAULT_OPTIONS;
};

#endif