#pragma once
#ifndef ENGINE_MATH_SIMPLEX_NOISE_H
#define ENGINE_MATH_SIMPLEX_NOISE_H

#include <serenity/core/engine/utils/Utils.h>
#include <serenity/core/engine/dependencies/glm.h>
#include <vector>

namespace Engine{
    namespace priv{
        class SimplexNoise final{
            private:
                double               m_Constants[9];

                glm::ivec2           m_Grad2[8];
                glm::ivec3           m_Grad3[24];
                glm::ivec4           m_Grad4[64];

                std::vector<short>   m_Perm;
                std::vector<short>   m_PermGradIndex3D;

                double internalExtrapolate(int xsb, int ysb, double dx, double dy);
                double internalExtrapolate(int xsb, int ysb, int zsb, double dx, double dy, double dz);
                double internalExtrapolate(int xsb, int ysb, int zsb, int wsb, double dx, double dy, double dz, double dw);

                void internalInitFromSeed(unsigned long long seed);
            public:
                SimplexNoise();
                ~SimplexNoise();

                double noiseOpenSimplex2D(double x, double y);
                double noiseOpenSimplex3D(double x, double y, double z);
                double noiseOpenSimplex4D(double x, double y, double z, double w);

                double noiseOpenSimplex2D(double x, double y, unsigned long long seed);
                double noiseOpenSimplex3D(double x, double y, double z, unsigned long long seed);
                double noiseOpenSimplex4D(double x, double y, double z, double w, unsigned long long seed);

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
