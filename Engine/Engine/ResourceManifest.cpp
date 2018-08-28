#include "ResourceManifest.h"
#include "Engine_ThreadManager.h"
#include "Engine_Resources.h"
#include "Material.h"

using namespace Engine;

Handle ResourceManifest::skyFromSpace;
Handle ResourceManifest::skyFromAtmosphere;
Handle ResourceManifest::groundFromSpace;

Handle ResourceManifest::TestMesh;
Handle ResourceManifest::PlanetMesh;
Handle ResourceManifest::DefiantMesh;
Handle ResourceManifest::AkiraMesh;
Handle ResourceManifest::MirandaMesh;
Handle ResourceManifest::IntrepidMesh;
Handle ResourceManifest::NorwayMesh;
Handle ResourceManifest::StarbaseMesh;
Handle ResourceManifest::RingMesh;
Handle ResourceManifest::DreadnaughtMesh;
Handle ResourceManifest::VenerexMesh;
Handle ResourceManifest::OberthMesh;

Handle ResourceManifest::StarbaseMaterial;
Handle ResourceManifest::StarMaterial;
Handle ResourceManifest::EarthSkyMaterial;
Handle ResourceManifest::DreadnaughtMaterial;
Handle ResourceManifest::DefiantMaterial;
Handle ResourceManifest::AkiraMaterial;
Handle ResourceManifest::MirandaMaterial;
Handle ResourceManifest::IntrepidMaterial;
Handle ResourceManifest::NorwayMaterial;
Handle ResourceManifest::VenerexMaterial;
Handle ResourceManifest::OberthMaterial;
Handle ResourceManifest::CrosshairMaterial;
Handle ResourceManifest::CrosshairArrowMaterial;
Handle ResourceManifest::StarFlareMaterial;
Handle ResourceManifest::GoldMaterial;

Handle ResourceManifest::CapsuleTunnelMesh;
Handle ResourceManifest::CapsuleRibbonMesh;
Handle ResourceManifest::CapsuleA;
Handle ResourceManifest::CapsuleB;
Handle ResourceManifest::CapsuleC;
Handle ResourceManifest::CapsuleD;

Handle ResourceManifest::PBCSound;
Handle ResourceManifest::SniperSound;

