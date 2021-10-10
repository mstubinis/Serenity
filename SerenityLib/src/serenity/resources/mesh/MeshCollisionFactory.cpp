#include <serenity/resources/mesh/MeshCollisionFactory.h>
#include <serenity/resources/mesh/VertexData.h>
#include <serenity/utils/Utils.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/resources/mesh/Mesh.h>

#include <BulletCollision/Gimpact/btGImpactShape.h>
#include <BulletCollision/CollisionDispatch/btInternalEdgeUtility.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <BulletCollision/CollisionShapes/btMultiSphereShape.h>
#include <BulletCollision/CollisionShapes/btUniformScalingShape.h>
#include <BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.h>

constexpr btScalar DEFAULT_MARGIN = static_cast<btScalar>(0.001);

Engine::priv::MeshCollisionFactory::MeshCollisionFactory(MeshCPUData& cpuData, MeshCollisionLoadingFlag::Flag flags)
    : m_CPUData{ &cpuData }
{
    auto& data                       = *m_CPUData->m_VertexData;
    std::vector<glm::vec3> positions = data.getPositions();
    if (flags & MeshCollisionLoadingFlag::LoadConvexHull) {
        internal_init_convex_data(positions);
    }
    if (flags & MeshCollisionLoadingFlag::LoadTriangleMesh) {
        internal_init_triangle_data(data, positions);
    }
}
Engine::priv::MeshCollisionFactory::MeshCollisionFactory(MeshCollisionFactory&& other) noexcept 
    : m_ConvesHullShape     { std::move(other.m_ConvesHullShape) }
    , m_TriangleStaticData  { std::move(other.m_TriangleStaticData) }
    , m_TriangleStaticShape { std::move(other.m_TriangleStaticShape) }
    , m_TriangleInfoMap     { std::move(other.m_TriangleInfoMap) }
    , m_CPUData             { std::exchange(other.m_CPUData, nullptr) }
{}
Engine::priv::MeshCollisionFactory& Engine::priv::MeshCollisionFactory::operator=(MeshCollisionFactory&& other) noexcept {
    m_ConvesHullShape     = std::move(other.m_ConvesHullShape);
    m_TriangleStaticData  = std::move(other.m_TriangleStaticData);
    m_TriangleStaticShape = std::move(other.m_TriangleStaticShape);
    m_TriangleInfoMap     = std::move(other.m_TriangleInfoMap);
    m_CPUData             = std::exchange(other.m_CPUData, nullptr);
    return *this;
}
void Engine::priv::MeshCollisionFactory::internal_init_convex_data(std::vector<glm::vec3>& positions) {
    if (m_ConvesHullShape.getNumPoints() > 0) {
        return;
    }
    //build convex hull using all the positions
    m_ConvesHullShape = btConvexHullShape{};
    for (auto& pos : positions) {
        m_ConvesHullShape.addPoint(btVector3(pos.x, pos.y, pos.z));
    }
    //then convert it to a more optimized hull by removing and transforming the un-needed positions
    btShapeHull optimizedHull = btShapeHull{ &m_ConvesHullShape };
    optimizedHull.buildHull(m_ConvesHullShape.getMargin());
    const btVector3* ptsArray = optimizedHull.getVertexPointer();
    m_ConvesHullShape = btConvexHullShape{};
    for (int i = 0; i < optimizedHull.numVertices(); ++i) {
        m_ConvesHullShape.addPoint(ptsArray[i]);
    }
    m_ConvesHullShape.setMargin(DEFAULT_MARGIN);
    m_ConvesHullShape.recalcLocalAabb();
}
void Engine::priv::MeshCollisionFactory::internal_init_triangle_data(VertexData& data, std::vector<glm::vec3>& positions) {
    if (m_TriangleStaticData.getNumTriangles() > 0) {
        return;
    }
    auto triangles = Engine::create_and_reserve<std::vector<glm::vec3>>(data.m_Indices.size());
    for (auto& indice : data.m_Indices) {
        triangles.emplace_back(positions[indice]);
    }
    m_TriangleStaticData = btTriangleMesh{};
    auto tri = Engine::create_and_reserve<std::vector<glm::vec3>>(3);
    for (auto& position : triangles) {
        tri.emplace_back(position);
        if (tri.size() == 3) {
            btVector3 v1 = Engine::Math::toBT(tri[0]);
            btVector3 v2 = Engine::Math::toBT(tri[1]);
            btVector3 v3 = Engine::Math::toBT(tri[2]);
            m_TriangleStaticData.addTriangle(v1, v2, v3, true);
            tri.clear();
        }
    }
    m_TriangleStaticShape.reset(new btBvhTriangleMeshShape{ &m_TriangleStaticData, true, true });
    m_TriangleStaticShape->setMargin(DEFAULT_MARGIN);
    m_TriangleStaticShape->recalcLocalAabb();

    m_TriangleInfoMap = btTriangleInfoMap{};
    btGenerateInternalEdgeInfo(m_TriangleStaticShape.get(), &m_TriangleInfoMap);
}
btSphereShape* Engine::priv::MeshCollisionFactory::buildSphereShape(ModelInstance* modelInstance, bool isCompoundChild) {
    ASSERT(m_CPUData->m_Radius > 0.0f, __FUNCTION__ << "(): m_CPUData->m_Radius is zero!");
    auto rad = (btScalar)m_CPUData->m_Radius;
    btSphereShape* sphere = new btSphereShape{ rad };
    sphere->setMargin(DEFAULT_MARGIN);
    if (isCompoundChild) {
        sphere->setUserPointer(modelInstance);
    }
    return sphere;
}
btMultiSphereShape* Engine::priv::MeshCollisionFactory::buildMultiSphereShape(ModelInstance* modelInstance, bool isCompoundChild) {
    ASSERT(m_CPUData->m_Radius > 0.0f, __FUNCTION__ << "(): m_CPUData->m_Radius is zero!");
    auto rad = (btScalar)m_CPUData->m_Radius;
    auto v   = btVector3{ 0, 0, 0 };
    btMultiSphereShape* multiSphere = new btMultiSphereShape{ &v, &rad, 1 };
    multiSphere->setMargin(DEFAULT_MARGIN);
    multiSphere->recalcLocalAabb();
    if (isCompoundChild) {
        multiSphere->setUserPointer(modelInstance);
    }
    return multiSphere;
}
btBoxShape* Engine::priv::MeshCollisionFactory::buildBoxShape(ModelInstance* modelInstance, bool isCompoundChild) {
    const auto& boxRad = m_CPUData->m_RadiusBox;
    ASSERT(boxRad.x > 0.0f || boxRad.y > 0.0f || boxRad.z > 0.0f, __FUNCTION__ << "(): m_CPUData->m_RadiusBox is zero!");
    btBoxShape* box = new btBoxShape{ btVector3 { boxRad.x <= 0.0f ? 0.05f : boxRad.x, boxRad.y <= 0.0f ? 0.05f : boxRad.y, boxRad.z <= 0.0f ? 0.05f : boxRad.z } };
    box->setMargin(DEFAULT_MARGIN);
    if (isCompoundChild) {
        box->setUserPointer(modelInstance);
    }
    return box;
}
btUniformScalingShape* Engine::priv::MeshCollisionFactory::buildConvexHull(ModelInstance* modelInstance, bool isCompoundChild) {
    if (m_ConvesHullShape.getNumPoints() == 0) {
        ENGINE_PRODUCTION_LOG(__FUNCTION__ << "(): m_ConvesHullShape was empty!")
        return nullptr;
    }
    btUniformScalingShape* uniformScalingShape = new btUniformScalingShape( &m_ConvesHullShape, (btScalar)1.0 );
    uniformScalingShape->setMargin(DEFAULT_MARGIN);
    if (isCompoundChild) {
        uniformScalingShape->getChildShape()->setUserPointer(modelInstance);
        uniformScalingShape->setUserPointer(&modelInstance);
    }
    return uniformScalingShape;
}
btScaledBvhTriangleMeshShape* Engine::priv::MeshCollisionFactory::buildTriangleShape(ModelInstance* modelInstance, bool isCompoundChild) {
    if (m_TriangleStaticData.getNumTriangles() == 0) {
        ENGINE_PRODUCTION_LOG(__FUNCTION__ << "(): m_TriangleStaticData was empty!")
        return nullptr;
    }
    btScaledBvhTriangleMeshShape* scaledBVH = new btScaledBvhTriangleMeshShape( m_TriangleStaticShape.get(), btVector3{ (btScalar)1.0, (btScalar)1.0, (btScalar)1.0} );
    scaledBVH->setMargin(DEFAULT_MARGIN);
    if (isCompoundChild) {
        scaledBVH->getChildShape()->setUserPointer(modelInstance);
        scaledBVH->setUserPointer(modelInstance);
    }
    return scaledBVH;
}
btGImpactMeshShape* Engine::priv::MeshCollisionFactory::buildTriangleShapeGImpact(ModelInstance* modelInstance, bool isCompoundChild) {
    if (m_TriangleStaticData.getNumTriangles() == 0) {
        ENGINE_PRODUCTION_LOG(__FUNCTION__ << "(): m_TriangleStaticData was empty!")
        return nullptr;
    }
    btGImpactMeshShape* gImpact = new btGImpactMeshShape{ &m_TriangleStaticData };
    gImpact->setMargin(DEFAULT_MARGIN);
    gImpact->updateBound();
    gImpact->postUpdate();
    if (isCompoundChild) {
        gImpact->setUserPointer(modelInstance);
    }
    return gImpact;
}
