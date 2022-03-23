#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_INCLUDES_H
#define ENGINE_RENDERER_PARTICLE_INCLUDES_H

#include <cstdint>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#define ENGINE_PARTICLES_HALF_SIZE //use this to reduce particle data size at the cost of precision. this small size uses 16 bytes per particle

#if defined(ENGINE_PARTICLES_HALF_SIZE)
using ParticleFloatType       = uint16_t;
using ParticleIDType          = uint16_t;
using ParticlePackedColorType = uint16_t;
#define PARTICLE_FLOAT_TYPE GL_HALF_FLOAT
#else
using ParticleFloatType       = float;
using ParticleIDType          = uint32_t;
using ParticlePackedColorType = uint32_t;
#define PARTICLE_FLOAT_TYPE GL_FLOAT
#endif

#endif