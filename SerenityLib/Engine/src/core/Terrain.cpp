#include "Terrain.h"

#include <core/engine/mesh/Mesh.h>
#include <core/engine/system/Engine.h>
#include <core/engine/physics/Engine_Physics.h>
#include <core/engine/events/Engine_Events.h>
#include <core/engine/mesh/MeshRequest.h>
#include <core/engine/physics/Collision.h>

#include <core/engine/model/ModelInstance.h>

#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <boost/make_shared.hpp>
#include <SFML/Graphics.hpp>

using namespace Engine;

Terrain::Terrain(const std::string& name, sf::Image& heightmapImage, Handle& material, Scene* scene):EntityWrapper(*scene){
    for(unsigned int i = 0; i < heightmapImage.getSize().x; ++i){
        for(unsigned int j = 0; j < heightmapImage.getSize().y; ++j){
            float pixel(heightmapImage.getPixel(i,j).r / 255.0f);
            m_Pixels.push_back(pixel);
        }
    }
    float minH = 0.0f;
    float maxH = 1.0f;

    ComponentBody& physics = *entity().addComponent<ComponentBody>(CollisionType::TriangleShape);
    Collision* c = NEW Collision(
        *(new btHeightfieldTerrainShape(heightmapImage.getSize().x, heightmapImage.getSize().y, &m_Pixels[0], 1.0f, minH, maxH, 1, PHY_FLOAT, false)), 
        CollisionType::TriangleShape, 
        0.0f
    );
    physics.setCollision(c);

    MeshRequestPart part;
    part.name = name;
    part.mesh = NEW Mesh(name, *static_cast<btHeightfieldTerrainShape*>(c->getBtShape()), 0.0005f);
    part.mesh->setName(name);
    part.handle = epriv::Core::m_Engine->m_ResourceManager.m_Resources->add(part.mesh, ResourceType::Mesh);
  
    auto* model = entity().addComponent<ComponentModel>(part.mesh, material);
    physics.setPosition(0, (minH + maxH) * 0.5f, 0);
    physics.setDynamic(false);
    physics.setGravity(0, 0, 0);
    Terrain::setScale(1,1,1);
}
Terrain::~Terrain(){
    m_Pixels.clear();
}
void Terrain::update(float dt){
}
void Terrain::setPosition(float x,float y,float z){
    ComponentBody& body = *entity().getComponent<ComponentBody>();
    body.setPosition(x, y, z);

    //Physics::removeRigidBody(m_RigidBody);
    //SAFE_DELETE(m_RigidBody);

    //btRigidBody::btRigidBodyConstructionInfo ci(0,m_MotionState,m_Collision->getCollisionShape(),*m_Collision->getInertia());
    //m_RigidBody = new btRigidBody(ci);
    //m_RigidBody->setUserPointer(this);
    //Physics::addRigidBody(m_RigidBody);
}
void Terrain::setPosition(glm::vec3 pos){
    Terrain::setPosition(pos.x,pos.y,pos.z);
}
void Terrain::setScale(float x,float y,float z){
    ComponentBody& body = *entity().getComponent<ComponentBody>();
    body.setScale(x, y, z);
    Terrain::setPosition(body.position());
}
void Terrain::setScale(glm::vec3 scl){
    Terrain::setScale(scl.x,scl.y,scl.z);
}