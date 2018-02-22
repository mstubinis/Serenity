#include "Engine.h"
#include "Terrain.h"
#include "Mesh.h"
#include "Engine_Resources.h"
#include "Engine_Physics.h"
#include "Engine_Events.h"
#include "ObjectDisplay.h"
#include "MeshInstance.h"

#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <boost/make_shared.hpp>

using namespace Engine;
using namespace std;

Terrain::Terrain(string n, sf::Image& image,string material,Scene* scene):ObjectDynamic("",material,glm::vec3(0),glm::vec3(1),n,nullptr,scene){
    for(unsigned int i = 0; i < image.getSize().x; i++){
        for(unsigned int j = 0; j < image.getSize().y; j++){
            float pixel(image.getPixel(i,j).r / 255.0f);
            m_Pixels.push_back(pixel);
        }
    }

    float minH = 0.0f;
    float maxH = 1.0f;
    m_Collision = new Collision(new btHeightfieldTerrainShape(image.getSize().x,image.getSize().y,&m_Pixels[0],1.0f,minH,maxH,1,PHY_FLOAT,false),
		                        CollisionType::TriangleShape,
                                0.0f);
    SAFE_DELETE(m_RigidBody);
    SAFE_DELETE(m_MotionState);

    btTransform tr;
    tr.setIdentity();
    tr.setOrigin(btVector3(0,(minH + maxH) * 0.5f,0));

    m_MotionState = new btDefaultMotionState(tr);
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(0,m_MotionState,m_Collision->getCollisionShape(),*m_Collision->getInertia());
    m_RigidBody = new btRigidBody(rigidBodyCI);

	Engine::impl::Core::m_Engine->m_ResourceManager->_addMesh(new Mesh(name(),(btHeightfieldTerrainShape*)(m_Collision->getCollisionShape()),0.0005f));

    if(material != ""){
        MeshInstance* meshInstance = new MeshInstance(name(),name(),material);
        m_MeshInstances.push_back(meshInstance);
    }

    if(Resources::getCurrentScene() == scene)
        Physics::addRigidBody(m_RigidBody);
    m_Collision->getCollisionShape()->setUserPointer(this);
    m_Collision->getCollisionShape()->setMargin(0.1f);
    m_RigidBody->setUserPointer(this);
    Terrain::setScale(1,1,1);
    setDynamic(false);
    m_RigidBody->setGravity(btVector3(0,0,0));
}
Terrain::~Terrain(){
    m_Pixels.clear();
    Physics::removeRigidBody(m_RigidBody);
    SAFE_DELETE(m_RigidBody);
}
void Terrain::update(float dt){
    ObjectDynamic::update(dt);
}
void Terrain::setPosition(float x,float y,float z){
    Physics::removeRigidBody(m_RigidBody);
    SAFE_DELETE(m_RigidBody);

    btTransform tr;tr.setIdentity();tr.setOrigin(btVector3(btScalar(x),btScalar(y),btScalar(z)));
    m_MotionState->setWorldTransform(tr);

    btRigidBody::btRigidBodyConstructionInfo ci(0,m_MotionState,m_Collision->getCollisionShape(),*m_Collision->getInertia());
    m_RigidBody = new btRigidBody(ci);
    m_RigidBody->setUserPointer(this);
    Physics::addRigidBody(m_RigidBody);
}
void Terrain::setPosition(glm::vec3 pos){ Terrain::setPosition(pos.x,pos.y,pos.z); }
void Terrain::setScale(float x,float y,float z){
    ObjectDynamic::setScale(x,y,z);
    Terrain::setPosition(Terrain::getPosition());
}
void Terrain::setScale(glm::vec3 scl){ Terrain::setScale(scl.x,scl.y,scl.z); }