#pragma once
#ifndef GAME_PARTICLES_FIRE_H
#define GAME_PARTICLES_FIRE_H

#include <core/engine/resources/Handle.h>

class  ParticleEmissionProperties;
class Fire {
    public:
        static Handle SmokeMaterial1;
        static Handle SmokeMaterial2;
        static Handle SmokeMaterial3;

        static ParticleEmissionProperties* Regular;
        static ParticleEmissionProperties* ShortLived;

        static void init();
        static void destruct();
};

#endif