#include "ResourceManifest.h"
#include "factions/Faction.h"
#include "ships/Ships.h"
#include "map/MapDatabase.h"
#include "particles/Fire.h"
#include "particles/Sparks.h"
#include "Planet.h"

#include <core/engine/threading/Engine_ThreadManager.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/materials/Material.h>
#include <core/engine/materials/MaterialComponent.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/renderer/particles/ParticleEmissionProperties.h>

#include <iostream>
#include <SFML/System.hpp>

using namespace Engine;

Handle ResourceManifest::skyFromSpace;
Handle ResourceManifest::skyFromAtmosphere;
Handle ResourceManifest::groundFromSpace;
Handle ResourceManifest::shieldsShaderProgram;
Handle ResourceManifest::ShipShaderProgramDeferred;
Handle ResourceManifest::ShipShaderProgramForward;

Handle ResourceManifest::RingMesh;
Handle ResourceManifest::ShieldMesh;
Handle ResourceManifest::ShieldColMesh;
Handle ResourceManifest::RadarDiscMesh;
Handle ResourceManifest::CannonEffectMesh;
Handle ResourceManifest::CannonEffectOutlineMesh;
Handle ResourceManifest::CannonTailMesh;
Handle ResourceManifest::PhaserBeamMesh;

/*extras*/
Handle ResourceManifest::VenerexMesh;

Handle ResourceManifest::VenerexMaterial;

//effects
Handle ResourceManifest::HullDamageOutline1Material;
Handle ResourceManifest::HullDamageMaterial1;
Handle ResourceManifest::HullDamageOutline2Material;
Handle ResourceManifest::HullDamageMaterial2;
Handle ResourceManifest::HullDamageOutline3Material;
Handle ResourceManifest::HullDamageMaterial3;

//gui
Handle ResourceManifest::CrosshairMaterial;
Handle ResourceManifest::CrosshairArrowMaterial;
Handle ResourceManifest::RadarEdgeMaterial;
Handle ResourceManifest::RadarTokenMaterial;
Handle ResourceManifest::RadarMaterial;
Handle ResourceManifest::RadarAntiCloakBarMaterial;
Handle ResourceManifest::RadarAntiCloakBarBackgroundMaterial;
Handle ResourceManifest::ShieldRingHUDTexture;
Handle ResourceManifest::ShieldRingBorderHUDTexture;
Handle ResourceManifest::ShieldRing2HUDTexture;
Handle ResourceManifest::ShieldRingBorder2HUDTexture;
Handle ResourceManifest::ShipStatusBackgroundHUDTexture;
Handle ResourceManifest::ShipStatusBackgroundBorderHUDTexture;

Handle ResourceManifest::GUITextureCorner;
Handle ResourceManifest::GUITextureSide;

Handle ResourceManifest::GUITextureCornerRoundSmall;
Handle ResourceManifest::GUITextureCornerBoxSmall;
Handle ResourceManifest::GUITextureSideSmall;

Handle ResourceManifest::GUIArrowBorder;

Handle ResourceManifest::EarthSkyMaterial;
Handle ResourceManifest::StarFlareMaterial;
Handle ResourceManifest::ShieldMaterial;

Handle ResourceManifest::CannonOutlineMaterial;
Handle ResourceManifest::CannonTailMaterial;
Handle ResourceManifest::PhaserBeamMaterial;
Handle ResourceManifest::PlasmaBeamMaterial;
Handle ResourceManifest::DisruptorBeamMaterial;
Handle ResourceManifest::BorgBeamMaterial;
Handle ResourceManifest::BorgCuttingBeamMaterial;

