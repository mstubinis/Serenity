#pragma once
#ifndef ENGINE_LIGHT_INCLUDES_H
#define ENGINE_LIGHT_INCLUDES_H

struct LightType {enum Type {
    Sun, 
    Point, 
    Directional, 
    Spot, 
    Rod,
_TOTAL};};
struct LightRange {enum Range {
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
_TOTAL};};
struct LightAttenuation {enum Model {
    Constant,
    Distance,
    Distance_Squared,
    Constant_Linear_Exponent,
    Distance_Radius_Squared,
_TOTAL};};

#endif