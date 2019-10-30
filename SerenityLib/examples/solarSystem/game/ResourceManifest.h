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

    static Handle PlanetMesh;
    static Handle DefiantMesh;
    static Handle RingMesh;
    static Handle ShieldMesh;
    static Handle ShieldColMesh;
    static Handle RadarDiscMesh;
    static Handle CannonEffectMesh;
    static Handle CannonEffectOutlineMesh;
    static Handle CannonTailMesh;
    static Handle PhaserBeamMesh;

	/*extras*/
	static Handle NovaMesh;
    static Handle MirandaMesh;
	static Handle VenerexMesh;
	static Handle IntrepidMesh;
    static Handle ExcelsiorMesh;
    static Handle ConstitutionMesh;
    static Handle LeviathanMesh;
    static Handle ShrikeMesh;
    static Handle BrelMesh;
    static Handle BrelMeshHead;
    static Handle BrelMeshWing;
    static Handle BrelMeshWing2;
    static Handle AkiraMesh;
    static Handle NorwayMesh;

	static Handle NovaMaterial;
    static Handle MirandaMaterial;
	static Handle VenerexMaterial;
	static Handle IntrepidMaterial;
    static Handle ExcelsiorMaterial;
    static Handle ConstitutionMaterial;
    static Handle DefiantSharkMaterial;
    static Handle ShrikeMaterial;
    static Handle BrelMaterial;
    static Handle AkiraMaterial;
    static Handle NorwayMaterial;

    //hull damage
    static Handle HullDamageOutline1Material;
    static Handle HullDamageMaterial1;
    static Handle HullDamageOutline2Material;
    static Handle HullDamageMaterial2;
    static Handle HullDamageOutline3Material;
    static Handle HullDamageMaterial3;

    static Handle StarMaterial;
    static Handle EarthSkyMaterial;
    static Handle DefiantMaterial;
    static Handle CrosshairMaterial;
    static Handle CrosshairArrowMaterial;
    static Handle RadarEdgeMaterial;
    static Handle RadarTokenMaterial;
    static Handle RadarMaterial;
    static Handle StarFlareMaterial;
    static Handle ShieldMaterial;
    
    static Handle CannonOutlineMaterial;
    static Handle CannonTailMaterial;
    static Handle PhaserBeamMaterial;
    static Handle PlasmaBeamMaterial;

    //sounds
    static Handle MenuMusic;
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

    //torpedos
    static Handle TorpedoFlareMesh;
    static Handle TorpedoFlareMaterial;
    static Handle TorpedoFlareTriMaterial;
    static Handle TorpedoCoreMaterial;
    static Handle TorpedoGlowMaterial;
    static Handle TorpedoGlow2Material;

    static std::string BasePath;

    static std::unordered_map<std::string, boost::tuple<Handle, Handle, glm::vec3, glm::vec3>> Ships;

    static void init();
};

#endif