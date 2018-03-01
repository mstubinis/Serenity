#pragma once
#ifndef RESOURCE_MANIFEST_H
#define RESOURCE_MANIFEST_H

struct Handle;

class ResourceManifest{
    public:
		static Handle skyFromSpace;
		static Handle skyFromAtmosphere;
		static Handle groundFromSpace;


		static Handle TestMesh;
		static Handle PlanetMesh;
		static Handle DefiantMesh;
		static Handle AkiraMesh;
		static Handle MirandaMesh;
		static Handle IntrepidMesh;
		static Handle NorwayMesh;
		static Handle StarbaseMesh;
		static Handle RingMesh;
		static Handle DreadnaughtMesh;
		static Handle VenerexMesh;

		static Handle StarbaseMaterial;
		static Handle StarMaterial;
		static Handle EarthMaterial;
		static Handle DreadnaughtMaterial;
		static Handle DefiantMaterial;
		static Handle AkiraMaterial;
		static Handle MirandaMaterial;
		static Handle IntrepidMaterial;
		static Handle NorwayMaterial;
		static Handle VenerexMaterial;
		static Handle CrosshairMaterial;
		static Handle CrosshairArrowMaterial;
		static Handle SunFlareMaterial;
		static Handle GoldMaterial;

		static Handle CapsuleTunnelMesh;
		static Handle CapsuleRibbonMesh;
		static Handle CapsuleA;
		static Handle CapsuleB;
		static Handle CapsuleC;
		static Handle CapsuleD;

		static Handle PBCSound;
		static Handle SniperSound;


		static void init();
};

#endif