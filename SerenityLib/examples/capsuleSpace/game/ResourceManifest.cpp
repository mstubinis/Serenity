#include "ResourceManifest.h"

#include <core/engine/threading/Engine_ThreadManager.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/Material.h>
#include <core/engine/textures/Texture.h>

using namespace Engine;

Handle ResourceManifest::DreadnaughtMesh;

Handle ResourceManifest::DreadnaughtMaterial;
Handle ResourceManifest::CrosshairMaterial;
Handle ResourceManifest::CrosshairArrowMaterial;
Handle ResourceManifest::StarFlareMaterial;

Handle ResourceManifest::CapsuleTunnelMesh;
Handle ResourceManifest::CapsuleRibbonMeshA;
Handle ResourceManifest::CapsuleRibbonMeshB;
Handle ResourceManifest::CapsuleA;
Handle ResourceManifest::CapsuleB;
Handle ResourceManifest::CapsuleC;
Handle ResourceManifest::CapsuleD;

std::string ResourceManifest::BasePath;

void ResourceManifest::init(){

    BasePath = "../";

    DreadnaughtMesh = Resources::loadMeshAsync(BasePath + "data/Models/dreadnaught.objcc").at(0);
 
    CapsuleTunnelMesh = Resources::loadMeshAsync(BasePath + "data/Models/capsuleTunnel.objcc").at(0);
    CapsuleRibbonMeshA = Resources::loadMeshAsync(BasePath + "data/Models/ribbon1.objcc").at(0);
    CapsuleRibbonMeshB = Resources::loadMeshAsync(BasePath + "data/Models/ribbon2.objcc").at(0);

    Engine::epriv::threading::waitForAll();

    DreadnaughtMaterial = Resources::addMaterial("Dreadnaught", BasePath + "data/Textures/dreadnaught.dds", BasePath + "data/Textures/dreadnaught_Normal.dds", BasePath + "data/Textures/dreadnaught_Glow.dds");

    CrosshairMaterial = Resources::addMaterial("Crosshair", BasePath + "data/Textures/HUD/Crosshair.dds","","","");
    CrosshairArrowMaterial = Resources::addMaterial("CrosshairArrow", BasePath + "data/Textures/HUD/CrosshairArrow.dds","","","");

    auto& crosshairDiffuse = *(((Material*)CrosshairMaterial.get())->getComponent(MaterialComponentType::Diffuse)->texture());
    auto& crosshairArrowDiffuse = *(((Material*)CrosshairArrowMaterial.get())->getComponent(MaterialComponentType::Diffuse)->texture());

    crosshairDiffuse.setWrapping(TextureWrap::ClampToEdge);
    crosshairDiffuse.setFilter(TextureFilter::Nearest);

    StarFlareMaterial = Resources::addMaterial("SunFlare", BasePath + "data/Textures/Skyboxes/StarFlare.dds");
    Resources::getMaterial(StarFlareMaterial)->setShadeless(true);

    CapsuleA = Resources::addMaterial("Capsule_A", BasePath + "data/Textures/Effects/capsule_a.dds");
    ((Material*)CapsuleA.get())->setShadeless(true);
    ((Material*)CapsuleA.get())->getComponent(MaterialComponentType::Diffuse)->texture()->setAnisotropicFiltering(2.0f);
    CapsuleB = Resources::addMaterial("Capsule_B", BasePath + "data/Textures/Effects/capsule_b.dds");
    ((Material*)CapsuleB.get())->setShadeless(true);
    ((Material*)CapsuleB.get())->getComponent(MaterialComponentType::Diffuse)->texture()->setAnisotropicFiltering(2.0f);
    CapsuleC = Resources::addMaterial("Capsule_C", BasePath + "data/Textures/Effects/capsule_c.dds");
    ((Material*)CapsuleC.get())->setShadeless(true);
    ((Material*)CapsuleC.get())->setGlow(0.2f);
    ((Material*)CapsuleC.get())->getComponent(MaterialComponentType::Diffuse)->texture()->setAnisotropicFiltering(2.0f);
    CapsuleD = Resources::addMaterial("Capsule_D", BasePath + "data/Textures/Effects/capsule_d.dds");
    ((Material*)CapsuleD.get())->setShadeless(true);  
    ((Material*)CapsuleD.get())->getComponent(MaterialComponentType::Diffuse)->texture()->setAnisotropicFiltering(2.0f);
}