//sounds
Handle ResourceManifest::SoundRomulanCloakingActivated;
Handle ResourceManifest::SoundRomulanCloakingDeactivated;
Handle ResourceManifest::SoundKlingonCloakingActivated;
Handle ResourceManifest::SoundKlingonCloakingDeactivated;
Handle ResourceManifest::SoundPulsePhaser;
Handle ResourceManifest::SoundPlasmaCannon;
Handle ResourceManifest::SoundDisruptorCannon;
Handle ResourceManifest::SoundPhotonTorpedo;
Handle ResourceManifest::SoundKlingonTorpedo;
Handle ResourceManifest::SoundQuantumTorpedo;
Handle ResourceManifest::SoundPhotonTorpedoOld;
Handle ResourceManifest::SoundPlasmaTorpedo;
Handle ResourceManifest::SoundPhaserBeam;
Handle ResourceManifest::SoundPlasmaBeam;
Handle ResourceManifest::SoundDisruptorBeam;
Handle ResourceManifest::SoundBorgBeam;
Handle ResourceManifest::SoundBorgCuttingBeam;
Handle ResourceManifest::SoundBorgTorpedo;
Handle ResourceManifest::SoundAntiCloakScan;
Handle ResourceManifest::SoundAntiCloakScanDetection;
Handle ResourceManifest::SoundExplosionSmall1;
Handle ResourceManifest::SoundExplosionSmall2;
Handle ResourceManifest::SoundExplosionSmall3;
Handle ResourceManifest::SoundExplosionLarge1;
Handle ResourceManifest::SoundExplosionLarge2;
Handle ResourceManifest::SoundExplosionVeryLarge1;

//torpedos
Handle ResourceManifest::TorpedoFlareMesh;
Handle ResourceManifest::TorpedoFlareMaterial;
Handle ResourceManifest::TorpedoFlareTriMaterial;
Handle ResourceManifest::TorpedoCoreMaterial;
Handle ResourceManifest::TorpedoGlowMaterial;
Handle ResourceManifest::TorpedoGlow2Material;

std::string ResourceManifest::BasePath;

