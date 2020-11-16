#pragma once
#ifndef ENGINE_LIGHT_INCLUDES_H
#define ENGINE_LIGHT_INCLUDES_H

enum class LightType : uint32_t {
    Sun, 
    Point, 
    Directional, 
    Spot, 
    Rod,
    Projection,
};
enum class LightRange : uint32_t {
    _7, 
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
    Constant,
    Distance,
    Distance_Squared,
    Constant_Linear_Exponent,
    Distance_Radius_Squared,
};

#endif