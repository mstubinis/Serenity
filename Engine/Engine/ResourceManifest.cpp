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
Handle ResourceManifest::SunFlareMaterial;
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
    PlanetMesh = Resources::addMeshAsync("data/Models/planet.obj",CollisionType::None,true);
    DefiantMesh = Resources::addMeshAsync("data/Models/defiant.obj",CollisionType::ConvexHull,true);
    //AkiraMesh = Resources::addMeshAsync("data/Models/akira.obj",CollisionType::ConvexHull,true);
    //MirandaMesh = Resources::addMeshAsync("data/Models/miranda.obj",CollisionType::ConvexHull);
    //IntrepidMesh = Resources::addMeshAsync("data/Models/intrepid.obj",CollisionType::ConvexHull);
    //NorwayMesh = Resources::addMeshAsync("data/Models/norway.obj",CollisionType::ConvexHull);
    StarbaseMesh = Resources::addMeshAsync("data/Models/starbase.obj",CollisionType::TriangleShapeStatic);
    RingMesh = Resources::addMeshAsync("data/Models/ring.obj",CollisionType::None);
    DreadnaughtMesh = Resources::addMeshAsync("data/Models/dreadnaught.obj",CollisionType::ConvexHull);
    //VenerexMesh = Resources::addMeshAsync("data/Models/venerex.obj",CollisionType::ConvexHull);
    //OberthMesh = Resources::addMeshAsync("data/Models/oberth.obj",CollisionType::ConvexHull);
    CapsuleTunnelMesh = Resources::addMeshAsync("data/Models/capsuleTunnel.obj",CollisionType::None);
    CapsuleRibbonMesh = Resources::addMeshAsync("data/Models/capsuleRibbon.obj",CollisionType::None);	

    Engine::epriv::threading::waitForAll();

    StarbaseMaterial = Resources::addMaterial("Starbase","data/Textures/starbase.png","data/Textures/starbase_Normal.png","data/Textures/starbase_Glow.png");
    StarMaterial = Resources::addMaterial("Star","data/Textures/Planets/Sun.jpg","","","");
    Resources::getMaterial(StarMaterial)->setShadeless(true);
    Resources::getMaterial(StarMaterial)->setGlow(0.21f);

    EarthSkyMaterial = Resources::addMaterial("EarthSky","data/Textures/Planets/Earth.jpg","","","",skyFromSpace);



    DreadnaughtMaterial = Resources::addMaterial("Dreadnaught","data/Textures/dreadnaught.png","data/Textures/dreadnaught_Normal.png","data/Textures/dreadnaught_Glow.png");
    DefiantMaterial = Resources::addMaterial("Defiant","data/Textures/defiant.png","data/Textures/defiant_Normal.png","data/Textures/defiant_Glow.png");
    //AkiraMaterial = Resources::addMaterial("Akira","data/Textures/akira.png","data/Textures/akira_Normal.png","data/Textures/akira_Glow.png");
    //MirandaMaterial = Resources::addMaterial("Miranda","data/Textures/miranda.png","data/Textures/miranda_Normal.png","data/Textures/miranda_Glow.png");
    //IntrepidMaterial = Resources::addMaterial("Intrepid","data/Textures/intrepid.png","data/Textures/intrepid_Normal.png","data/Textures/intrepid_Glow.png");
    //NorwayMaterial = Resources::addMaterial("Norway","data/Textures/norway.png","data/Textures/norway_Normal.png","data/Textures/norway_Glow.png");
    //VenerexMaterial = Resources::addMaterial("Venerex","data/Textures/venerex.png","data/Textures/venerex_Normal.png","data/Textures/venerex_Glow.png");
    //OberthMaterial = Resources::addMaterial("Oberth","data/Textures/oberth.png","data/Textures/oberth_Normal.png","data/Textures/oberth_Glow.png");
    CrosshairMaterial = Resources::addMaterial("Crosshair","data/Textures/HUD/Crosshair.png","","","");
    CrosshairArrowMaterial = Resources::addMaterial("CrosshairArrow","data/Textures/HUD/CrosshairArrow.png","","","");
    SunFlareMaterial = Resources::addMaterial("SunFlare","data/Textures/Skyboxes/StarFlare.png");
    Resources::getMaterial(SunFlareMaterial)->setShadeless(true);

    GoldMaterial = Resources::addMaterial("Gold","");
    Resources::getMaterial(GoldMaterial)->setMaterialPhysics(MaterialPhysics::Gold);

    PBCSound = Resources::addSoundData("data/Sounds/Effects/pbc.ogg","pbc");
    SniperSound = Resources::addSoundData("data/Sounds/Effects/snipercannon.ogg","sniper");

    CapsuleA = Resources::addMaterial("Capsule_A","data/Textures/Effects/capsule_a.png");
    Resources::getMaterial(CapsuleA)->setShadeless(true);
    CapsuleB = Resources::addMaterial("Capsule_B","data/Textures/Effects/capsule_b.png");
    Resources::getMaterial(CapsuleB)->setShadeless(true);
    CapsuleC = Resources::addMaterial("Capsule_C","data/Textures/Effects/capsule_c.png");
    Resources::getMaterial(CapsuleC)->setShadeless(true);
    Resources::getMaterial(CapsuleC)->setGlow(0.01f);
    CapsuleD = Resources::addMaterial("Capsule_D","data/Textures/Effects/capsule_d.png");
    Resources::getMaterial(CapsuleD)->setShadeless(true);

}