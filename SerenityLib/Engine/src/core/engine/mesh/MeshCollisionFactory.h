#pragma once
#ifndef ENGINE_MESH_COLLISION_FACTORY_INCLUDE_GUARD
#define ENGINE_MESH_COLLISION_FACTORY_INCLUDE_GUARD

struct VertexData;
class  Mesh;
class  ModelInstance;
class  btShapeHull;
class  btMultiSphereShape;
class  btConvexHullShape;
class  btTriangleMesh;
class  btBvhTriangleMeshShape;
class  btSphereShape;
class  btBoxShape;
class  btUniformScalingShape;
class  btScaledBvhTriangleMeshShape;
class  btGImpactMeshShape;

namespace Engine {
namespace epriv {
    class MeshCollisionFactory final {
        private:
            Mesh&                          m_Mesh;
            btShapeHull*                   m_ConvexHullData;
            btConvexHullShape*             m_ConvesHullShape;
            btTriangleMesh*                m_TriangleStaticData;
            btBvhTriangleMeshShape*        m_TriangleStaticShape;

            void  _initConvexData(VertexData& data);
            void  _initTriangleData(VertexData& data);
        public:
            MeshCollisionFactory(Mesh& _mesh);
            ~MeshCollisionFactory();
            MeshCollisionFactory(const MeshCollisionFactory&)                      = delete;
            MeshCollisionFactory& operator=(const MeshCollisionFactory&)           = delete;
            MeshCollisionFactory(MeshCollisionFactory&& other) noexcept            = delete;
            MeshCollisionFactory& operator=(MeshCollisionFactory&& other) noexcept = delete;

            btMultiSphereShape*            buildSphereShape(ModelInstance* modelInstance, const bool isCompoundChild = false);
            btBoxShape*                    buildBoxShape(ModelInstance* modelInstance, const bool isCompoundChild = false);
            btUniformScalingShape*         buildConvexHull(ModelInstance* modelInstance, const bool isCompoundChild = false);
            btScaledBvhTriangleMeshShape*  buildTriangleShape(ModelInstance* modelInstance, const bool isCompoundChild = false);
            btGImpactMeshShape*            buildTriangleShapeGImpact(ModelInstance* modelInstance, const bool isCompoundChild = false);
    };
};
};

#endif