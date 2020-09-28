#pragma once
#ifndef ENGINE_MATERIAL_ENUMS_H
#define ENGINE_MATERIAL_ENUMS_H

#define MAX_MATERIAL_LAYERS_PER_COMPONENT 4
#define MAX_MATERIAL_COMPONENTS 10
#define MAX_LIGHTS_PER_PASS 16

enum class MaterialLayerBlendMode : unsigned char {
    Default,
    Mix,
    Add,
    Subtract,
    Multiply,
    Divide,
    Screen,
    Overlay,
    Dissolve,
    Dodge,
    Burn,
    HardLight,
    SoftLight,
    GrainExtract,
    GrainMerge,
    Difference,
    Darken,
    Lighten,
    Hue,
    Saturation,
    Color,
    Value,
_TOTAL};
enum class MaterialUVModificationType : unsigned char {
    Translation,
    Rotation,
_TOTAL};
enum class MaterialComponentType : unsigned char {
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
_TOTAL};
enum class MaterialPhysics : unsigned char {
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
_TOTAL};
enum class DiffuseModel : unsigned char {
    None,
    Lambert,
    Oren_Nayar,
    Ashikhmin_Shirley,
    Minnaert,
_TOTAL};
enum class SpecularModel : unsigned char {
    None,
    Blinn_Phong,
    Phong,
    GGX,
    Cook_Torrance,
    Guassian,
    Beckmann,
    Ashikhmin_Shirley,
_TOTAL};

#endif