#pragma once
#ifndef ENGINE_MATERIAL_ENUMS_H
#define ENGINE_MATERIAL_ENUMS_H

#define MAX_MATERIAL_LAYERS_PER_COMPONENT 5
#define MAX_MATERIAL_COMPONENTS 10
#define MAX_LIGHTS_PER_PASS 16

#include <serenity/system/TypeDefs.h>
#include <serenity/system/Macros.h>

enum class MaterialLayerBlendMode : uint8_t {
    Default = 0,
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
    _TOTAL,
};
enum class MaterialUVModificationType : uint8_t {
    Translation = 0,
    Rotation,
    _TOTAL,
};
class MaterialComponentType { public: 
    enum Type : uint8_t {
        Diffuse = 0,
        Normal,
        Glow,
        Specular,
        Metalness,
        Smoothness,
        AO,
        Reflection,
        Refraction,
        ParallaxOcclusion,
        Empty,
        _TOTAL,
    };
    BUILD_ENUM_CLASS_MEMBERS(MaterialComponentType, Type)
};
class MaterialPhysics { public:
    enum Type : uint8_t {
        Water = 0,
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
        _TOTAL,
    };
    BUILD_ENUM_CLASS_MEMBERS(MaterialPhysics, Type)
};
class DiffuseModel { public:
    enum Type : uint8_t {
        None = 0,
        Lambert,
        Oren_Nayar,
        Ashikhmin_Shirley,
        Minnaert,
        _TOTAL,
    };
    BUILD_ENUM_CLASS_MEMBERS(DiffuseModel, Type)
};
class SpecularModel { public:
    enum Type : uint8_t {
        None = 0,
        Blinn_Phong,
        Phong,
        GGX,
        Cook_Torrance,
        Guassian,
        Beckmann,
        Ashikhmin_Shirley,
        _TOTAL,
    };
    BUILD_ENUM_CLASS_MEMBERS(SpecularModel, Type)
};

#endif