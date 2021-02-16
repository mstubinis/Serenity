#pragma once
#ifndef ENGINE_MESH_COLLISION_FACTORY_H
#define ENGINE_MESH_COLLISION_FACTORY_H

struct VertexData;
class  Mesh;
struct MeshCPUData;
class  ModelInstance;
class  btMultiSphereShape;
class  btSphereShape;
class  btBoxShape;
class  btUniformScalingShape;
class  btScaledBvhTriangleMeshShape;
class  btGImpactMeshShape;

#include <serenity/dependencies/glm.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btTriangleInfoMap.h>
#include <serenity/resources/mesh/MeshIncludes.h>
#include <serenity/resources/Handle.h>

namespace Engine::priv {
    class MeshCollisionFactory final {
        friend struct MeshCPUData;
        private:
            btConvexHullShape       m_ConvesHullShape;
            btTriangleMesh          m_TriangleStaticData;
            std::unique_ptr<btBvhTriangleMeshShape>  m_TriangleStaticShape;
            btTriangleInfoMap       m_TriangleInfoMap;
            MeshCPUData*            m_CPUData = nullptr;

            void internal_init_convex_data(std::vector<glm::vec3>& positions);
            void internal_init_triangle_data(VertexData& data, std::vector<glm::vec3>& positions);
        public:
            MeshCollisionFactory() = default;
            MeshCollisionFactory(MeshCPUData& cpuData, MeshCollisionLoadingFlag::Flag = MESH_COLLISION_FACTORY_DEFAULT_LOAD_FLAG);

            MeshCollisionFactory(const MeshCollisionFactory&)                = delete;
            MeshCollisionFactory& operator=(const MeshCollisionFactory&)     = delete;
            MeshCollisionFactory(MeshCollisionFactory&&) noexcept;
            MeshCollisionFactory& operator=(MeshCollisionFactory&&) noexcept;
  
            [[nodiscard]] btMultiSphereShape*            buildSphereShape(ModelInstance* modelInstance, bool isCompoundChild = false);
            [[nodiscard]] btBoxShape*                    buildBoxShape(ModelInstance* modelInstance, bool isCompoundChild = false);
            [[nodiscard]] btUniformScalingShape*         buildConvexHull(ModelInstance* modelInstance, bool isCompoundChild = false);
            [[nodiscard]] btScaledBvhTriangleMeshShape*  buildTriangleShape(ModelInstance* modelInstance, bool isCompoundChild = false);
            [[nodiscard]] btGImpactMeshShape*            buildTriangleShapeGImpact(ModelInstance* modelInstance, bool isCompoundChild = false);
    };
};

#endif