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

priv::MeshCollisionFactory::MeshCollisionFactory(Mesh& mesh) : m_Mesh(mesh) {
    auto& data            = *mesh.m_VertexData;
    _initConvexData(data);
    _initTriangleData(data);
}
priv::MeshCollisionFactory::~MeshCollisionFactory() {
    SAFE_DELETE(m_ConvexHullData);
    SAFE_DELETE(m_ConvesHullShape);
    SAFE_DELETE(m_TriangleStaticData);
    SAFE_DELETE(m_TriangleStaticShape);
}
void priv::MeshCollisionFactory::_initConvexData(VertexData& data) {
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
        m_ConvesHullShape->setMargin(static_cast<btScalar>(0.001));
        m_ConvesHullShape->recalcLocalAabb();
    }
}
void priv::MeshCollisionFactory::_initTriangleData(VertexData& data) {
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
        m_TriangleStaticShape->setMargin(static_cast<btScalar>(0.001));
        m_TriangleStaticShape->recalcLocalAabb();
    }
}
btMultiSphereShape* priv::MeshCollisionFactory::buildSphereShape(ModelInstance* modelInstance, const bool isCompoundChild) {
    const auto& rad = static_cast<btScalar>(m_Mesh.getRadius());
    auto v = btVector3(0, 0, 0);
    btMultiSphereShape* sphere = new btMultiSphereShape(&v, &rad, 1);
    sphere->setMargin(static_cast<btScalar>(0.001));
    sphere->recalcLocalAabb();
    if (isCompoundChild) {
        sphere->setUserPointer(modelInstance);
    }
    return sphere;
}
btBoxShape* priv::MeshCollisionFactory::buildBoxShape(ModelInstance* modelInstance, const bool isCompoundChild) {
    btBoxShape* box = new btBoxShape(Math::btVectorFromGLM(m_Mesh.getRadiusBox()));
    box->setMargin(static_cast<btScalar>(0.001));
    if (isCompoundChild) {
        box->setUserPointer(modelInstance);
    }
    return box;
}
btUniformScalingShape* priv::MeshCollisionFactory::buildConvexHull(ModelInstance* modelInstance, const bool isCompoundChild) {
    btUniformScalingShape* uniformScalingShape = new btUniformScalingShape(m_ConvesHullShape, static_cast<btScalar>(1.0));
    uniformScalingShape->setMargin(static_cast<btScalar>(0.001));
    if (isCompoundChild) {
        uniformScalingShape->getChildShape()->setUserPointer(modelInstance);
        uniformScalingShape->setUserPointer(&modelInstance);
    }
    return uniformScalingShape;
}
btScaledBvhTriangleMeshShape* priv::MeshCollisionFactory::buildTriangleShape(ModelInstance* modelInstance, const bool isCompoundChild) {
    btScaledBvhTriangleMeshShape* scaledBVH = new btScaledBvhTriangleMeshShape(m_TriangleStaticShape, btVector3(static_cast<btScalar>(1.0), static_cast<btScalar>(1.0), static_cast<btScalar>(1.0)));
    if (isCompoundChild) {
        scaledBVH->getChildShape()->setUserPointer(modelInstance);
        scaledBVH->setUserPointer(modelInstance);
    }
    return scaledBVH;
}
btGImpactMeshShape* priv::MeshCollisionFactory::buildTriangleShapeGImpact(ModelInstance* modelInstance, const bool isCompoundChild) {
    btGImpactMeshShape* gImpact = new btGImpactMeshShape(m_TriangleStaticData);
    gImpact->setMargin(static_cast<btScalar>(0.001));
    gImpact->updateBound();
    gImpact->postUpdate();
    if (isCompoundChild) {
        gImpact->setUserPointer(modelInstance);
    }
    return gImpact;
}
