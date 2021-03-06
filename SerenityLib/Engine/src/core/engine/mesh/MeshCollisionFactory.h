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

#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btTriangleInfoMap.h>
#include <core/engine/mesh/MeshIncludes.h>
#include <core/engine/resources/Handle.h>

namespace Engine::priv {
    class MeshCollisionFactory final {
        friend struct MeshCPUData;
        private:
            std::unique_ptr<btShapeHull>             m_ConvexHullData;
            std::unique_ptr<btConvexHullShape>       m_ConvesHullShape;
            std::unique_ptr<btTriangleMesh>          m_TriangleStaticData;
            std::unique_ptr<btBvhTriangleMeshShape>  m_TriangleStaticShape;
            std::unique_ptr<btTriangleInfoMap>       m_TriangleInfoMap;
            MeshCPUData*                             m_CPUData = nullptr;

            void internal_init_convex_data(VertexData& data, std::vector<glm::vec3>& positions);
            void internal_init_triangle_data(VertexData& data, std::vector<glm::vec3>& positions);
        public:
            MeshCollisionFactory() = default;
            MeshCollisionFactory(MeshCPUData& cpuData, MeshCollisionLoadingFlag::Flag = MESH_COLLISION_FACTORY_DEFAULT_LOAD_FLAG);
            MeshCollisionFactory(const MeshCollisionFactory& other)                = delete;
            MeshCollisionFactory& operator=(const MeshCollisionFactory& other)     = delete;
            MeshCollisionFactory(MeshCollisionFactory&& other) noexcept;
            MeshCollisionFactory& operator=(MeshCollisionFactory&& other) noexcept;
            ~MeshCollisionFactory() = default;

            btMultiSphereShape*            buildSphereShape(ModelInstance* modelInstance, bool isCompoundChild = false);
            btBoxShape*                    buildBoxShape(ModelInstance* modelInstance, bool isCompoundChild = false);
            btUniformScalingShape*         buildConvexHull(ModelInstance* modelInstance, bool isCompoundChild = false);
            btScaledBvhTriangleMeshShape*  buildTriangleShape(ModelInstance* modelInstance, bool isCompoundChild = false);
            btGImpactMeshShape*            buildTriangleShapeGImpact(ModelInstance* modelInstance, bool isCompoundChild = false);
    };
};

#endif