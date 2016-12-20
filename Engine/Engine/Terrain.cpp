#include "Terrain.h"
#include "Mesh.h"
#include "Engine_Resources.h"
#include "Engine_Physics.h"
#include "Engine_Events.h"
#include "ObjectDisplay.h"

#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <boost/make_shared.hpp>

using namespace Engine;

Terrain::Terrain(std::string n, sf::Image& image,std::string material,Scene* scene):ObjectDynamic("",material,glm::v3(0),glm::vec3(1),n,nullptr,scene){
    for(unsigned int i = 0; i < image.getSize().x; i++){
        for(unsigned int j = 0; j < image.getSize().y; j++){
            float pixel(image.getPixel(i,j).r / 255.0f);
            m_Pixels.push_back(pixel);
        }
    }

    float minH = 0.0f;
    float maxH = 1.0f;
    m_Collision = new Collision(new btHeightfieldTerrainShape(image.getSize().x,image.getSize().y,&m_Pixels[0],1.0f,minH,maxH,1,PHY_FLOAT,false),
                                COLLISION_TYPE_TRIANGLESHAPE,
                                0.0f);
    SAFE_DELETE(m_RigidBody);
    SAFE_DELETE(m_MotionState);

    btTransform tr;
    tr.setIdentity();
    tr.setOrigin(btVector3(0,(minH + maxH) * 0.5f,0));

    m_MotionState = new btDefaultMotionState(tr);
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(0,m_MotionState,m_Collision->getCollisionShape(),*m_Collision->getInertia());
    m_RigidBody = new btRigidBody(rigidBodyCI);

    Resources::Detail::ResourceManagement::m_Meshes[name()] = boost::make_shared<Mesh>(name(),static_cast<btHeightfieldTerrainShape*>(m_Collision->getCollisionShape()));

    if(material != "")
        m_DisplayItems.push_back(new DisplayItem(Resources::getMesh(name()),Resources::getMaterial(material)));

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
void Terrain::render(){
    ObjectDynamic::render();
}
void Terrain::setPosition(glm::num x,glm::num y,glm::num z){
    Physics::removeRigidBody(m_RigidBody);
    SAFE_DELETE(m_RigidBody);

    btTransform tr;tr.setIdentity();tr.setOrigin(btVector3(btScalar(x),btScalar(y),btScalar(z)));
    m_MotionState->setWorldTransform(tr);

    btRigidBody::btRigidBodyConstructionInfo ci(0,m_MotionState,m_Collision->getCollisionShape(),*m_Collision->getInertia());
    m_RigidBody = new btRigidBody(ci);
    m_RigidBody->setUserPointer(this);
    Physics::addRigidBody(m_RigidBody);
}
void Terrain::setPosition(glm::v3 pos){ Terrain::setPosition(pos.x,pos.y,pos.z); }
void Terrain::setScale(float x,float y,float z){
    ObjectDynamic::setScale(x,y,z);
    Terrain::setPosition(Terrain::getPosition());
}
void Terrain::setScale(glm::vec3 scl){ Terrain::setScale(scl.x,scl.y,scl.z); }