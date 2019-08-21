#include "ResourceManifest.h"

#include <core/engine/threading/Engine_ThreadManager.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/materials/Material.h>
#include <core/engine/textures/Texture.h>

using namespace Engine;

Handle ResourceManifest::skyFromSpace;
Handle ResourceManifest::skyFromAtmosphere;
Handle ResourceManifest::groundFromSpace;
Handle ResourceManifest::shieldsShaderProgram;

Handle ResourceManifest::PlanetMesh;
Handle ResourceManifest::DefiantMesh;
Handle ResourceManifest::RingMesh;
Handle ResourceManifest::ShieldMesh;
Handle ResourceManifest::RadarDiscMesh;
Handle ResourceManifest::CannonEffectMesh;
Handle ResourceManifest::CannonEffectOutlineMesh;
Handle ResourceManifest::CannonTailMesh;

/*extras*/
Handle ResourceManifest::NovaMesh;
Handle ResourceManifest::MirandaMesh;
Handle ResourceManifest::VenerexMesh;
Handle ResourceManifest::IntrepidMesh;
Handle ResourceManifest::ExcelsiorMesh;
Handle ResourceManifest::ConstitutionMesh;
Handle ResourceManifest::LeviathanMesh;
Handle ResourceManifest::ShrikeMesh;
Handle ResourceManifest::BrelMesh;

Handle ResourceManifest::NovaMaterial;
Handle ResourceManifest::MirandaMaterial;
Handle ResourceManifest::VenerexMaterial;
Handle ResourceManifest::IntrepidMaterial;
Handle ResourceManifest::ExcelsiorMaterial;
Handle ResourceManifest::ConstitutionMaterial;
Handle ResourceManifest::DefiantSharkMaterial;
Handle ResourceManifest::ShrikeMaterial;
Handle ResourceManifest::BrelMaterial;


Handle ResourceManifest::StarMaterial;
Handle ResourceManifest::EarthSkyMaterial;
Handle ResourceManifest::DefiantMaterial;
Handle ResourceManifest::CrosshairMaterial;
Handle ResourceManifest::CrosshairArrowMaterial;
Handle ResourceManifest::RadarEdgeMaterial;
Handle ResourceManifest::RadarTokenMaterial;
Handle ResourceManifest::RaderMaterial;
Handle ResourceManifest::StarFlareMaterial;
Handle ResourceManifest::ShieldMaterial;

Handle ResourceManifest::CannonOutlineMaterial;
Handle ResourceManifest::CannonTailMaterial;

//sounds
Handle ResourceManifest::MenuMusic;
Handle ResourceManifest::SoundCloakingActivated;
Handle ResourceManifest::SoundCloakingDeactivated;
Handle ResourceManifest::SoundPulsePhaser;
Handle ResourceManifest::SoundPlasmaCannon;
Handle ResourceManifest::SoundDisruptorCannon;

std::unordered_map<std::string, boost::tuple<Handle, Handle, glm::vec3, glm::vec3>> ResourceManifest::Ships;

std::string ResourceManifest::BasePath;



