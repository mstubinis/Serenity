#pragma once
#ifndef ENGINE_NOISE_H
#define ENGINE_NOISE_H

#include <glm/glm.hpp>
#include <vector>

typedef unsigned int uint;

namespace Engine{
    namespace Math{
        namespace Noise{
            namespace Detail{
                class MathNoiseManagement{
                    public:
                        const static double STRETCH_CONSTANT_2D;    //(1/Math.sqrt(2+1)-1)/2;
                        const static double SQUISH_CONSTANT_2D;     //(Math.sqrt(2+1)-1)/2;
                        const static double STRETCH_CONSTANT_3D;    //(1/Math.sqrt(3+1)-1)/3;
                        const static double SQUISH_CONSTANT_3D;     //(Math.sqrt(3+1)-1)/3;
                        const static double STRETCH_CONSTANT_4D;    //(1/Math.sqrt(4+1)-1)/4;
                        const static double SQUISH_CONSTANT_4D;     //(Math.sqrt(4+1)-1)/4;

                        const static double NORM_CONSTANT_2D;
                        const static double NORM_CONSTANT_3D;
                        const static double NORM_CONSTANT_4D;

                        static std::vector<short> perm;
                        static std::vector<short> permGradIndex3D;

                        static std::vector<glm::ivec2> grad2;
                        static std::vector<glm::ivec3> grad3;
                        static std::vector<glm::ivec4> grad4;

                        static void _initFromSeed(unsigned long long seed);
                };
            };
            namespace _2D{
                double noiseOpenSimplex2D(double x, double y);
                double noiseOpenSimplex2D(double x, double y, unsigned long long seed);
            };
            namespace _3D{
                double noiseOpenSimplex3D(double x, double y, double z);
                double noiseOpenSimplex3D(double x, double y, double z, unsigned long long seed);
            };
            namespace _4D{
                double noiseOpenSimplex4D(double x, double y, double z, double w);
                double noiseOpenSimplex4D(double x, double y, double z, double w, unsigned long long seed);
            };
            double extrapolate(int xsb, int ysb, double dx, double dy);
            double extrapolate(int xsb, int ysb, int zsb, double dx, double dy, double dz);
            double extrapolate(int xsb, int ysb, int zsb, int wsb, double dx, double dy, double dz, double dw);
        };
    };
};

#endif
