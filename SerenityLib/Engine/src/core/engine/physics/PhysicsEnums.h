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
    None = 0,
    StaticObject = 1 << 0,
    KinematicObject = 1 << 1,
    NoContactResponse = 1 << 2,
    CustomMaterialCallback = 1 << 3,//this allows per-triangle material (friction/restitution)
    CharacterObject = 1 << 4,
    DisableVisualizeObject = 1 << 5, //disable debug drawing
    DisableSPUCollisionProcessing = 1 << 6,//disable parallel/SPU processing
    HasContactStiffnessDamping = 1 << 7,
    HasCustomDebugRenderingColor = 1 << 8,
    HasFrictionAnchor = 1 << 9,
    HasCollisionSoundTrigger = 1 << 10,
};};

struct CollisionFilter final {enum Filter {
    NoFilter             = 0,
    DefaultFilter   = 1 << 0,
    StaticFilter    = 1 << 1,
    KinematicFilter = 1 << 2,
    DebrisFilter    = 1 << 3,
    SensorTrigger   = 1 << 4,
    CharacterFilter = 1 << 5,
    AllFilter       = -1 //all bits set
};};

#endif