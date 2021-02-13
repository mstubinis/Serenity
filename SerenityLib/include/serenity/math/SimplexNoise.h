#pragma once
#ifndef ENGINE_MATH_SIMPLEX_NOISE_H
#define ENGINE_MATH_SIMPLEX_NOISE_H

#include <serenity/utils/Utils.h>
#include <serenity/dependencies/glm.h>
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

                [[nodiscard]] double internalExtrapolate(int xsb, int ysb, double dx, double dy);
                [[nodiscard]] double internalExtrapolate(int xsb, int ysb, int zsb, double dx, double dy, double dz);
                [[nodiscard]] double internalExtrapolate(int xsb, int ysb, int zsb, int wsb, double dx, double dy, double dz, double dw);

                void internalInitFromSeed(unsigned long long seed);
            public:
                SimplexNoise();
                ~SimplexNoise();

                [[nodiscard]] double noiseOpenSimplex2D(double x, double y);
                [[nodiscard]] double noiseOpenSimplex3D(double x, double y, double z);
                [[nodiscard]] double noiseOpenSimplex4D(double x, double y, double z, double w);

                [[nodiscard]] double noiseOpenSimplex2D(double x, double y, unsigned long long seed);
                [[nodiscard]] double noiseOpenSimplex3D(double x, double y, double z, unsigned long long seed);
                [[nodiscard]] double noiseOpenSimplex4D(double x, double y, double z, double w, unsigned long long seed);

        };
    };
    namespace Noise{
        [[nodiscard]] double noiseOpenSimplex2D(double x, double y);
        [[nodiscard]] double noiseOpenSimplex2D(double x, double y, unsigned long long seed);

        [[nodiscard]] double noiseOpenSimplex3D(double x, double y, double z);
        [[nodiscard]] double noiseOpenSimplex3D(double x, double y, double z, unsigned long long seed);

        [[nodiscard]] double noiseOpenSimplex4D(double x, double y, double z, double w);
        [[nodiscard]] double noiseOpenSimplex4D(double x, double y, double z, double w, unsigned long long seed);
    };
};

#endif
