#include "ResourceManifest.h"
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

Handle ResourceManifest::StarbaseMaterial;
Handle ResourceManifest::StarMaterial;
Handle ResourceManifest::EarthMaterial;
Handle ResourceManifest::DreadnaughtMaterial;
Handle ResourceManifest::DefiantMaterial;
Handle ResourceManifest::AkiraMaterial;
Handle ResourceManifest::MirandaMaterial;
Handle ResourceManifest::IntrepidMaterial;
Handle ResourceManifest::NorwayMaterial;
Handle ResourceManifest::VenerexMaterial;
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
    Handle skyFromSpaceVert = Resources::addShader("AS_SkyFromSpace_Vert","data/Shaders/AS_skyFromSpace_vert.glsl",ShaderType::Vertex);
    Handle skyFromSpaceFrag = Resources::addShader("AS_SkyFromSpace_Frag","data/Shaders/AS_skyFromSpace_frag.glsl",ShaderType::Fragment);
    skyFromSpace = Resources::addShaderProgram("AS_SkyFromSpace",skyFromSpaceVert,skyFromSpaceFrag,ShaderRenderPass::Geometry);

    Handle skyFromAtVert = Resources::addShader("AS_SkyFromAtmosphere_Vert","data/Shaders/AS_skyFromAtmosphere_vert.glsl",ShaderType::Vertex);
    Handle skyFromAtFrag = Resources::addShader("AS_SkyFromAtmosphere_Frag","data/Shaders/AS_skyFromAtmosphere_frag.glsl",ShaderType::Fragment);
    skyFromAtmosphere = Resources::addShaderProgram("AS_SkyFromAtmosphere",skyFromAtVert,skyFromAtFrag,ShaderRenderPass::Geometry);

    Handle groundFromSpaceVert = Resources::addShader("AS_GroundFromSpace_Vert","data/Shaders/AS_groundFromSpace_vert.glsl",ShaderType::Vertex);
    Handle groundFromSpaceFrag = Resources::addShader("AS_GroundFromSpace_Frag","data/Shaders/AS_groundFromSpace_frag.glsl",ShaderType::Fragment);
    groundFromSpace = Resources::addShaderProgram("AS_GroundFromSpace",groundFromSpaceVert,groundFromSpaceFrag,ShaderRenderPass::Geometry);


    TestMesh = Resources::addMesh("data/Models/1911.fbx",CollisionType::None,true,0.0f);
    PlanetMesh = Resources::addMesh("data/Models/planet.obj",CollisionType::None,true);
    DefiantMesh = Resources::addMesh("data/Models/defiant.obj",CollisionType::ConvexHull,true);
    AkiraMesh = Resources::addMesh("data/Models/akira.obj",CollisionType::ConvexHull,true);
    MirandaMesh = Resources::addMesh("data/Models/miranda.obj",CollisionType::ConvexHull);
    IntrepidMesh = Resources::addMesh("data/Models/intrepid.obj",CollisionType::ConvexHull);
    NorwayMesh = Resources::addMesh("data/Models/norway.obj",CollisionType::ConvexHull);
    StarbaseMesh = Resources::addMesh("data/Models/starbase.obj",CollisionType::TriangleShapeStatic);
    RingMesh = Resources::addMesh("data/Models/ring.obj",CollisionType::None);
    DreadnaughtMesh = Resources::addMesh("data/Models/dreadnaught.obj",CollisionType::ConvexHull);
    VenerexMesh = Resources::addMesh("data/Models/venerex.obj",CollisionType::ConvexHull);


    StarbaseMaterial = Resources::addMaterial("Starbase","data/Textures/starbase.png","data/Textures/starbase_Normal.png","data/Textures/starbase_Glow.png");
    StarMaterial = Resources::addMaterial("Star","data/Textures/Planets/Sun.jpg","","","");
    Resources::getMaterial(StarMaterial)->setShadeless(true);
    Resources::getMaterial(StarMaterial)->setGlow(0.21f);
    EarthMaterial = Resources::addMaterial("Earth","data/Textures/Planets/Earth.jpg","","data/Textures/Planets/EarthNight.jpg","",ResourceManifest::groundFromSpace);
    DreadnaughtMaterial = Resources::addMaterial("Dreadnaught","data/Textures/dreadnaught.png","data/Textures/dreadnaught_Normal.png","data/Textures/dreadnaught_Glow.png");
    DefiantMaterial = Resources::addMaterial("Defiant","data/Textures/defiant.png","data/Textures/defiant_Normal.png","data/Textures/defiant_Glow.png");
    AkiraMaterial = Resources::addMaterial("Akira","data/Textures/akira.png","data/Textures/akira_Normal.png","data/Textures/akira_Glow.png");
    MirandaMaterial = Resources::addMaterial("Miranda","data/Textures/miranda.png","data/Textures/miranda_Normal.png","data/Textures/miranda_Glow.png");
    IntrepidMaterial = Resources::addMaterial("Intrepid","data/Textures/intrepid.png","data/Textures/intrepid_Normal.png","data/Textures/intrepid_Glow.png");
    NorwayMaterial = Resources::addMaterial("Norway","data/Textures/norway.png","data/Textures/norway_Normal.png","data/Textures/norway_Glow.png");
    VenerexMaterial = Resources::addMaterial("Venerex","data/Textures/venerex.png","data/Textures/venerex_Normal.png","data/Textures/venerex_Glow.png");
    CrosshairMaterial = Resources::addMaterial("Crosshair","data/Textures/HUD/Crosshair.png","","","");
    CrosshairArrowMaterial = Resources::addMaterial("CrosshairArrow","data/Textures/HUD/CrosshairArrow.png","","","");
    SunFlareMaterial = Resources::addMaterial("SunFlare","data/Textures/Skyboxes/StarFlare.png");
    Resources::getMaterial(SunFlareMaterial)->setShadeless(true);

    GoldMaterial = Resources::addMaterial("Gold","");
    Resources::getMaterial(GoldMaterial)->setMaterialPhysics(MaterialPhysics::Gold);

    PBCSound = Resources::addSoundData("data/Sounds/Effects/pbc.ogg","pbc");
    SniperSound = Resources::addSoundData("data/Sounds/Effects/snipercannon.ogg","sniper");



    CapsuleTunnelMesh = Resources::addMesh("data/Models/capsuleTunnel.obj",CollisionType::None);
    CapsuleRibbonMesh = Resources::addMesh("data/Models/capsuleRibbon.obj",CollisionType::None);


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