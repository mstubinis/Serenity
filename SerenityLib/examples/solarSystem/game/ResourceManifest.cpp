#include "ResourceManifest.h"
#include "factions/Faction.h"
#include "ships/Ships.h"

#include <core/engine/threading/Engine_ThreadManager.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/materials/Material.h>
#include <core/engine/textures/Texture.h>

#include <iostream>
#include <SFML/System.hpp>

using namespace Engine;

Handle ResourceManifest::skyFromSpace;
Handle ResourceManifest::skyFromAtmosphere;
Handle ResourceManifest::groundFromSpace;
Handle ResourceManifest::shieldsShaderProgram;

Handle ResourceManifest::PlanetMesh;
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
Handle ResourceManifest::BrelMeshHead;
Handle ResourceManifest::BrelMeshWing;
Handle ResourceManifest::BrelMeshWing2;

Handle ResourceManifest::VenerexMaterial;

//hull damage
Handle ResourceManifest::HullDamageOutline1Material;
Handle ResourceManifest::HullDamageMaterial1;
Handle ResourceManifest::HullDamageOutline2Material;
Handle ResourceManifest::HullDamageMaterial2;
Handle ResourceManifest::HullDamageOutline3Material;
Handle ResourceManifest::HullDamageMaterial3;


Handle ResourceManifest::StarMaterial;
Handle ResourceManifest::EarthSkyMaterial;
Handle ResourceManifest::CrosshairMaterial;
Handle ResourceManifest::CrosshairArrowMaterial;
Handle ResourceManifest::RadarEdgeMaterial;
Handle ResourceManifest::RadarTokenMaterial;
Handle ResourceManifest::RadarMaterial;
Handle ResourceManifest::StarFlareMaterial;
Handle ResourceManifest::ShieldMaterial;

Handle ResourceManifest::CannonOutlineMaterial;
Handle ResourceManifest::CannonTailMaterial;
Handle ResourceManifest::PhaserBeamMaterial;
Handle ResourceManifest::PlasmaBeamMaterial;

//sounds
Handle ResourceManifest::MenuMusic;
Handle ResourceManifest::SoundCloakingActivated;
Handle ResourceManifest::SoundCloakingDeactivated;
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

//torpedos
Handle ResourceManifest::TorpedoFlareMesh;
Handle ResourceManifest::TorpedoFlareMaterial;
Handle ResourceManifest::TorpedoFlareTriMaterial;
Handle ResourceManifest::TorpedoCoreMaterial;
Handle ResourceManifest::TorpedoGlowMaterial;
Handle ResourceManifest::TorpedoGlow2Material;

std::string ResourceManifest::BasePath;

