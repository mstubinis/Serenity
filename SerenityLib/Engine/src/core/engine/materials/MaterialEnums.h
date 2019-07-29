#pragma once
#ifndef ENGINE_MATERIAL_ENUMS_H
#define ENGINE_MATERIAL_ENUMS_H

struct MaterialUVModificationType final { enum Type {
    Translation,
    Rotation,
_TOTAL};};

struct MaterialComponentType final { enum Type {
    Diffuse,
    Normal,
    Glow,
    Specular,
    AO,
    Metalness,
    Smoothness,
    Reflection,
    Refraction,
    ParallaxOcclusion,
_TOTAL};};
struct MaterialPhysics final { enum Physics {
    Water,
    Plastic_Or_Glass_Low,
    Plastic_High,
    Glass_Or_Ruby_High,
    Diamond,
    Iron,
    Copper,
    Gold,
    Aluminium,
    Silver,
    Black_Leather,
    Yellow_Paint_MERL,
    Chromium,
    Red_Plastic_MERL,
    Blue_Rubber_MERL,
    Zinc,
    Car_Paint_Orange,
    Skin,
    Quartz,
    Crystal,
    Alcohol,
    Milk,
    Glass,
    Titanium,
    Platinum,
    Nickel,
_TOTAL};};
struct DiffuseModel final { enum Model {
    None,
    Lambert,
    Oren_Nayar,
    Ashikhmin_Shirley,
    Minnaert,
_TOTAL};};
struct SpecularModel final {enum Model {
    None,
    Blinn_Phong,
    Phong,
    GGX,
    Cook_Torrance,
    Guassian,
    Beckmann,
    Ashikhmin_Shirley,
_TOTAL};};

#endif