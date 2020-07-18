#pragma once
#ifndef ENGINE_MATH_SIMPLEX_NOISE_H
#define ENGINE_MATH_SIMPLEX_NOISE_H

#include <core/engine/utils/Utils.h>

namespace Engine{
    namespace priv{
        class SimplexNoise final{
            private:
                double              m_Constants[9];

                glm::ivec2          m_Grad2[8];
                glm::ivec3          m_Grad3[24];
                glm::ivec4          m_Grad4[64];

                std::vector<short>   m_Perm;
                std::vector<short>   m_PermGradIndex3D;

                double internalExtrapolate(const int xsb, const int ysb, const double& dx, const double& dy);
                double internalExtrapolate(const int xsb, const int ysb, const int zsb, const double& dx, const double& dy, const double& dz);
                double internalExtrapolate(const int xsb, const int ysb, const int zsb, const int wsb, const double& dx, const double& dy, const double& dz, const double& dw);

                void internalInitFromSeed(const unsigned long long& seed);
            public:
                SimplexNoise();
                ~SimplexNoise();

                double noiseOpenSimplex2D(const double& x, const double& y);
                double noiseOpenSimplex3D(const double& x, const double& y, const double& z);
                double noiseOpenSimplex4D(const double& x, const double& y, const double& z, const double& w);

                double noiseOpenSimplex2D(const double& x, const double& y, const unsigned long long& seed);
                double noiseOpenSimplex3D(const double& x, const double& y, const double& z, const unsigned long long& seed);
                double noiseOpenSimplex4D(const double& x, const double& y, const double& z, const double& w, const unsigned long long& seed);

        };
    };
    namespace Noise{
        const double noiseOpenSimplex2D(const double& x, const double& y);
        const double noiseOpenSimplex2D(const double& x, const double& y, const unsigned long long& seed);

        const double noiseOpenSimplex3D(const double& x, const double& y, const double& z);
        const double noiseOpenSimplex3D(const double& x, const double& y, const double& z, const unsigned long long& seed);

        const double noiseOpenSimplex4D(const double& x, const double& y, const double& z, const double& w);
        const double noiseOpenSimplex4D(const double& x, const double& y, const double& z, const double& w, const unsigned long long& seed);
    };
};

#endif
