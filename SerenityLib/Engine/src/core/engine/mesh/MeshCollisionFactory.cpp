#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/mesh/MeshCollisionFactory.h>
#include <core/engine/mesh/VertexData.h>
#include <core/engine/utils/Utils.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/mesh/Mesh.h>

#include <BulletCollision/Gimpact/btGImpactShape.h>
#include <BulletCollision/CollisionDispatch/btInternalEdgeUtility.h>
#include <BulletCollision/CollisionShapes/btMultiSphereShape.h>
#include <BulletCollision/CollisionShapes/btUniformScalingShape.h>
#include <BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.h>

constexpr btScalar DEFAULT_MARGIN = (btScalar)0.001;

Engine::priv::MeshCollisionFactory::MeshCollisionFactory(MeshCPUData& cpuData, MeshCollisionLoadingFlag::Flag flags)
    : m_CPUData{ &cpuData }
{
    auto& data                       = *m_CPUData->m_VertexData;
    std::vector<glm::vec3> positions = data.getPositions();

    if (flags & MeshCollisionLoadingFlag::LoadConvexHull) {
        internal_init_convex_data(data, positions);
    }
    if (flags & MeshCollisionLoadingFlag::LoadTriangleMesh) {
        internal_init_triangle_data(data, positions);
    }
}
Engine::priv::MeshCollisionFactory::MeshCollisionFactory(MeshCollisionFactory&& other) noexcept 
    : m_ConvexHullData      { std::move(other.m_ConvexHullData) }
    , m_TriangleStaticData  { std::move(other.m_TriangleStaticData) }
    , m_TriangleStaticShape { std::move(other.m_TriangleStaticShape) }
    , m_TriangleInfoMap     { std::move(other.m_TriangleInfoMap) }
    , m_CPUData             { std::exchange(other.m_CPUData, nullptr) }
{}
Engine::priv::MeshCollisionFactory& Engine::priv::MeshCollisionFactory::operator=(MeshCollisionFactory&& other) noexcept {
    m_ConvexHullData      = std::move(other.m_ConvexHullData);
    m_TriangleStaticData  = std::move(other.m_TriangleStaticData);
    m_TriangleStaticShape = std::move(other.m_TriangleStaticShape);
    m_TriangleInfoMap     = std::move(other.m_TriangleInfoMap);
    m_CPUData             = std::exchange(other.m_CPUData, nullptr);
    return *this;
}

