#pragma once
#ifndef GAME_PARTICLES_SPARKS_H
#define GAME_PARTICLES_SPARKS_H

#include <core/engine/resources/Handle.h>

class ParticleEmissionProperties;
class Sparks {
    public:
        static Handle SparksMaterial1;

        static ParticleEmissionProperties Spray;
        static ParticleEmissionProperties Burst;
        static ParticleEmissionProperties ExplosionSparks;

        static void init();
        static void destruct();
    };

#endif#pragma once
