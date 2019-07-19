#pragma once
#ifndef ENGINE_MATH_SIMPLEX_NOISE_H
#define ENGINE_MATH_SIMPLEX_NOISE_H

#include <memory>
#include <core/engine/Engine_Utils.h>

namespace Engine{
    namespace epriv{
        class NoiseManager final{
            public:
                class impl; std::unique_ptr<impl> m_i;

                NoiseManager(const char* name, const uint& w, const uint& h);
                ~NoiseManager();
        };
    };
    namespace Noise{
        const double noiseOpenSimplex2D(const double x, const double y);
        const double noiseOpenSimplex2D(const double x, const double y, const unsigned long long seed);

        const double noiseOpenSimplex3D(const double x, const double y, const double z);
        const double noiseOpenSimplex3D(const double x, const double y, const double z, const unsigned long long seed);

        const double noiseOpenSimplex4D(const double x, const double y, const double z, const double w);
        const double noiseOpenSimplex4D(const double x, const double y, const double z, const double w, const unsigned long long seed);
    };
};

#endif
