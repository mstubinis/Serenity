#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_EMITTER_H
#define ENGINE_RENDERER_PARTICLE_EMITTER_H

#include <ecs/Entity.h>

class ParticleEmitter final : public EntityWrapper{
    private:


    public:
        ParticleEmitter(Scene* = nullptr);
        ~ParticleEmitter();

        void update(const double& dt);
        void render();
};

#endif