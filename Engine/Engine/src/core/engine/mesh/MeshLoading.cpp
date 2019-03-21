#include <core/engine/mesh/MeshLoading.h>
#include <core/engine/mesh/MeshImportedData.h>
#include <core/engine/mesh/VertexData.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/Engine_Math.h>

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <bullet/BulletCollision/CollisionShapes/btShapeHull.h>
#include <bullet/BulletCollision/Gimpact/btGImpactShape.h>

#include <boost/math/special_functions/fpclassify.hpp>
#include <glm/glm.hpp>

using namespace Engine;
using namespace std;
namespace boostm = boost::math;

bool epriv::MeshLoader::IsNear(float& v1, float& v2, float& threshold) {
    return std::abs(v1 - v2) < threshold;
}
bool epriv::MeshLoader::IsNear(glm::vec2& v1, glm::vec2& v2, float& threshold) {
    return (std::abs(v1.x - v2.x) < threshold && std::abs(v1.y - v2.y) < threshold) ? true : false;
}
bool epriv::MeshLoader::IsNear(glm::vec3& v1, glm::vec3& v2, float& threshold) {
    return (std::abs(v1.x - v2.x) < threshold && std::abs(v1.y - v2.y) < threshold && std::abs(v1.z - v2.z) < threshold) ? true : false;
}
bool epriv::MeshLoader::IsSpecialFloat(float& f) {
    if (boostm::isnan(f) || boostm::isinf(f)) return true;
    return false;
}
bool epriv::MeshLoader::IsSpecialFloat(glm::vec2& v) {
    if (boostm::isnan(v.x) || boostm::isnan(v.y)) return true;
    if (boostm::isinf(v.x) || boostm::isinf(v.y)) return true;
    return false;
}
bool epriv::MeshLoader::IsSpecialFloat(glm::vec3& v) {
    if (boostm::isnan(v.x) || boostm::isnan(v.y) || boostm::isnan(v.z)) return true;
    if (boostm::isinf(v.x) || boostm::isinf(v.y) || boostm::isinf(v.z)) return true;
    return false;
}
bool epriv::MeshLoader::GetSimilarVertexIndex(glm::vec3& in_pos, glm::vec2& in_uv, glm::vec3& in_norm, vector<glm::vec3>& pts, vector<glm::vec2>& uvs, vector<glm::vec3>& norms, unsigned short& result, float& threshold) {
    for (uint t = 0; t < pts.size(); ++t) {
        if (IsNear(in_pos, pts[t], threshold) && IsNear(in_uv, uvs[t], threshold) && IsNear(in_norm, norms[t], threshold)) {
            result = t;
            return true;
        }
    }
    return false;
}
void epriv::MeshLoader::CalculateTBNAssimp(MeshImportedData& data) {
    if (data.normals.size() == 0) return;
    uint dataSize(data.points.size());
    for (uint i = 0; i < dataSize; i += 3) {
        uint p0(i + 0); uint p1(i + 1); uint p2(i + 2);

        glm::vec3 dataP0, dataP1, dataP2;
        glm::vec2 uvP0, uvP1, uvP2;

        uint uvSize(data.uvs.size());

        if (dataSize > p0) dataP0 = data.points[p0];
        else              dataP0 = glm::vec3(0.0f);
        if (dataSize > p1) dataP1 = data.points[p1];
        else              dataP1 = glm::vec3(0.0f);
        if (dataSize > p2) dataP2 = data.points[p2];
        else              dataP2 = glm::vec3(0.0f);

        if (uvSize > p0)   uvP0 = data.uvs[p0];
        else              uvP0 = glm::vec2(0.0f);
        if (uvSize > p1)   uvP1 = data.uvs[p1];
        else              uvP1 = glm::vec2(0.0f);
        if (uvSize > p2)   uvP2 = data.uvs[p2];
        else              uvP2 = glm::vec2(0.0f);

        glm::vec3 v(dataP1 - dataP0);
        glm::vec3 w(dataP2 - dataP0);

        // texture offset p1->p2 and p1->p3
        float sx(uvP1.x - uvP0.x);
        float sy(uvP1.y - uvP0.y);
        float tx(uvP2.x - uvP0.x);
        float ty(uvP2.y - uvP0.y);
        float dirCorrection = 1.0;

        if ((tx * sy - ty * sx) < 0.0f) dirCorrection = -1.0f;//this is important for normals and mirrored mesh geometry using identical uv's


        // when t1, t2, t3 in same position in UV space, just use default UV direction.
        //if ( 0 == sx && 0 == sy && 0 == tx && 0 == ty ) {
        if (sx * ty == sy * tx) {
            sx = 0.0; sy = 1.0;
            tx = 1.0; ty = 0.0;
        }

        // tangent points in the direction where to positive X axis of the texture coord's would point in model space
        // bitangent's points along the positive Y axis of the texture coord's, respectively
        glm::vec3 tangent;
        glm::vec3 bitangent;
        tangent.x = (w.x * sy - v.x * ty) * dirCorrection;
        tangent.y = (w.y * sy - v.y * ty) * dirCorrection;
        tangent.z = (w.z * sy - v.z * ty) * dirCorrection;
        bitangent.x = (w.x * sx - v.x * tx) * dirCorrection;
        bitangent.y = (w.y * sx - v.y * tx) * dirCorrection;
        bitangent.z = (w.z * sx - v.z * tx) * dirCorrection;

        // store for every vertex of that face
        for (uint b = 0; b < 3; ++b) {
            uint p;
            glm::vec3 normal;
            if (b == 0)      p = p0;
            else if (b == 1) p = p1;
            else          p = p2;

            if (data.normals.size() > p) normal = data.normals[p];
            else                        normal = glm::vec3(0.0f);

            // project tangent and bitangent into the plane formed by the vertex' normal
            glm::vec3 localTangent(tangent - normal * (tangent   * normal));
            glm::vec3 localBitangent(bitangent - normal * (bitangent * normal));
            localTangent = glm::normalize(localTangent);
            localBitangent = glm::normalize(localBitangent);

            // reconstruct tangent/bitangent according to normal and bitangent/tangent when it's infinite or NaN.
            bool invalid_tangent = IsSpecialFloat(localTangent);
            bool invalid_bitangent = IsSpecialFloat(localBitangent);
            if (invalid_tangent != invalid_bitangent) {
                if (invalid_tangent) localTangent = glm::normalize(glm::cross(normal, localBitangent));
                else                 localBitangent = glm::normalize(glm::cross(localTangent, normal));
            }
            data.tangents.push_back(localTangent);
            data.binormals.push_back(localBitangent);
        }
    }
    for (uint i = 0; i < data.points.size(); ++i) {
        //hmm.. should b and t be swapped here?
        glm::vec3& n = data.normals[i];
        glm::vec3& b = data.tangents[i];
        glm::vec3& t = data.binormals[i];
        t = glm::normalize(t - n * glm::dot(n, t)); // Gram-Schmidt orthogonalize
    }
};



