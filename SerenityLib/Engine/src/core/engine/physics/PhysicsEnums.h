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

#endif