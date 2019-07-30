#include "ResourceManifest.h"

#include <core/engine/threading/Engine_ThreadManager.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/materials/Material.h>
#include <core/engine/textures/Texture.h>

using namespace Engine;

Handle ResourceManifest::skyFromSpace;
Handle ResourceManifest::skyFromAtmosphere;
Handle ResourceManifest::groundFromSpace;

Handle ResourceManifest::PlanetMesh;
Handle ResourceManifest::DefiantMesh;
Handle ResourceManifest::RingMesh;

/*extras*/
Handle ResourceManifest::NovaMesh;
Handle ResourceManifest::MirandaMesh;
Handle ResourceManifest::VenerexMesh;
Handle ResourceManifest::IntrepidMesh;
Handle ResourceManifest::ExcelsiorMesh;
Handle ResourceManifest::LeviathanMesh;

Handle ResourceManifest::NovaMaterial;
Handle ResourceManifest::MirandaMaterial;
Handle ResourceManifest::VenerexMaterial;
Handle ResourceManifest::IntrepidMaterial;
Handle ResourceManifest::ExcelsiorMaterial;
Handle ResourceManifest::DefiantSharkMaterial;


Handle ResourceManifest::StarMaterial;
Handle ResourceManifest::EarthSkyMaterial;
Handle ResourceManifest::DefiantMaterial;
Handle ResourceManifest::CrosshairMaterial;
Handle ResourceManifest::CrosshairArrowMaterial;
Handle ResourceManifest::StarFlareMaterial;

//music
Handle ResourceManifest::MenuMusic;