void ResourceManifest::init(){
    Handle skyFromSpaceVert = Resources::addShader("data/Shaders/AS_skyFromSpace_vert.glsl",ShaderType::Vertex);
    Handle skyFromSpaceFrag = Resources::addShader("data/Shaders/AS_skyFromSpace_frag.glsl",ShaderType::Fragment);
    skyFromSpace = Resources::addShaderProgram("AS_SkyFromSpace",skyFromSpaceVert,skyFromSpaceFrag,ShaderRenderPass::Forward);

    Handle skyFromAtVert = Resources::addShader("data/Shaders/AS_skyFromAtmosphere_vert.glsl",ShaderType::Vertex);
    Handle skyFromAtFrag = Resources::addShader("data/Shaders/AS_skyFromAtmosphere_frag.glsl",ShaderType::Fragment);
    skyFromAtmosphere = Resources::addShaderProgram("AS_SkyFromAtmosphere",skyFromAtVert,skyFromAtFrag,ShaderRenderPass::Forward);

    Handle groundFromSpaceVert = Resources::addShader("data/Shaders/AS_groundFromSpace_vert.glsl",ShaderType::Vertex);
    Handle groundFromSpaceFrag = Resources::addShader("data/Shaders/AS_groundFromSpace_frag.glsl",ShaderType::Fragment);
    groundFromSpace = Resources::addShaderProgram("AS_GroundFromSpace",groundFromSpaceVert,groundFromSpaceFrag,ShaderRenderPass::Geometry);


    TestMesh = Resources::addMeshAsync("data/Models/1911.fbx",CollisionType::None,true,0.0f);
    PlanetMesh = Resources::addMeshAsync("data/Models/planet.obj",CollisionType::None);
    DefiantMesh = Resources::addMeshAsync("data/Models/defiant.obj",CollisionType::ConvexHull); //220 metres long (0.22 km)
    //AkiraMesh = Resources::addMeshAsync("data/Models/akira.obj",CollisionType::ConvexHull);
    //MirandaMesh = Resources::addMeshAsync("data/Models/miranda.obj",CollisionType::ConvexHull);
    //IntrepidMesh = Resources::addMeshAsync("data/Models/intrepid.obj",CollisionType::ConvexHull);
    //NorwayMesh = Resources::addMeshAsync("data/Models/norway.obj",CollisionType::ConvexHull);
	StarbaseMesh = Resources::addMeshAsync("data/Models/starbase.obj",CollisionType::TriangleShapeStatic); //6950 meters in height (6.95 km)
    RingMesh = Resources::addMeshAsync("data/Models/ring.obj",CollisionType::None);
    DreadnaughtMesh = Resources::addMeshAsync("data/Models/dreadnaught.obj",CollisionType::ConvexHull);
    //VenerexMesh = Resources::addMeshAsync("data/Models/venerex.obj",CollisionType::ConvexHull);
    //OberthMesh = Resources::addMeshAsync("data/Models/oberth.obj",CollisionType::ConvexHull);
    CapsuleTunnelMesh = Resources::addMeshAsync("data/Models/capsuleTunnel.obj",CollisionType::None);
    CapsuleRibbonMesh = Resources::addMeshAsync("data/Models/capsuleRibbon.obj",CollisionType::None);

    Engine::epriv::threading::waitForAll();

    StarbaseMaterial = Resources::addMaterial("Starbase","data/Textures/starbase.png","data/Textures/starbase_Normal.png","data/Textures/starbase_Glow.png");
    StarMaterial = Resources::addMaterial("Star","data/Textures/Planets/Sun.dds","","","");
    Resources::getMaterial(StarMaterial)->setShadeless(true);
    Resources::getMaterial(StarMaterial)->setGlow(0.21f);

    EarthSkyMaterial = Resources::addMaterial("EarthSky","data/Textures/Planets/Earth.dds","","","",skyFromSpace);

    DreadnaughtMaterial = Resources::addMaterial("Dreadnaught","data/Textures/dreadnaught.dds","data/Textures/dreadnaught_Normal.dds","data/Textures/dreadnaught_Glow.dds");
    DefiantMaterial = Resources::addMaterial("Defiant","data/Textures/defiant.dds","data/Textures/defiant_Normal.dds","data/Textures/defiant_Glow.dds");
    //AkiraMaterial = Resources::addMaterial("Akira","data/Textures/akira.dds","data/Textures/akira_Normal.png","data/Textures/akira_Glow.png");
    //MirandaMaterial = Resources::addMaterial("Miranda","data/Textures/miranda.dds","data/Textures/miranda_Normal.png","data/Textures/miranda_Glow.png");
    //IntrepidMaterial = Resources::addMaterial("Intrepid","data/Textures/intrepid.dds","data/Textures/intrepid_Normal.png","data/Textures/intrepid_Glow.png");
    //NorwayMaterial = Resources::addMaterial("Norway","data/Textures/norway.dds","data/Textures/norway_Normal.png","data/Textures/norway_Glow.png");
    //VenerexMaterial = Resources::addMaterial("Venerex","data/Textures/venerex.dds","data/Textures/venerex_Normal.png","data/Textures/venerex_Glow.png");
    //OberthMaterial = Resources::addMaterial("Oberth","data/Textures/oberth.dds","data/Textures/oberth_Normal.png","data/Textures/oberth_Glow.png");
    CrosshairMaterial = Resources::addMaterial("Crosshair","data/Textures/HUD/Crosshair.dds","","","");
    CrosshairArrowMaterial = Resources::addMaterial("CrosshairArrow","data/Textures/HUD/CrosshairArrow.dds","","","");
    StarFlareMaterial = Resources::addMaterial("SunFlare","data/Textures/Skyboxes/StarFlare.dds");
    Resources::getMaterial(StarFlareMaterial)->setShadeless(true);

    GoldMaterial = Resources::addMaterial("Gold","");
    Resources::getMaterial(GoldMaterial)->setMaterialPhysics(MaterialPhysics::Gold);

    PBCSound = Resources::addSoundData("data/Sounds/Effects/pbc.ogg","pbc");
    SniperSound = Resources::addSoundData("data/Sounds/Effects/snipercannon.ogg","sniper");

    CapsuleA = Resources::addMaterial("Capsule_A","data/Textures/Effects/capsule_a.dds");
    Resources::getMaterial(CapsuleA)->setShadeless(true);
    CapsuleB = Resources::addMaterial("Capsule_B","data/Textures/Effects/capsule_b.dds");
    Resources::getMaterial(CapsuleB)->setShadeless(true);
    CapsuleC = Resources::addMaterial("Capsule_C","data/Textures/Effects/capsule_c.dds");
    Resources::getMaterial(CapsuleC)->setShadeless(true);
    Resources::getMaterial(CapsuleC)->setGlow(0.01f);
    CapsuleD = Resources::addMaterial("Capsule_D","data/Textures/Effects/capsule_d.dds");
    Resources::getMaterial(CapsuleD)->setShadeless(true);

}