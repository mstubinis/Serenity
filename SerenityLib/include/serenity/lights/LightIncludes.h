#pragma once
#ifndef ENGINE_LIGHT_INCLUDES_H
#define ENGINE_LIGHT_INCLUDES_H

#include <serenity/system/TypeDefs.h>

enum class LightType : uint32_t {
    Sun = 0, 
    Point, 
    Directional, 
    Spot, 
    Rod,
    Projection,
};
enum class LightRange : uint32_t {
    _7 = 0, 
    _13, 
    _20, 
    _32, 
    _50, 
    _65, 
    _100, 
    _160, 
    _200, 
    _325, 
    _600, 
    _3250,
_TOTAL};
enum class LightAttenuation : uint32_t {
    Constant = 0,
    Distance,
    Distance_Squared,
    Constant_Linear_Exponent,
    Distance_Radius_Squared,
};

#endif