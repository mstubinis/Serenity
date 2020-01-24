#pragma once
#ifndef GAME_RESOURCE_MANIFEST_H
#define GAME_RESOURCE_MANIFEST_H

#include <string>

struct Handle;
struct ResourceManifest{
    static Handle DreadnaughtMesh;
    static Handle DreadnaughtMaterial;
    static Handle CrosshairMaterial;
    static Handle CrosshairArrowMaterial;
    static Handle StarFlareMaterial;
    static Handle CapsuleTunnelMesh;
    static Handle CapsuleRibbonMeshA;
    static Handle CapsuleRibbonMeshB;
    static Handle CapsuleA;
    static Handle CapsuleB;
    static Handle CapsuleC;
    static Handle CapsuleD;
    static std::string BasePath;
    static void init();
};
#endif