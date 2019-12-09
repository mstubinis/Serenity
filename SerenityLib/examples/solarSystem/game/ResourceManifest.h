#pragma once
#ifndef GAME_RESOURCE_MANIFEST_H
#define GAME_RESOURCE_MANIFEST_H

#include <string>
#include <boost/tuple/tuple.hpp>
#include <core/engine/resources/Handle.h>
#include <unordered_map>
#include <glm/vec3.hpp>

struct ResourceManifest{
    static Handle skyFromSpace;
    static Handle skyFromAtmosphere;
    static Handle groundFromSpace;
    static Handle shieldsShaderProgram;
    static Handle ShipShaderProgramDeferred;
    static Handle ShipShaderProgramForward;

    static Handle RingMesh;
    static Handle ShieldMesh;
    static Handle ShieldColMesh;
    static Handle RadarDiscMesh;
    static Handle CannonEffectMesh;
    static Handle CannonEffectOutlineMesh;
    static Handle CannonTailMesh;
    static Handle PhaserBeamMesh;

	/*extras*/
	static Handle VenerexMesh;
    static Handle BrelMeshHead;
    static Handle BrelMeshWing;
    static Handle BrelMeshWing2;

	static Handle VenerexMaterial;

    //gui
    static Handle CrosshairMaterial;
    static Handle CrosshairArrowMaterial;
    static Handle RadarEdgeMaterial;
    static Handle RadarTokenMaterial;
    static Handle RadarMaterial;
    static Handle RadarAntiCloakBarMaterial;
    static Handle RadarAntiCloakBarBackgroundMaterial;
    static Handle ShieldRingHUDTexture;
    static Handle ShieldRingBorderHUDTexture;
    static Handle ShieldRing2HUDTexture;
    static Handle ShieldRingBorder2HUDTexture;
    static Handle ShipStatusBackgroundHUDTexture;
    static Handle ShipStatusBackgroundBorderHUDTexture;

    static Handle GUITextureCorner;
    static Handle GUITextureSide;

    //effects
    static Handle HullDamageOutline1Material;
    static Handle HullDamageMaterial1;
    static Handle HullDamageOutline2Material;
    static Handle HullDamageMaterial2;
    static Handle HullDamageOutline3Material;
    static Handle HullDamageMaterial3;

    static Handle EarthSkyMaterial;
    static Handle StarFlareMaterial;
    static Handle ShieldMaterial;

    static Handle CannonOutlineMaterial;
    static Handle CannonTailMaterial;
    static Handle PhaserBeamMaterial;
    static Handle PlasmaBeamMaterial;
    static Handle DisruptorBeamMaterial;
    static Handle BorgBeamMaterial;
    static Handle BorgCuttingBeamMaterial;

    //sounds
    static Handle SoundCloakingActivated;
    static Handle SoundCloakingDeactivated;
    static Handle SoundPulsePhaser;
    static Handle SoundPlasmaCannon;
    static Handle SoundDisruptorCannon;
    static Handle SoundPhotonTorpedo;
    static Handle SoundKlingonTorpedo;
    static Handle SoundQuantumTorpedo;
    static Handle SoundPhotonTorpedoOld;
    static Handle SoundPlasmaTorpedo;
    static Handle SoundPhaserBeam;
    static Handle SoundPlasmaBeam;
    static Handle SoundDisruptorBeam;
    static Handle SoundBorgBeam;
    static Handle SoundBorgCuttingBeam;
    static Handle SoundBorgTorpedo;
    static Handle SoundAntiCloakScan;
    static Handle SoundAntiCloakScanDetection;
    static Handle SoundExplosionSmall1;
    static Handle SoundExplosionSmall2;
    static Handle SoundExplosionSmall3;
    static Handle SoundExplosionLarge1;
    static Handle SoundExplosionLarge2;
    static Handle SoundExplosionVeryLarge1;


    //torpedos
    static Handle TorpedoFlareMesh;
    static Handle TorpedoFlareMaterial;
    static Handle TorpedoFlareTriMaterial;
    static Handle TorpedoCoreMaterial;
    static Handle TorpedoGlowMaterial;
    static Handle TorpedoGlow2Material;

    static std::string BasePath;

    static void init();
    static void destruct();
};

#endif