void ResourceManifest::init(){
    Factions::init();
    Ships::init();

    epriv::threading::waitForAll();

    BasePath = "../";
    
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

    PlanetMesh = Resources::loadMeshAsync(BasePath + "data/Models/planet.objcc")[0];
    RingMesh = Resources::loadMeshAsync(BasePath + "data/Models/ring.objcc")[0];
    ShieldMesh = Resources::loadMeshAsync(BasePath + "data/Models/shields.objcc")[0];
    ShieldColMesh = Resources::loadMeshAsync(BasePath + "data/Models/shields_Col.objcc")[0];
    RadarDiscMesh = Resources::loadMeshAsync(BasePath + "data/Models/radar_rings.objcc")[0];
    CannonEffectMesh = Resources::loadMeshAsync(BasePath + "data/Models/cannon_bolt.objcc")[0];
    CannonTailMesh = Resources::loadMeshAsync(BasePath + "data/Models/cannon_tail.objcc")[0];
    CannonEffectOutlineMesh = Resources::loadMeshAsync(BasePath + "data/Models/cannon_outline.objcc")[0];
    PhaserBeamMesh = Resources::loadMeshAsync(BasePath + "data/Models/phaser_beam.objcc")[0];

	//extras
    BrelMeshHead = Resources::loadMeshAsync(BasePath + "data/Ships/brel/brel_head.objcc")[0];
    BrelMeshWing = Resources::loadMeshAsync(BasePath + "data/Ships/brel/brel_wing.objcc")[0];
    BrelMeshWing2 = Resources::loadMeshAsync(BasePath + "data/Ships/brel/brel_wing2.objcc")[0];

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

    ShieldMaterial = Resources::loadMaterialAsync("Shields", BasePath + "data/Textures/Effects/shields_1.dds");
    HullDamageOutline1Material = Resources::loadMaterialAsync("HullDamage1Outline", BasePath + "data/Textures/Effects/hull_dmg_outline_1.dds");
    HullDamageMaterial1 = Resources::loadMaterialAsync("HullDamage1", BasePath + "data/Textures/Effects/hull_dmg.dds");
    HullDamageOutline2Material = Resources::loadMaterialAsync("HullDamage2Outline", BasePath + "data/Textures/Effects/hull_dmg_outline_2.dds");
    HullDamageMaterial2 = Resources::loadMaterialAsync("HullDamage2", BasePath + "data/Textures/Effects/hull_dmg.dds");
    HullDamageOutline3Material = Resources::loadMaterialAsync("HullDamage3Outline", BasePath + "data/Textures/Effects/hull_dmg_outline_3.dds");
    HullDamageMaterial3 = Resources::loadMaterialAsync("HullDamage3", BasePath + "data/Textures/Effects/hull_dmg.dds");
    StarMaterial = Resources::loadMaterialAsync("Star", BasePath + "data/Textures/Planets/Sun.dds");
    EarthSkyMaterial = Resources::loadMaterialAsync("EarthSky", BasePath + "data/Textures/Planets/Earth.dds");
    CrosshairMaterial = Resources::loadMaterialAsync("Crosshair", BasePath + "data/Textures/HUD/Crosshair.dds");
    CrosshairArrowMaterial = Resources::loadMaterialAsync("CrosshairArrow", BasePath + "data/Textures/HUD/CrosshairArrow.dds");
    RadarEdgeMaterial = Resources::loadMaterialAsync("RadarEdge", BasePath + "data/Textures/HUD/RadarEdge.dds");
    RadarTokenMaterial = Resources::loadMaterialAsync("RadarToken", BasePath + "data/Textures/HUD/RadarToken.dds");
    RadarMaterial = Resources::loadMaterialAsync("Radar", BasePath + "data/Textures/HUD/Radar.dds");
    StarFlareMaterial = Resources::loadMaterialAsync("SunFlare", BasePath + "data/Textures/Skyboxes/StarFlare.dds");
    
    epriv::threading::waitForAll();

    Material& phaserBeamMat = *((Material*)PhaserBeamMaterial.get());
    phaserBeamMat.setShadeless(true);
    phaserBeamMat.setGlow(1.0f);
    phaserBeamMat.getComponent(0).layer(0).addUVModificationSimpleTranslation(1.4f, 0.0f);
    phaserBeamMat.getComponent(0).layer(0).setData2(1.0f, 0.25f, 0.0f, 1.0f);
    phaserBeamMat.getComponent(0).layer(0).getTexture()->setFilter(TextureFilter::Nearest);
    auto* phaserLayer = phaserBeamMat.getComponent(0).addLayer();
    phaserLayer->setTexture(BasePath + "data/Textures/Effects/phaser_beam_inside.dds");
    phaserLayer->addUVModificationSimpleTranslation(1.4f, 0.0f);
    phaserLayer->setData2(1.0f, 0.7f, 0.0f, 1.0f);
    phaserLayer->getTexture()->setFilter(TextureFilter::Nearest);

    Material& plasmaBeamMat = *((Material*)PlasmaBeamMaterial.get());
    plasmaBeamMat.setShadeless(true);
    plasmaBeamMat.setGlow(1.0f);
    plasmaBeamMat.getComponent(0).layer(0).addUVModificationSimpleTranslation(1.4f, 0.0f);
    plasmaBeamMat.getComponent(0).layer(0).setData2(0.0f, 0.93f, 0.6f, 1.0f);
    plasmaBeamMat.getComponent(0).layer(0).getTexture()->setFilter(TextureFilter::Nearest);
    auto* plasmaLayer = plasmaBeamMat.getComponent(0).addLayer();
    plasmaLayer->setTexture(BasePath + "data/Textures/Effects/phaser_beam_inside.dds");
    plasmaLayer->addUVModificationSimpleTranslation(1.4f, 0.0f);
    plasmaLayer->setData2(0.53f, 1.0f, 0.73f, 1.0f);
    plasmaLayer->getTexture()->setFilter(TextureFilter::Nearest);




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
    layershield1.addUVModificationSimpleTranslation(-0.8f, -0.6f);
    layershield1.setData2(1.0f, 1.0f, 1.0f, 0.5f);
    auto& layershield2 = *shieldMat.getComponent(0).addLayer();
    layershield2.setTexture(BasePath + "data/Textures/Effects/shields_2.dds");
    layershield2.addUVModificationSimpleTranslation(-0.4f, -0.6f);
    layershield2.setData2(1.0f, 1.0f, 1.0f, 0.4f);
    auto& layershield3 = *shieldMat.getComponent(0).addLayer();
    layershield3.setTexture(BasePath + "data/Textures/Effects/shields_3.dds");
    layershield3.addUVModificationSimpleTranslation(0.2f, -0.35f);
    layershield3.setData2(1.0f, 1.0f, 1.0f, 0.4f);


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

    ((Material*)StarMaterial.get())->setShadeless(true);
    ((Material*)StarMaterial.get())->setGlow(0.21f);

    auto& crosshairDiffuse = *(((Material*)CrosshairMaterial.get())->getComponent(0).texture());
    auto& crosshairArrowDiffuse = *(((Material*)CrosshairArrowMaterial.get())->getComponent(0).texture());

    crosshairDiffuse.setWrapping(TextureWrap::ClampToEdge);
    crosshairDiffuse.setFilter(TextureFilter::Nearest);

    ((Material*)StarFlareMaterial.get())->setShadeless(true);


    //sounds
    MenuMusic = Resources::addSoundData(BasePath + "data/Sounds/menu.ogg");
    SoundCloakingActivated = Resources::addSoundData(BasePath + "data/Sounds/effects/cloaking.ogg");
    SoundCloakingDeactivated = Resources::addSoundData(BasePath + "data/Sounds/effects/decloaking.ogg");
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
}