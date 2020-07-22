#pragma once
#ifndef ENGINE_MESH_COLLISION_FACTORY_H
#define ENGINE_MESH_COLLISION_FACTORY_H

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
struct btTriangleInfoMap;

namespace Engine::priv {
    class MeshCollisionFactory final : public Engine::NonCopyable, public Engine::NonMoveable {
        private:
            Mesh&                          m_Mesh;
            btShapeHull*                   m_ConvexHullData      = nullptr;
            btConvexHullShape*             m_ConvesHullShape     = nullptr;
            btTriangleMesh*                m_TriangleStaticData  = nullptr;
            btBvhTriangleMeshShape*        m_TriangleStaticShape = nullptr;
            btTriangleInfoMap*             m_TriangleInfoMap     = nullptr;

            void initConvexData(VertexData& data, std::vector<glm::vec3>& positions);
            void initTriangleData(VertexData& data, std::vector<glm::vec3>& positions);
        public:
            MeshCollisionFactory(Mesh& mesh);
            ~MeshCollisionFactory();

            btMultiSphereShape*            buildSphereShape(ModelInstance* modelInstance, const bool isCompoundChild = false);
            btBoxShape*                    buildBoxShape(ModelInstance* modelInstance, const bool isCompoundChild = false);
            btUniformScalingShape*         buildConvexHull(ModelInstance* modelInstance, const bool isCompoundChild = false);
            btScaledBvhTriangleMeshShape*  buildTriangleShape(ModelInstance* modelInstance, const bool isCompoundChild = false);
            btGImpactMeshShape*            buildTriangleShapeGImpact(ModelInstance* modelInstance, const bool isCompoundChild = false);
    };
};

#endif