epriv::MeshCollisionFactory::MeshCollisionFactory(Mesh& _mesh, VertexData& data) :m_Mesh(_mesh) {
    m_ConvexHullData = nullptr;
    m_ConvesHullShape = nullptr;
    m_TriangleStaticData = nullptr;
    m_TriangleStaticShape = nullptr;
    _initConvexData(data);
    _initTriangleData(data);
}
epriv::MeshCollisionFactory::~MeshCollisionFactory(){
    SAFE_DELETE(m_ConvexHullData);
    SAFE_DELETE(m_ConvesHullShape);
    SAFE_DELETE(m_TriangleStaticData);
    SAFE_DELETE(m_TriangleStaticShape);
}
void epriv::MeshCollisionFactory::_initConvexData(VertexData& data) {
    auto& positions = data.getData<glm::vec3>(0);
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
            m_ConvesHullShape->addPoint(btVector3(ptsArray[i].x(), ptsArray[i].y(), ptsArray[i].z()));
        }
        m_ConvesHullShape->setMargin(0.001f);
        m_ConvesHullShape->recalcLocalAabb();
    }
}
void epriv::MeshCollisionFactory::_initTriangleData(VertexData& data) {
    if (!m_TriangleStaticData) {
        auto& positions = data.getData<glm::vec3>(0);
        vector<glm::vec3> triangles;
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
                btVector3 v1 = Math::btVectorFromGLM(tri[0]);
                btVector3 v2 = Math::btVectorFromGLM(tri[1]);
                btVector3 v3 = Math::btVectorFromGLM(tri[2]);
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
btSphereShape* epriv::MeshCollisionFactory::buildSphereShape() {
    btSphereShape* sphere = new btSphereShape(m_Mesh.getRadius());
    sphere->setMargin(0.001f);
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