void ResourceManifest::init(){
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

    PlanetMesh = Resources::loadMeshAsync(BasePath + "data/Models/planet.objcc").at(0);
    DefiantMesh = Resources::loadMeshAsync(BasePath + "data/Models/defiant.objcc").at(0); //220 metres long (0.22 km)
    RingMesh = Resources::loadMeshAsync(BasePath + "data/Models/ring.objcc").at(0);
    ShieldMesh = Resources::loadMeshAsync(BasePath + "data/Models/shields.objcc").at(0);
    RadarDiscMesh = Resources::loadMeshAsync(BasePath + "data/Models/radar_disc.objcc").at(0);
    CannonEffectMesh = Resources::loadMeshAsync(BasePath + "data/Models/cannon_bolt.objcc").at(0);
    CannonTailMesh = Resources::loadMeshAsync(BasePath + "data/Models/cannon_tail.objcc").at(0);
    CannonEffectOutlineMesh = Resources::loadMeshAsync(BasePath + "data/Models/cannon_outline.objcc").at(0);


	//extras
	NovaMesh = Resources::loadMeshAsync(BasePath + "data/Models/nova.objcc").at(0);
    MirandaMesh = Resources::loadMeshAsync(BasePath + "data/Models/miranda.objcc").at(0);
	//VenerexMesh = Resources::loadMeshAsync(BasePath + "data/Models/venerex.objcc").at(0);
	//IntrepidMesh = Resources::loadMeshAsync(BasePath + "data/Models/intrepid.objcc").at(0);
    ExcelsiorMesh = Resources::loadMeshAsync(BasePath + "data/Models/excelsior.objcc").at(0);
    ConstitutionMesh = Resources::loadMeshAsync(BasePath + "data/Models/constitution.objcc").at(0);
    //LeviathanMesh = Resources::loadMeshAsync(BasePath + "data/Models/leviathan.objcc").at(0);
    ShrikeMesh = Resources::loadMeshAsync(BasePath + "data/Models/shrike.objcc").at(0);


    Engine::epriv::threading::waitForAll();

    CannonOutlineMaterial = Resources::addMaterial("CannonOutline", BasePath + "data/Textures/Effects/cannon_texture.dds", "","", "");
    Material& cannonOutlineMat = *((Material*)CannonOutlineMaterial.get());
    cannonOutlineMat.setShadeless(true);
    cannonOutlineMat.setGlow(1.0f);

    CannonTailMaterial = Resources::addMaterial("CannonTail", BasePath + "data/Textures/Effects/cannon_texture_tip.dds", "", "", "");
    Material& cannonTailMat = *((Material*)CannonTailMaterial.get());
    cannonTailMat.setShadeless(true);
    cannonTailMat.setGlow(1.0f);


    DefiantMaterial = Resources::addMaterial("Defiant", BasePath + "data/Textures/defiant/defiant.dds");
    Material& defMat = *((Material*)DefiantMaterial.get());
    auto* layer = defMat.getComponent(0).addLayer();
    layer->setTexture(BasePath + "data/Textures/Effects/Buzzards.dds");
    layer->setMask(BasePath + "data/Textures/defiant/defiant_Mask_1.dds");
    layer->addUVModificationSimpleTranslation(0.02f, 0.02f);
    defMat.addComponent(MaterialComponentType::Normal, BasePath + "data/Textures/defiant/defiant_Normal.dds");
    defMat.addComponent(MaterialComponentType::Glow, BasePath + "data/Textures/defiant/defiant_Glow.dds");
    
    
    
    NovaMaterial = Resources::addMaterial("Nova", BasePath + "data/Textures/nova/nova.dds"); 
    Material& novaMat = *((Material*)NovaMaterial.get());
    auto* layer1 = novaMat.getComponent(0).addLayer();
    layer1->setTexture(BasePath + "data/Textures/Effects/Buzzards.dds");
    layer1->setMask(BasePath + "data/Textures/nova/nova_Mask_1.dds");
    layer1->addUVModificationSimpleTranslation(0.02f, 0.02f);
    novaMat.addComponent(MaterialComponentType::Normal, BasePath + "data/Textures/nova/nova_Normal.dds");
    novaMat.addComponent(MaterialComponentType::Glow, BasePath + "data/Textures/nova/nova_Glow.dds");

    MirandaMaterial = Resources::addMaterial("Miranda", BasePath + "data/Textures/miranda/miranda.dds", BasePath + "data/Textures/miranda/miranda_Normal.dds", BasePath + "data/Textures/miranda/miranda_Glow.dds", BasePath + "data/Textures/miranda/miranda_Specular.dds");
	//VenerexMaterial = Resources::addMaterial("Venerex", BasePath + "data/Textures/venerex.dds", BasePath + "data/Textures/venerex_Normal.png", BasePath + "data/Textures/venerex_Glow.png");
	//IntrepidMaterial = Resources::addMaterial("Intrepid", BasePath + "data/Textures/intrepid.dds", BasePath + "data/Textures/intrepid_Normal.png", BasePath + "data/Textures/intrepid_Glow.png");
    ExcelsiorMaterial = Resources::addMaterial("Excelsior", BasePath + "data/Textures/excelsior/excelsior.dds", BasePath + "data/Textures/excelsior/excelsior_Normal.dds", BasePath + "data/Textures/excelsior/excelsior_Glow.dds", BasePath + "data/Textures/excelsior/excelsior_Specular.dds");
    ConstitutionMaterial = Resources::addMaterial("Constitution", BasePath + "data/Textures/constitution/constitution.dds", BasePath + "data/Textures/constitution/constitution_Normal.dds", BasePath + "data/Textures/constitution/constitution_Glow.dds", BasePath + "data/Textures/constitution/constitution_Specular.dds");
    //DefiantSharkMaterial = Resources::addMaterial("DefiantShark", BasePath + "data/Textures/defiant/defiantShark.dds", BasePath + "data/Textures/defiant/defiant_Normal.dds", BasePath + "data/Textures/defiant/defiant_Glow.dds");
    ShrikeMaterial = Resources::addMaterial("Shrike", BasePath + "data/Textures/shrike/shrike.dds", BasePath + "data/Textures/shrike/shrike_Normal.dds", BasePath + "data/Textures/shrike/shrike_Glow.dds", BasePath + "data/Textures/shrike/shrike_Specular.dds");
    Material& shrikeMat = *((Material*)ShrikeMaterial.get());
    shrikeMat.addComponentMetalness(BasePath + "data/Textures/shrike/shrike_Metalness.dds", 1.0f);
    shrikeMat.addComponentSmoothness(BasePath + "data/Textures/shrike/shrike_Smoothness.dds", 1.0f);

    ShieldMaterial = Resources::addMaterial("Shields", BasePath + "data/Textures/Effects/shields_1.dds");
    Material& shieldMat = *((Material*)ShieldMaterial.get());
    shieldMat.setShadeless(true);

    auto& layershield = shieldMat.getComponent(0).layer(0);
    layershield.addUVModificationSimpleTranslation(0.8f, 0.6f);
    layershield.setData2(1.0f, 1.0f, 1.0f, 0.5f);

    auto* layershield1 = shieldMat.getComponent(0).addLayer();
    layershield1->setTexture(BasePath + "data/Textures/Effects/shields_1.dds");
    layershield1->addUVModificationSimpleTranslation(-0.8f, -0.6f);
    layershield1->setData2(1.0f, 1.0f, 1.0f, 0.5f);
    auto* layershield2 = shieldMat.getComponent(0).addLayer();
    layershield2->setTexture(BasePath + "data/Textures/Effects/shields_2.dds");
    layershield2->addUVModificationSimpleTranslation(-0.4f, -0.6f);
    layershield2->setData2(1.0f, 1.0f, 1.0f, 0.4f);
    auto* layershield3 = shieldMat.getComponent(0).addLayer();
    layershield3->setTexture(BasePath + "data/Textures/Effects/shields_3.dds");
    layershield3->addUVModificationSimpleTranslation(0.2f, -0.35f);
    layershield3->setData2(1.0f, 1.0f, 1.0f, 0.4f);


    StarMaterial = Resources::addMaterial("Star", BasePath + "data/Textures/Planets/Sun.dds");
    ((Material*)StarMaterial.get())->setShadeless(true);
    ((Material*)StarMaterial.get())->setGlow(0.21f);

    EarthSkyMaterial = Resources::addMaterial("EarthSky", BasePath + "data/Textures/Planets/Earth.dds");

    

    CrosshairMaterial = Resources::addMaterial("Crosshair", BasePath + "data/Textures/HUD/Crosshair.dds");
    CrosshairArrowMaterial = Resources::addMaterial("CrosshairArrow", BasePath + "data/Textures/HUD/CrosshairArrow.dds");
    RadarEdgeMaterial = Resources::addMaterial("RadarEdge", BasePath + "data/Textures/HUD/RadarEdge.dds");
    RadarTokenMaterial = Resources::addMaterial("RadarToken", BasePath + "data/Textures/HUD/RadarToken.dds");
    RaderMaterial = Resources::addMaterial("Radar", BasePath + "data/Textures/HUD/Radar.dds");


    auto& crosshairDiffuse = *(((Material*)CrosshairMaterial.get())->getComponent(0).texture());
    auto& crosshairArrowDiffuse = *(((Material*)CrosshairArrowMaterial.get())->getComponent(0).texture());

    crosshairDiffuse.setWrapping(TextureWrap::ClampToEdge);
    crosshairDiffuse.setFilter(TextureFilter::Nearest);

    StarFlareMaterial = Resources::addMaterial("SunFlare", BasePath + "data/Textures/Skyboxes/StarFlare.dds");
    Resources::getMaterial(StarFlareMaterial)->setShadeless(true);


    //sounds
    MenuMusic = Resources::addSoundData(BasePath + "data/Sounds/menu.ogg");
    SoundCloakingActivated = Resources::addSoundData(BasePath + "data/Sounds/effects/cloaking.ogg");
    SoundCloakingDeactivated = Resources::addSoundData(BasePath + "data/Sounds/effects/decloaking.ogg");
    SoundPulsePhaser = Resources::addSoundData(BasePath + "data/Sounds/effects/pulse_phaser.ogg");
    SoundPlasmaCannon = Resources::addSoundData(BasePath + "data/Sounds/effects/plasma_cannon.ogg");
    SoundDisruptorCannon = Resources::addSoundData(BasePath + "data/Sounds/effects/disruptor_cannon.ogg");



    ResourceManifest::Ships["Defiant"]      = boost::tuple<Handle, Handle, glm::vec3, glm::vec3>(DefiantMesh,      DefiantMaterial, glm::vec3(1, 1, 1), glm::vec3(0,0,1));
    ResourceManifest::Ships["Nova"]         = boost::tuple<Handle, Handle, glm::vec3, glm::vec3>(NovaMesh,         NovaMaterial, glm::vec3(1, 1, 1), glm::vec3(0, 0, 1));
    ResourceManifest::Ships["Excelsior"]    = boost::tuple<Handle, Handle, glm::vec3, glm::vec3>(ExcelsiorMesh,    ExcelsiorMaterial, glm::vec3(1, 1, 1), glm::vec3(0, 0, 1));
    ResourceManifest::Ships["Miranda"]      = boost::tuple<Handle, Handle, glm::vec3, glm::vec3>(MirandaMesh,      MirandaMaterial, glm::vec3(1, 1, 1), glm::vec3(0, 0, 1));
    ResourceManifest::Ships["Constitution"] = boost::tuple<Handle, Handle, glm::vec3, glm::vec3>(ConstitutionMesh, ConstitutionMaterial, glm::vec3(1, 1, 1), glm::vec3(0, 0, 1));
    ResourceManifest::Ships["Shrike"]       = boost::tuple<Handle, Handle, glm::vec3, glm::vec3>(ShrikeMesh,       ShrikeMaterial, glm::vec3(0.33f, 0.72f, 0.48f), glm::vec3(0, 1, 0));
    ResourceManifest::Ships["Leviathan"]    = boost::tuple<Handle, Handle, glm::vec3, glm::vec3>(LeviathanMesh,    DefiantMaterial, glm::vec3(1, 1, 1), glm::vec3(0, 0, 1));
}