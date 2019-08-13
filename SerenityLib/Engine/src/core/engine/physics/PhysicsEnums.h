#pragma once
#ifndef ENGINE_PHYSICS_ENUMS_H
#define ENGINE_PHYSICS_ENUMS_H

struct CollisionType {enum Type {
    None,
    Sphere,
    Box,
    ConvexHull,
    TriangleShape,
    TriangleShapeStatic,
    Compound,
_TOTAL,};};

//pretty much a copy of bullet physics enums
struct CollisionFlag final {enum Flag {
    None                          =      0,
    StaticObject                  = 1 << 0,
    KinematicObject               = 1 << 1,
    NoContactResponse             = 1 << 2, //no collision based forces i think
    CustomMaterialCallback        = 1 << 3, //this allows per-triangle material (friction/restitution)
    CharacterObject               = 1 << 4,
    DisableVisualizeObject        = 1 << 5, //disable debug drawing
    DisableSPUCollisionProcessing = 1 << 6, //disable parallel/SPU processing
    HasContactStiffnessDamping    = 1 << 7,
    HasCustomDebugRenderingColor  = 1 << 8,
    HasFrictionAnchor             = 1 << 9,
    HasCollisionSoundTrigger      = 1 << 10,
    _Custom_1                     = 1 << 11,
    _Custom_2                     = 1 << 12,
    _Custom_3                     = 1 << 13,
    _Custom_4                     = 1 << 14,
    _Custom_5                     = 1 << 15,
    _Custom_6                     = 1 << 16,
};};

struct CollisionFilter final {enum Filter {
    NoFilter        =      0,
    DefaultFilter   = 1 << 0,
    StaticFilter    = 1 << 1,
    KinematicFilter = 1 << 2,
    DebrisFilter    = 1 << 3,
    SensorTrigger   = 1 << 4,
    CharacterFilter = 1 << 5,
    _Custom_1       = 1 << 6,
    _Custom_2       = 1 << 7,
    _Custom_3       = 1 << 8,
    _Custom_4       = 1 << 9,
    _Custom_5       = 1 << 10,
    AllFilter       = -1 //all bits set
};};

#endif