std::unordered_map<std::string, boost::tuple<Handle, Handle>> ResourceManifest::Ships;

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
 

    PlanetMesh = Resources::loadMeshAsync(BasePath + "data/Models/planet.objcc").at(0);
    DefiantMesh = Resources::loadMeshAsync(BasePath + "data/Models/defiant.objcc").at(0); //220 metres long (0.22 km)
    RingMesh = Resources::loadMeshAsync(BasePath + "data/Models/ring.objcc").at(0);


	//extras
	NovaMesh = Resources::loadMeshAsync(BasePath + "data/Models/nova.objcc").at(0);
    MirandaMesh = Resources::loadMeshAsync(BasePath + "data/Models/miranda.objcc").at(0);
	//VenerexMesh = Resources::loadMeshAsync(BasePath + "data/Models/venerex.objcc").at(0);
	//IntrepidMesh = Resources::loadMeshAsync(BasePath + "data/Models/intrepid.objcc").at(0);
    ExcelsiorMesh = Resources::loadMeshAsync(BasePath + "data/Models/excelsior.objcc").at(0);
    //LeviathanMesh = Resources::loadMeshAsync(BasePath + "data/Models/leviathan.objcc").at(0);


    Engine::epriv::threading::waitForAll();

    DefiantMaterial = Resources::addMaterial("Defiant", BasePath + "data/Textures/defiant/defiant.dds");
    Material& defMat = *((Material*)DefiantMaterial.get());
    defMat.getComponent(0).layer(0).setMask(BasePath + "data/Textures/defiant/defiant_Mask.dds");
    auto layer = defMat.getComponent(0).addLayer();
    layer->setTexture(BasePath + "data/Textures/Effects/Buzzards.dds");
    layer->setMask(BasePath + "data/Textures/defiant/defiant_Mask_1.dds");
    layer->addUVModificationSimpleTranslation(0.02f, 0.02f);
    defMat.addComponent(MaterialComponentType::Normal, BasePath + "data/Textures/defiant/defiant_Normal.dds");
    defMat.addComponent(MaterialComponentType::Glow, BasePath + "data/Textures/defiant/defiant_Glow.dds");
    
    
    
    NovaMaterial = Resources::addMaterial("Nova", BasePath + "data/Textures/nova/nova.dds"); 
    Material& novaMat = *((Material*)NovaMaterial.get());
    novaMat.getComponent(0).layer(0).setMask(BasePath + "data/Textures/nova/nova_Mask.dds");
    auto layer1 = novaMat.getComponent(0).addLayer();
    layer1->setTexture(BasePath + "data/Textures/Effects/Buzzards.dds");
    layer1->setMask(BasePath + "data/Textures/nova/nova_Mask_1.dds");
    layer1->addUVModificationSimpleTranslation(0.02f, 0.02f);
    novaMat.addComponent(MaterialComponentType::Normal, BasePath + "data/Textures/nova/nova_Normal.dds");
    novaMat.addComponent(MaterialComponentType::Glow, BasePath + "data/Textures/nova/nova_Glow.dds");

    MirandaMaterial = Resources::addMaterial("Miranda", BasePath + "data/Textures/miranda/miranda.dds", BasePath + "data/Textures/miranda/miranda_Normal.dds", BasePath + "data/Textures/miranda/miranda_Glow.dds", BasePath + "data/Textures/miranda/miranda_Specular.dds");
	//VenerexMaterial = Resources::addMaterial("Venerex", BasePath + "data/Textures/venerex.dds", BasePath + "data/Textures/venerex_Normal.png", BasePath + "data/Textures/venerex_Glow.png");
	//IntrepidMaterial = Resources::addMaterial("Intrepid", BasePath + "data/Textures/intrepid.dds", BasePath + "data/Textures/intrepid_Normal.png", BasePath + "data/Textures/intrepid_Glow.png");
    ExcelsiorMaterial = Resources::addMaterial("Excelsior", BasePath + "data/Textures/excelsior/excelsior.dds", BasePath + "data/Textures/excelsior/excelsior_Normal.dds", BasePath + "data/Textures/excelsior/excelsior_Glow.dds", BasePath + "data/Textures/excelsior/excelsior_Specular.dds");
    //DefiantSharkMaterial = Resources::addMaterial("DefiantShark", BasePath + "data/Textures/defiant/defiantShark.dds", BasePath + "data/Textures/defiant/defiant_Normal.dds", BasePath + "data/Textures/defiant/defiant_Glow.dds");


    StarMaterial = Resources::addMaterial("Star", BasePath + "data/Textures/Planets/Sun.dds");
    ((Material*)StarMaterial.get())->setShadeless(true);
    ((Material*)StarMaterial.get())->setGlow(0.21f);

    EarthSkyMaterial = Resources::addMaterial("EarthSky", BasePath + "data/Textures/Planets/Earth.dds");

    

    CrosshairMaterial = Resources::addMaterial("Crosshair", BasePath + "data/Textures/HUD/Crosshair.dds");
    CrosshairArrowMaterial = Resources::addMaterial("CrosshairArrow", BasePath + "data/Textures/HUD/CrosshairArrow.dds");

    auto& crosshairDiffuse = *(((Material*)CrosshairMaterial.get())->getComponent(0).texture());
    auto& crosshairArrowDiffuse = *(((Material*)CrosshairArrowMaterial.get())->getComponent(0).texture());

    crosshairDiffuse.setWrapping(TextureWrap::ClampToEdge);
    crosshairDiffuse.setFilter(TextureFilter::Nearest);

    StarFlareMaterial = Resources::addMaterial("SunFlare", BasePath + "data/Textures/Skyboxes/StarFlare.dds");
    Resources::getMaterial(StarFlareMaterial)->setShadeless(true);


    //sounds
    MenuMusic = Resources::addSoundData(BasePath + "data/Sounds/menu.ogg");


    ResourceManifest::Ships["Defiant"]     = boost::tuple<Handle, Handle>(DefiantMesh,     DefiantMaterial);
    ResourceManifest::Ships["Nova"]        = boost::tuple<Handle, Handle>(NovaMesh,        NovaMaterial);
    ResourceManifest::Ships["Excelsior"]   = boost::tuple<Handle, Handle>(ExcelsiorMesh,   ExcelsiorMaterial);
    ResourceManifest::Ships["Miranda"]     = boost::tuple<Handle, Handle>(MirandaMesh,     MirandaMaterial);
}