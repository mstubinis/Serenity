#include "Terrain.h"

#include <ecs/Components.h>
#include <core/engine/system/Engine.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/system/Engine.h>
#include <core/engine/mesh/MeshRequest.h>
#include <core/engine/physics/Collision.h>

#include <core/engine/model/ModelInstance.h>

#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <SFML/Graphics.hpp>

using namespace std;
using namespace Engine;


void Terrain::Data::calculate_data(sf::Image& heightmapImage) {
    const auto heightmapSize = heightmapImage.getSize();
    m_Data.clear();
    //init the map with points at 0.0
    m_Data.resize((heightmapSize.y * 2) + 1);
    for (unsigned int row = 0; row < m_Data.size(); ++row) {
        for (unsigned int col = 0; col < (heightmapSize.x * 2) + 1; ++col) {
            m_Data[row].push_back(0.0f);
        }
    }
    //init all points within a sect to the pixel height
    for (int i = 0; i < heightmapSize.y; ++i) {
        for (int j = 0; j < heightmapSize.x; ++j) {

        }
    }
}
btHeightfieldTerrainShape* Terrain::Data::generate_bt_shape() {
    vector<float> temp;
    temp.reserve(m_Data.size() * m_Data[0].size());
    for (const auto& row : m_Data) {
        for (const auto pixel : row) {
            temp.push_back(pixel);
        }
    }
    SAFE_DELETE(m_BtHeightfieldShape);
    m_BtHeightfieldShape = new btHeightfieldTerrainShape(m_Data[0].size(), m_Data.size(), &temp[0], m_HeightScale, m_MinHeight, m_MaxHeight, 1, PHY_FLOAT, false);
    m_BtHeightfieldShape->setUserIndex(m_Data[0].size());
    m_BtHeightfieldShape->setUserIndex2(m_Data.size());
    return m_BtHeightfieldShape;
}
void Terrain::Data::subdivide(unsigned int levels) {

}

Terrain::Terrain(const string& name, sf::Image& heightmapImage, Handle& materialHandle, Scene* scene) : Entity(*scene){
    m_TerrainData.calculate_data(heightmapImage);
    m_TerrainData.generate_bt_shape();

    m_Mesh = NEW Mesh(name, *m_TerrainData.m_BtHeightfieldShape, 0.0005f);
    m_Mesh->setName(name);
    Handle handle  = priv::Core::m_Engine->m_ResourceManager.m_Resources.add(m_Mesh, ResourceType::Mesh);

    addComponent<ComponentModel>(m_Mesh, materialHandle);
    addComponent<ComponentBody>(CollisionType::TriangleShape); //TODO: check CollisionType::TriangleShapeStatic
    auto* body   = getComponent<ComponentBody>();
    auto* model  = getComponent<ComponentModel>();
    Collision* c = NEW Collision(*body);
    c->setBtShape(m_TerrainData.m_BtHeightfieldShape);
    body->setCollision(c);
    body->setPosition(0, (m_TerrainData.m_MinHeight + m_TerrainData.m_MaxHeight) * 0.5f, 0);
    body->setDynamic(false);
    body->setGravity(0, 0, 0);
    Terrain::setScale(1,1,1);
}
Terrain::~Terrain(){
}
void Terrain::subdivide(unsigned int levels) {
    //subdivides the mesh by the number of levels. if levels is zero, the mesh just recalculates using the class's m_TerrainData
    m_TerrainData.subdivide(levels);
}
void Terrain::update(const float dt){
}
void Terrain::setPosition(float x, float y, float z){
    ComponentBody& body = *getComponent<ComponentBody>();
    Physics::removeRigidBody(body);
    body.setPosition(x, y, z);
    Physics::addRigidBody(body);
}
void Terrain::setPosition(glm::vec3 pos){
    Terrain::setPosition(pos.x,pos.y,pos.z);
}
void Terrain::setScale(float x, float y, float z){
    ComponentBody& body = *getComponent<ComponentBody>();
    body.setScale(x, y, z);
    Terrain::setPosition(body.position());
}
void Terrain::setScale(glm::vec3 scl){
    Terrain::setScale(scl.x,scl.y,scl.z);
}