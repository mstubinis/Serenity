#pragma once
#ifndef GAME_PARTICLES_FIRE_H
#define GAME_PARTICLES_FIRE_H


class ParticleEmissionProperties;
class Fire {
    public:
        static ParticleEmissionProperties* Regular;
        static ParticleEmissionProperties* ShortLived;

        static void init();
        static void destruct();
};

#endif