void Engine::priv::MeshCollisionFactory::internal_init_convex_data(VertexData& data, std::vector<glm::vec3>& positions) {
    if (!m_ConvexHullData) {
        m_ConvesHullShape.reset(new btConvexHullShape());
        for (auto& pos : positions) {
            m_ConvesHullShape->addPoint(btVector3(pos.x, pos.y, pos.z));
        }
        m_ConvexHullData.reset(new btShapeHull(m_ConvesHullShape.get()));
        m_ConvexHullData->buildHull(m_ConvesHullShape->getMargin());
        const btVector3* ptsArray = m_ConvexHullData->getVertexPointer();
        m_ConvesHullShape.reset(new btConvexHullShape());
        for (int i = 0; i < m_ConvexHullData->numVertices(); ++i) {
            m_ConvesHullShape->addPoint(ptsArray[i]);
        }
        m_ConvesHullShape->setMargin(DEFAULT_MARGIN);
        m_ConvesHullShape->recalcLocalAabb();
    }
}
void Engine::priv::MeshCollisionFactory::internal_init_triangle_data(VertexData& data, std::vector<glm::vec3>& positions) {
    if (!m_TriangleStaticData) {
        std::vector<glm::vec3> triangles;
        triangles.reserve(data.m_Indices.size());
        for (auto& indice : data.m_Indices) {
            triangles.emplace_back(positions[indice]);
        }
        m_TriangleStaticData.reset(new btTriangleMesh());
        uint32_t count = 0;
        std::vector<glm::vec3> tri;
        for (auto& position : triangles) {
            tri.emplace_back(position);
            ++count;
            if (count == 3) {
                btVector3 v1 = Engine::Math::btVectorFromGLM(tri[0]);
                btVector3 v2 = Engine::Math::btVectorFromGLM(tri[1]);
                btVector3 v3 = Engine::Math::btVectorFromGLM(tri[2]);
                m_TriangleStaticData->addTriangle(v1, v2, v3, true);
                tri.clear();
                count = 0;
            }
        }
        m_TriangleStaticShape.reset(new btBvhTriangleMeshShape(m_TriangleStaticData.get(), true));
        m_TriangleStaticShape->setMargin(DEFAULT_MARGIN);
        m_TriangleStaticShape->recalcLocalAabb();

        m_TriangleInfoMap.reset(new btTriangleInfoMap());
        btGenerateInternalEdgeInfo(m_TriangleStaticShape.get(), m_TriangleInfoMap.get());
    }
}
btMultiSphereShape* Engine::priv::MeshCollisionFactory::buildSphereShape(ModelInstance* modelInstance, bool isCompoundChild) {
    ASSERT(m_CPUData->m_Radius > 0.0f, __FUNCTION__ << "(): m_CPUData->m_Radius is zero!");
    auto rad = (btScalar)m_CPUData->m_Radius;
    auto v   = btVector3(0, 0, 0);
    btMultiSphereShape* sphere = new btMultiSphereShape(&v, &rad, 1);
    sphere->setMargin(DEFAULT_MARGIN);
    sphere->recalcLocalAabb();
    if (isCompoundChild) {
        sphere->setUserPointer(modelInstance);
    }
    return sphere;
}
btBoxShape* Engine::priv::MeshCollisionFactory::buildBoxShape(ModelInstance* modelInstance, bool isCompoundChild) {
    ASSERT(m_CPUData->m_RadiusBox.x > 0.0f || m_CPUData->m_RadiusBox.y > 0.0f || m_CPUData->m_RadiusBox.z > 0.0f, __FUNCTION__ << "(): m_CPUData->m_RadiusBox is zero!");
    btVector3 btBox{ 
        m_CPUData->m_RadiusBox.x == 0.0f ? 0.005f : m_CPUData->m_RadiusBox.x,
        m_CPUData->m_RadiusBox.y == 0.0f ? 0.005f : m_CPUData->m_RadiusBox.y,
        m_CPUData->m_RadiusBox.z == 0.0f ? 0.005f : m_CPUData->m_RadiusBox.z
    };
    btBoxShape* box = new btBoxShape(btBox);
    box->setMargin(DEFAULT_MARGIN);
    if (isCompoundChild) {
        box->setUserPointer(modelInstance);
    }
    return box;
}
btUniformScalingShape* Engine::priv::MeshCollisionFactory::buildConvexHull(ModelInstance* modelInstance, bool isCompoundChild) {
    if (!m_ConvesHullShape) {
        ENGINE_PRODUCTION_LOG("Engine::priv::MeshCollisionFactory::buildConvexHull(): m_ConvesHullShape was null!")
        return nullptr;
    }
    btUniformScalingShape* uniformScalingShape = new btUniformScalingShape(m_ConvesHullShape.get(), (btScalar)1.0);
    uniformScalingShape->setMargin(DEFAULT_MARGIN);
    if (isCompoundChild) {
        uniformScalingShape->getChildShape()->setUserPointer(modelInstance);
        uniformScalingShape->setUserPointer(&modelInstance);
    }
    return uniformScalingShape;
}
btScaledBvhTriangleMeshShape* Engine::priv::MeshCollisionFactory::buildTriangleShape(ModelInstance* modelInstance, bool isCompoundChild) {
    if (!m_TriangleStaticData) {
        ENGINE_PRODUCTION_LOG("Engine::priv::MeshCollisionFactory::buildTriangleShape(): m_TriangleStaticData was null!")
        return nullptr;
    }
    btScaledBvhTriangleMeshShape* scaledBVH = new btScaledBvhTriangleMeshShape(m_TriangleStaticShape.get(), btVector3((btScalar)1.0, (btScalar)1.0, (btScalar)1.0));
    if (isCompoundChild) {
        scaledBVH->getChildShape()->setUserPointer(modelInstance);
        scaledBVH->setUserPointer(modelInstance);
    }
    return scaledBVH;
}
btGImpactMeshShape* Engine::priv::MeshCollisionFactory::buildTriangleShapeGImpact(ModelInstance* modelInstance, bool isCompoundChild) {
    if (!m_TriangleStaticData) {
        ENGINE_PRODUCTION_LOG("Engine::priv::MeshCollisionFactory::buildTriangleShapeGImpact(): m_TriangleStaticData was null!")
        return nullptr;
    }
    btGImpactMeshShape* gImpact = new btGImpactMeshShape(m_TriangleStaticData.get());
    gImpact->setMargin(DEFAULT_MARGIN);
    gImpact->updateBound();
    gImpact->postUpdate();
    if (isCompoundChild) {
        gImpact->setUserPointer(modelInstance);
    }
    return gImpact;
}
