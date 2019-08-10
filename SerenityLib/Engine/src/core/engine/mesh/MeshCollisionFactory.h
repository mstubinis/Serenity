#pragma once
#ifndef ENGINE_MESH_COLLISION_FACTORY_INCLUDE_GUARD
#define ENGINE_MESH_COLLISION_FACTORY_INCLUDE_GUARD

struct VertexData;
class  Mesh;
class  btShapeHull;
//class  btVector3;
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
    // slightly missleading with the name. its a class (per mesh) that allows for
    // bullet physics collision objects to be created using the mesh data
    class MeshCollisionFactory final {
        private:
            Mesh&                          m_Mesh;
            //btVector3*                     m_Zero;
            btShapeHull*                   m_ConvexHullData;
            btConvexHullShape*             m_ConvesHullShape;
            btTriangleMesh*                m_TriangleStaticData;
            btBvhTriangleMeshShape*        m_TriangleStaticShape;

            void  _initConvexData(VertexData& data);
            void  _initTriangleData(VertexData& data);
        public:
            MeshCollisionFactory(Mesh& _mesh);
            ~MeshCollisionFactory();

            btMultiSphereShape*            buildSphereShape();
            btBoxShape*                    buildBoxShape();
            btUniformScalingShape*         buildConvexHull();
            btScaledBvhTriangleMeshShape*  buildTriangleShape();
            btGImpactMeshShape*            buildTriangleShapeGImpact();
    };
};
};

#endif