void ResourceManifest::init(){
    BasePath = "../";

    MapDatabase::init();
    Factions::init();
    Ships::init();
    Planets::init();


    priv::threading::waitForAll();

    
    Handle skyFromSpaceVert = Resources::addShader(BasePath + "data/Shaders/AS_skyFromSpace_vert.glsl",ShaderType::Vertex);
    Handle skyFromSpaceFrag = Resources::addShader(BasePath + "data/Shaders/AS_skyFromSpace_frag.glsl",ShaderType::Fragment);
    skyFromSpace = Resources::addShaderProgram("AS_SkyFromSpace",skyFromSpaceVert,skyFromSpaceFrag);

    Handle skyFromAtVert = Resources::addShader(BasePath + "data/Shaders/AS_skyFromAtmosphere_vert.glsl",ShaderType::Vertex);
    Handle skyFromAtFrag = Resources::addShader(BasePath + "data/Shaders/AS_skyFromAtmosphere_frag.glsl",ShaderType::Fragment);
    skyFromAtmosphere = Resources::addShaderProgram("AS_SkyFromAtmosphere",skyFromAtVert,skyFromAtFrag);

    Handle groundFromSpaceVert = Resources::addShader(BasePath + "data/Shaders/AS_groundFromSpace_vert.glsl",ShaderType::Vertex);
    Handle groundFromSpaceFrag = Resources::addShader(BasePath + "data/Shaders/AS_groundFromSpace_frag.glsl",ShaderType::Fragment);
    groundFromSpace = Resources::addShaderProgram("AS_GroundFromSpace",groundFromSpaceVert,groundFromSpaceFrag);
 

    Handle shieldsVert = Resources::addShader(BasePath + "data/Shaders/shields_vert.glsl", ShaderType::Vertex);
    Handle shieldsFrag = Resources::addShader(BasePath + "data/Shaders/shields_frag.glsl", ShaderType::Fragment);
    shieldsShaderProgram = Resources::addShaderProgram("ShieldsShaderProgram", shieldsVert, shieldsFrag);

    Handle ShipShaderProgramDeferredVert = Resources::addShader(BasePath + "data/Shaders/shipDef_vert.glsl", ShaderType::Vertex);
    Handle ShipShaderProgramDeferredFrag = Resources::addShader(BasePath + "data/Shaders/shipDef_frag.glsl", ShaderType::Fragment);
    ShipShaderProgramDeferred = Resources::addShaderProgram("ShipShaderProgramDeferred", ShipShaderProgramDeferredVert, ShipShaderProgramDeferredFrag);

    Handle ShipShaderProgramForwardVert = Resources::addShader(BasePath + "data/Shaders/shipFwd_vert.glsl", ShaderType::Vertex);
    Handle ShipShaderProgramForwardFrag = Resources::addShader(BasePath + "data/Shaders/shipFwd_frag.glsl", ShaderType::Fragment);
    ShipShaderProgramForward = Resources::addShaderProgram("ShipShaderProgramForward", ShipShaderProgramForwardVert, ShipShaderProgramForwardFrag);


    RingMesh = Resources::loadMeshAsync(BasePath + "data/Planets/Models/ring.objcc")[0];
    ShieldMesh = Resources::loadMeshAsync(BasePath + "data/Models/shields.objcc")[0];
    ShieldColMesh = Resources::loadMeshAsync(BasePath + "data/Models/shields_Col.objcc")[0];
    RadarDiscMesh = Resources::loadMeshAsync(BasePath + "data/Models/radar_rings.objcc")[0];
    CannonEffectMesh = Resources::loadMeshAsync(BasePath + "data/Models/cannon_bolt.objcc")[0];
    CannonTailMesh = Resources::loadMeshAsync(BasePath + "data/Models/cannon_tail.objcc")[0];
    CannonEffectOutlineMesh = Resources::loadMeshAsync(BasePath + "data/Models/cannon_outline.objcc")[0];
    PhaserBeamMesh = Resources::loadMeshAsync(BasePath + "data/Models/phaser_beam.objcc")[0];

    //torpedos
    TorpedoFlareMesh        = Resources::loadMeshAsync(BasePath + "data/Models/torpedo_flare.objcc")[0];
    TorpedoFlareMaterial    = Resources::loadMaterialAsync("TorpedoFlare", BasePath + "data/Textures/Effects/torpedo_flare.dds", "", "", "");
    TorpedoFlareTriMaterial = Resources::loadMaterialAsync("TorpedoFlareTri", BasePath + "data/Textures/Effects/torpedo_flare_tri.dds", "", "", "");
    TorpedoCoreMaterial     = Resources::loadMaterialAsync("TorpedoCore", BasePath + "data/Textures/Effects/torpedo_core.dds", "", "", "");
    TorpedoGlowMaterial     = Resources::loadMaterialAsync("TorpedoGlow", BasePath + "data/Textures/Effects/torpedo_outer_glow.dds", "", "", "");
    TorpedoGlow2Material    = Resources::loadMaterialAsync("TorpedoGlow2", BasePath + "data/Textures/Effects/torpedo_outer_glow_2.dds", "", "", "");

    Material& torpedoFlareMat = *((Material*)TorpedoFlareMaterial.get());
    torpedoFlareMat.setShadeless(true);
    torpedoFlareMat.setGlow(1.0f);

    Material& torpedoFlareTriMat = *((Material*)TorpedoFlareTriMaterial.get());
    torpedoFlareTriMat.setShadeless(true);
    torpedoFlareTriMat.setGlow(1.0f);

    Material& torpedoCoreMat = *((Material*)TorpedoCoreMaterial.get());
    torpedoCoreMat.setShadeless(true);
    torpedoCoreMat.setGlow(1.0f);

    Material& torpedoGlowMat = *((Material*)TorpedoGlowMaterial.get());
    torpedoGlowMat.setShadeless(true);
    torpedoGlowMat.setGlow(1.0f);

    Material& torpedoGlowMat2 = *((Material*)TorpedoGlow2Material.get());
    torpedoGlowMat2.setShadeless(true);
    torpedoGlowMat2.setGlow(1.0f);


    CannonOutlineMaterial = Resources::loadMaterialAsync("CannonOutline", BasePath + "data/Textures/Effects/cannon_texture.dds", "","", "");
    CannonTailMaterial = Resources::loadMaterialAsync("CannonTail", BasePath + "data/Textures/Effects/cannon_texture_tip.dds", "", "", "");
    PhaserBeamMaterial = Resources::loadMaterialAsync("PhaserBeam", BasePath + "data/Textures/Effects/phaser_beam_outline.dds", "", "", "");
    PlasmaBeamMaterial = Resources::loadMaterialAsync("PlasmaBeam", BasePath + "data/Textures/Effects/phaser_beam_outline.dds", "", "", "");
    DisruptorBeamMaterial = Resources::loadMaterialAsync("DisruptorBeam", BasePath + "data/Textures/Effects/phaser_beam_outline.dds", "", "", "");
    BorgBeamMaterial = Resources::loadMaterialAsync("BorgBeam", BasePath + "data/Textures/Effects/phaser_beam_outline.dds", "", "", "");
    BorgCuttingBeamMaterial = Resources::loadMaterialAsync("BorgCuttingBeam", BasePath + "data/Textures/Effects/cutting_beam.dds", "", "", "");


    ShieldMaterial = Resources::loadMaterialAsync("Shields", BasePath + "data/Textures/Effects/shields_1.dds");
    HullDamageOutline1Material = Resources::loadMaterialAsync("HullDamage1Outline", BasePath + "data/Textures/Effects/hull_dmg_outline_1.dds");
    HullDamageMaterial1 = Resources::loadMaterialAsync("HullDamage1", BasePath + "data/Textures/Effects/hull_dmg.dds");
    HullDamageOutline2Material = Resources::loadMaterialAsync("HullDamage2Outline", BasePath + "data/Textures/Effects/hull_dmg_outline_2.dds");
    HullDamageMaterial2 = Resources::loadMaterialAsync("HullDamage2", BasePath + "data/Textures/Effects/hull_dmg.dds");
    HullDamageOutline3Material = Resources::loadMaterialAsync("HullDamage3Outline", BasePath + "data/Textures/Effects/hull_dmg_outline_3.dds");
    HullDamageMaterial3 = Resources::loadMaterialAsync("HullDamage3", BasePath + "data/Textures/Effects/hull_dmg.dds");


    StarFlareMaterial = Resources::loadMaterialAsync("SunFlare", BasePath + "data/Textures/Skyboxes/StarFlare.dds");
    EarthSkyMaterial = Resources::loadMaterialAsync("EarthSky", BasePath + "data/Textures/Planets/Earth.dds");
    CrosshairMaterial = Resources::loadMaterialAsync("Crosshair", BasePath + "data/Textures/HUD/Crosshair.dds");
    CrosshairArrowMaterial = Resources::loadMaterialAsync("CrosshairArrow", BasePath + "data/Textures/HUD/CrosshairArrow.dds");
    RadarEdgeMaterial = Resources::loadMaterialAsync("RadarEdge", BasePath + "data/Textures/HUD/RadarEdge.dds");
    RadarTokenMaterial = Resources::loadMaterialAsync("RadarToken", BasePath + "data/Textures/HUD/RadarToken.dds");
    RadarMaterial = Resources::loadMaterialAsync("Radar", BasePath + "data/Textures/HUD/Radar.dds");
    RadarAntiCloakBarMaterial = Resources::loadMaterialAsync("RadarAntiCloakBar", BasePath + "data/Textures/HUD/RadarAntiCloakBar.dds");
    RadarAntiCloakBarBackgroundMaterial = Resources::loadMaterialAsync("RadarAntiCloakBarBackground", BasePath + "data/Textures/HUD/RadarAntiCloakBarBackground.dds");
    
    ((Texture*)RadarMaterial.get())->setFilter(TextureFilter::Nearest);

    priv::threading::waitForAll();


    Material& phaserBeamMat = *((Material*)PhaserBeamMaterial.get());
    phaserBeamMat.setShadeless(true);
    phaserBeamMat.setGlow(1.0f);
    phaserBeamMat.getComponent(0).layer(0).addUVModificationSimpleTranslation(-1.4f, 0.0f);
    phaserBeamMat.getComponent(0).layer(0).setData2(1.0f, 0.25f, 0.0f, 1.0f);
    phaserBeamMat.getComponent(0).layer(0).getTexture()->setFilter(TextureFilter::Nearest);
    auto* phaserLayer = phaserBeamMat.getComponent(0).addLayer();
    phaserLayer->setTexture(BasePath + "data/Textures/Effects/phaser_beam_inside.dds");
    phaserLayer->addUVModificationSimpleTranslation(-1.4f, 0.0f);
    phaserLayer->setData2(1.0f, 0.7f, 0.0f, 1.0f);
    phaserLayer->getTexture()->setFilter(TextureFilter::Nearest);

    Material& plasmaBeamMat = *((Material*)PlasmaBeamMaterial.get());
    plasmaBeamMat.setShadeless(true);
    plasmaBeamMat.setGlow(1.0f);
    plasmaBeamMat.getComponent(0).layer(0).addUVModificationSimpleTranslation(-1.4f, 0.0f);
    plasmaBeamMat.getComponent(0).layer(0).setData2(0.0f, 0.93f, 0.6f, 1.0f);
    plasmaBeamMat.getComponent(0).layer(0).getTexture()->setFilter(TextureFilter::Nearest);
    auto* plasmaLayer = plasmaBeamMat.getComponent(0).addLayer();
    plasmaLayer->setTexture(BasePath + "data/Textures/Effects/phaser_beam_inside.dds");
    plasmaLayer->addUVModificationSimpleTranslation(-1.4f, 0.0f);
    plasmaLayer->setData2(0.53f, 1.0f, 0.73f, 1.0f);
    plasmaLayer->getTexture()->setFilter(TextureFilter::Nearest);


    Material& disBeamMat = *((Material*)DisruptorBeamMaterial.get());
    disBeamMat.setShadeless(true);
    disBeamMat.setGlow(1.0f);
    disBeamMat.getComponent(0).layer(0).addUVModificationSimpleTranslation(-1.4f, 0.0f);
    disBeamMat.getComponent(0).layer(0).setData2(0.15f, 0.969f, 0.192f, 1.0f);
    disBeamMat.getComponent(0).layer(0).getTexture()->setFilter(TextureFilter::Nearest);
    auto* disLayer = disBeamMat.getComponent(0).addLayer();
    disLayer->setTexture(BasePath + "data/Textures/Effects/phaser_beam_inside.dds");
    disLayer->addUVModificationSimpleTranslation(-1.4f, 0.0f);
    disLayer->setData2(0.632f, 1.0f, 0.0f, 1.0f);
    disLayer->getTexture()->setFilter(TextureFilter::Nearest);



    Material& borgBeamMat = *((Material*)BorgBeamMaterial.get());
    borgBeamMat.setShadeless(true);
    borgBeamMat.setGlow(1.0f);
    borgBeamMat.getComponent(0).layer(0).addUVModificationSimpleTranslation(-1.4f, 0.0f);
    borgBeamMat.getComponent(0).layer(0).setData2(0.085f, 0.97f, 0.43f, 1.0f);
    borgBeamMat.getComponent(0).layer(0).getTexture()->setFilter(TextureFilter::Nearest);
    auto* borgBeamLayer = borgBeamMat.getComponent(0).addLayer();
    borgBeamLayer->setTexture(BasePath + "data/Textures/Effects/phaser_beam_inside.dds");
    borgBeamLayer->addUVModificationSimpleTranslation(-1.4f, 0.0f);
    borgBeamLayer->setData2(0.411f, 0.87f, 0.45f, 1.0f);
    borgBeamLayer->getTexture()->setFilter(TextureFilter::Nearest);




    Material& borgCuttingBeamMat = *((Material*)BorgCuttingBeamMaterial.get());
    borgCuttingBeamMat.setShadeless(true);
    borgCuttingBeamMat.setGlow(1.0f);
    borgCuttingBeamMat.getComponent(0).layer(0).addUVModificationSimpleTranslation(0.0f, -1.2f);
    borgCuttingBeamMat.getComponent(0).layer(0).setData2(0.23f, 0.57f, 0.45f, 1.0f);
    borgCuttingBeamMat.getComponent(0).layer(0).getTexture()->setFilter(TextureFilter::Nearest);
    /*
    auto* borgCuttingBeamLayer = borgCuttingBeamMat.getComponent(0).addLayer();
    borgCuttingBeamLayer->setTexture(BasePath + "data/Textures/Effects/phaser_beam_inside.dds");
    borgCuttingBeamLayer->addUVModificationSimpleTranslation(0.0f, 0.2f);
    borgCuttingBeamLayer->setData2(0.411f, 0.87f, 0.45f, 1.0f);
    borgCuttingBeamLayer->getTexture()->setFilter(TextureFilter::Nearest);
    */






    Material& cannonOutlineMat = *((Material*)CannonOutlineMaterial.get());
    cannonOutlineMat.setShadeless(true);
    cannonOutlineMat.setGlow(1.0f);

    Material& cannonTailMat = *((Material*)CannonTailMaterial.get());
    cannonTailMat.setShadeless(true);
    cannonTailMat.setGlow(1.0f);

    Material& shieldMat = *((Material*)ShieldMaterial.get());
    shieldMat.setShadeless(true);

    auto& layershield = shieldMat.getComponent(0).layer(0);
    layershield.addUVModificationSimpleTranslation(0.8f, 0.6f);
    layershield.setData2(1.0f, 1.0f, 1.0f, 0.5f);

    auto& layershield1 = *shieldMat.getComponent(0).addLayer();
    layershield1.setTexture(BasePath + "data/Textures/Effects/shields_1.dds");
    layershield1.addUVModificationSimpleTranslation(-1.6f, -1.2f);
    layershield1.setData2(1.0f, 1.0f, 1.0f, 0.4f);
    auto& layershield2 = *shieldMat.getComponent(0).addLayer();
    layershield2.setTexture(BasePath + "data/Textures/Effects/shields_2.dds");
    layershield2.addUVModificationSimpleTranslation(-0.8f, -1.2f);
    layershield2.setData2(1.0f, 1.0f, 1.0f, 0.7f);
    auto& layershield3 = *shieldMat.getComponent(0).addLayer();
    layershield3.setTexture(BasePath + "data/Textures/Effects/shields_3.dds");
    layershield3.addUVModificationSimpleTranslation(0.6f, -0.7f);
    layershield3.setData2(1.0f, 1.0f, 1.0f, 0.7f);


    Material& hullDamage1Material = *((Material*)HullDamageMaterial1.get());
    auto& hull1Layer1 = hullDamage1Material.getComponent(0).layer(0);
    hull1Layer1.setMask(BasePath + "data/Textures/Effects/hull_dmg_mask_1.dds");
    auto& hull1Layer2 = *hullDamage1Material.getComponent(0).addLayer();
    hull1Layer2.setTexture(BasePath + "data/Textures/Effects/hull_dmg.dds");
    hull1Layer2.setMask(BasePath + "data/Textures/Effects/hull_dmg_mask_1_a.dds");
    hull1Layer2.addUVModificationSimpleTranslation(0.1f, 0.1f);
    hullDamage1Material.addComponent(MaterialComponentType::Glow, BasePath + "data/Textures/Effects/hull_dmg_mask_1.dds");

    Material& hullDamage1Material2 = *((Material*)HullDamageMaterial2.get());
    auto& hull1Layer12 = hullDamage1Material2.getComponent(0).layer(0);
    hull1Layer12.setMask(BasePath + "data/Textures/Effects/hull_dmg_mask_2.dds");
    auto& hull1Layer22 = *hullDamage1Material2.getComponent(0).addLayer();
    hull1Layer22.setTexture(BasePath + "data/Textures/Effects/hull_dmg.dds");
    hull1Layer22.setMask(BasePath + "data/Textures/Effects/hull_dmg_mask_2_a.dds");
    hull1Layer22.addUVModificationSimpleTranslation(0.1f, 0.1f);
    hullDamage1Material2.addComponent(MaterialComponentType::Glow, BasePath + "data/Textures/Effects/hull_dmg_mask_2.dds");

    Material& hullDamage1Material3 = *((Material*)HullDamageMaterial3.get());
    auto& hull1Layer13 = hullDamage1Material3.getComponent(0).layer(0);
    hull1Layer13.setMask(BasePath + "data/Textures/Effects/hull_dmg_mask_3.dds");
    auto& hull1Layer23 = *hullDamage1Material3.getComponent(0).addLayer();
    hull1Layer23.setTexture(BasePath + "data/Textures/Effects/hull_dmg.dds");
    hull1Layer23.setMask(BasePath + "data/Textures/Effects/hull_dmg_mask_3_a.dds");
    hull1Layer23.addUVModificationSimpleTranslation(0.1f, 0.1f);
    hullDamage1Material3.addComponent(MaterialComponentType::Glow, BasePath + "data/Textures/Effects/hull_dmg_mask_3.dds");

    auto& crosshairDiffuse = *(((Material*)CrosshairMaterial.get())->getComponent(0).texture());
    auto& crosshairArrowDiffuse = *(((Material*)CrosshairArrowMaterial.get())->getComponent(0).texture());

    crosshairDiffuse.setWrapping(TextureWrap::ClampToEdge);
    crosshairDiffuse.setFilter(TextureFilter::Nearest);

    ((Material*)StarFlareMaterial.get())->setShadeless(true);


    ShieldRingHUDTexture = Resources::loadTextureAsync(BasePath + "data/Textures/HUD/shield_Ring.dds");
    ShieldRingBorderHUDTexture = Resources::loadTextureAsync(BasePath + "data/Textures/HUD/shield_Ring_Border.dds");
    ShieldRing2HUDTexture = Resources::loadTextureAsync(BasePath + "data/Textures/HUD/shield_Ring_2.dds");
    ShieldRingBorder2HUDTexture = Resources::loadTextureAsync(BasePath + "data/Textures/HUD/shield_Ring_2_Border.dds");
    ShipStatusBackgroundHUDTexture = Resources::loadTextureAsync(BasePath + "data/Textures/HUD/ShipStatusBackground.dds");
    ShipStatusBackgroundBorderHUDTexture = Resources::loadTextureAsync(BasePath + "data/Textures/HUD/ShipStatusBackgroundBorder.dds");

    ((Texture*)ShipStatusBackgroundHUDTexture.get())->setFilter(TextureFilter::Nearest);
    ((Texture*)ShipStatusBackgroundBorderHUDTexture.get())->setFilter(TextureFilter::Nearest);


    GUITextureCorner = Resources::loadTexture(BasePath + "data/Textures/HUD/GUI_Corner.dds");
    GUITextureSide = Resources::loadTexture(BasePath + "data/Textures/HUD/GUI_Side.dds");
    ((Texture*)GUITextureCorner.get())->setFilter(TextureFilter::Nearest);
    ((Texture*)GUITextureSide.get())->setFilter(TextureFilter::Nearest);


    GUITextureCornerRoundSmall = Resources::loadTexture(BasePath + "data/Textures/HUD/GUI_Corner_Round_Small.dds");
    GUITextureCornerBoxSmall = Resources::loadTexture(BasePath + "data/Textures/HUD/GUI_Corner_Box_Small.dds");
    GUITextureSideSmall = Resources::loadTexture(BasePath + "data/Textures/HUD/GUI_Side_Small.dds");
    ((Texture*)GUITextureCornerRoundSmall.get())->setFilter(TextureFilter::Nearest);
    ((Texture*)GUITextureCornerBoxSmall.get())->setFilter(TextureFilter::Nearest);
    ((Texture*)GUITextureSideSmall.get())->setFilter(TextureFilter::Nearest);

    GUIArrowBorder = Resources::loadTexture(BasePath + "data/Textures/HUD/GUI_Arrow_border.dds");

    //sounds
    SoundRomulanCloakingActivated = Resources::addSoundData(BasePath + "data/Sounds/effects/romulan_cloaking.ogg");
    SoundRomulanCloakingDeactivated = Resources::addSoundData(BasePath + "data/Sounds/effects/romulan_decloaking.ogg");
    SoundKlingonCloakingActivated = Resources::addSoundData(BasePath + "data/Sounds/effects/klingon_cloaking.ogg");
    SoundKlingonCloakingDeactivated = Resources::addSoundData(BasePath + "data/Sounds/effects/klingon_decloaking.ogg");

    SoundPulsePhaser = Resources::addSoundData(BasePath + "data/Sounds/effects/pulse_phaser.ogg");
    SoundPlasmaCannon = Resources::addSoundData(BasePath + "data/Sounds/effects/plasma_cannon.ogg");
    SoundDisruptorCannon = Resources::addSoundData(BasePath + "data/Sounds/effects/disruptor_cannon.ogg");
    SoundPhotonTorpedo = Resources::addSoundData(BasePath + "data/Sounds/effects/photon_torpedo.ogg");
    SoundKlingonTorpedo = Resources::addSoundData(BasePath + "data/Sounds/effects/klingon_torpedo.ogg");
    SoundQuantumTorpedo = Resources::addSoundData(BasePath + "data/Sounds/effects/quantum_torpedo.ogg");
    SoundPhotonTorpedoOld = Resources::addSoundData(BasePath + "data/Sounds/effects/photon_torpedo_old.ogg");
    SoundPlasmaTorpedo = Resources::addSoundData(BasePath + "data/Sounds/effects/plasma_torpedo.ogg");
    SoundPhaserBeam = Resources::addSoundData(BasePath + "data/Sounds/effects/phaser_beam.ogg");
    SoundPlasmaBeam = Resources::addSoundData(BasePath + "data/Sounds/effects/plasma_beam.ogg");
    SoundDisruptorBeam = Resources::addSoundData(BasePath + "data/Sounds/effects/disruptor_beam.ogg");


    SoundBorgBeam = Resources::addSoundData(BasePath + "data/Sounds/effects/borg_beam.ogg");
    SoundBorgCuttingBeam = Resources::addSoundData(BasePath + "data/Sounds/effects/borg_cutting_beam.ogg");
    SoundBorgTorpedo = Resources::addSoundData(BasePath + "data/Sounds/effects/borg_torpedo.ogg");

    SoundAntiCloakScan = Resources::addSoundData(BasePath + "data/Sounds/effects/anti_cloak_scan.ogg");
    SoundAntiCloakScanDetection = Resources::addSoundData(BasePath + "data/Sounds/effects/anti_cloak_scan_detection.ogg");
    SoundExplosionSmall1 = Resources::addSoundData(BasePath + "data/Sounds/effects/explosion_small_1.ogg");
    SoundExplosionSmall2 = Resources::addSoundData(BasePath + "data/Sounds/effects/explosion_small_2.ogg");
    SoundExplosionSmall3 = Resources::addSoundData(BasePath + "data/Sounds/effects/explosion_small_3.ogg");
    SoundExplosionLarge1 = Resources::addSoundData(BasePath + "data/Sounds/effects/explosion_large_1.ogg");
    SoundExplosionLarge2 = Resources::addSoundData(BasePath + "data/Sounds/effects/explosion_large_2.ogg");
    SoundExplosionVeryLarge1 = Resources::addSoundData(BasePath + "data/Sounds/effects/explosion_very_large_1.ogg");

    Fire::init();
    Sparks::init();
}
void ResourceManifest::destruct() {
    Fire::destruct();
    Sparks::destruct();
    Factions::destruct();
    Ships::destruct();
}