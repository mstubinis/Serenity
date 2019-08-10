#include <core/engine/mesh/MeshCollisionFactory.h>
#include <core/engine/mesh/VertexData.h>
#include <core/engine/utils/Utils.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/mesh/Mesh.h>

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <BulletCollision/Gimpact/btGImpactShape.h>

#include <glm/glm.hpp>

using namespace Engine;
using namespace std;

epriv::MeshCollisionFactory::MeshCollisionFactory(Mesh& _mesh) :m_Mesh(_mesh) {
    auto& data            = *_mesh.m_VertexData;
    m_ConvexHullData      = nullptr;
    m_ConvesHullShape     = nullptr;
    m_TriangleStaticData  = nullptr;
    m_TriangleStaticShape = nullptr;
    //m_Zero = new btVector3(0, 0, 0);
    _initConvexData(data);
    _initTriangleData(data);
}
epriv::MeshCollisionFactory::~MeshCollisionFactory() {
    //SAFE_DELETE(m_Zero);
    SAFE_DELETE(m_ConvexHullData);
    SAFE_DELETE(m_ConvesHullShape);
    SAFE_DELETE(m_TriangleStaticData);
    SAFE_DELETE(m_TriangleStaticShape);
}
void epriv::MeshCollisionFactory::_initConvexData(VertexData& data) {
    const auto& positions = data.getData<glm::vec3>(0);
    if (!m_ConvexHullData) {
        m_ConvesHullShape = new btConvexHullShape();
        for (auto& pos : positions) {
            m_ConvesHullShape->addPoint(btVector3(pos.x, pos.y, pos.z));
        }
        m_ConvexHullData = new btShapeHull(m_ConvesHullShape);
        m_ConvexHullData->buildHull(m_ConvesHullShape->getMargin());
        SAFE_DELETE(m_ConvesHullShape);
        const btVector3* ptsArray = m_ConvexHullData->getVertexPointer();
        m_ConvesHullShape = new btConvexHullShape();
        for (int i = 0; i < m_ConvexHullData->numVertices(); ++i) {
            m_ConvesHullShape->addPoint(ptsArray[i]);
        }
        m_ConvesHullShape->setMargin(0.001f);
        m_ConvesHullShape->recalcLocalAabb();
    }
}
void epriv::MeshCollisionFactory::_initTriangleData(VertexData& data) {
    if (!m_TriangleStaticData) {
        const auto& positions = data.getData<glm::vec3>(0);
        vector<glm::vec3> triangles;
        triangles.reserve(data.indices.size());
        for (auto& indice : data.indices) {
            triangles.push_back(positions[indice]);
        }
        m_TriangleStaticData = new btTriangleMesh();
        uint count = 0;
        vector<glm::vec3> tri;
        for (auto& position : triangles) {
            tri.push_back(position);
            ++count;
            if (count == 3) {
                const btVector3& v1 = Math::btVectorFromGLM(tri[0]);
                const btVector3& v2 = Math::btVectorFromGLM(tri[1]);
                const btVector3& v3 = Math::btVectorFromGLM(tri[2]);
                m_TriangleStaticData->addTriangle(v1, v2, v3, true);
                vector_clear(tri);
                count = 0;
            }
        }
        m_TriangleStaticShape = new btBvhTriangleMeshShape(m_TriangleStaticData, true);
        m_TriangleStaticShape->setMargin(0.001f);
        m_TriangleStaticShape->recalcLocalAabb();
    }
}
btMultiSphereShape* epriv::MeshCollisionFactory::buildSphereShape() {
    const auto& rad = m_Mesh.getRadius();
    auto v = btVector3(0, 0, 0);
    btMultiSphereShape* sphere = new btMultiSphereShape(&v, &rad, 1);
    sphere->setMargin(0.001f);
    sphere->recalcLocalAabb();
    return sphere;
}
btBoxShape* epriv::MeshCollisionFactory::buildBoxShape() {
    btBoxShape* box = new btBoxShape(Math::btVectorFromGLM(m_Mesh.getRadiusBox()));
    box->setMargin(0.001f);
    return box;
}
btUniformScalingShape* epriv::MeshCollisionFactory::buildConvexHull() {
    btUniformScalingShape* shape = new btUniformScalingShape(m_ConvesHullShape, 1.0f);
    return shape;
}
btScaledBvhTriangleMeshShape* epriv::MeshCollisionFactory::buildTriangleShape() {
    btScaledBvhTriangleMeshShape* shape = new btScaledBvhTriangleMeshShape(m_TriangleStaticShape, btVector3(1.0f, 1.0f, 1.0f));
    return shape;
}
btGImpactMeshShape* epriv::MeshCollisionFactory::buildTriangleShapeGImpact() {
    btGImpactMeshShape* shape = new btGImpactMeshShape(m_TriangleStaticData);
    shape->setMargin(0.001f);
    shape->updateBound();
    return shape;
}
