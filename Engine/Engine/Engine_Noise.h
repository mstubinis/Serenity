#pragma once
#ifndef ENGINE_NOISE_H
#define ENGINE_NOISE_H

#include <memory>
#include "Engine_Utils.h"

typedef unsigned int uint;

namespace Engine{
    namespace epriv{
        class NoiseManager final: private Engine::epriv::noncopyable{
            public:
                class impl; std::unique_ptr<impl> m_i;

                NoiseManager(const char* name,uint w,uint h);
                ~NoiseManager();
        };
    };
    namespace Noise{
        double noiseOpenSimplex2D(double x, double y);
        double noiseOpenSimplex2D(double x, double y, unsigned long long seed);

        double noiseOpenSimplex3D(double x, double y, double z);
        double noiseOpenSimplex3D(double x, double y, double z, unsigned long long seed);

        double noiseOpenSimplex4D(double x, double y, double z, double w);
        double noiseOpenSimplex4D(double x, double y, double z, double w, unsigned long long seed);
    };
};

#endif
