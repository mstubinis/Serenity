#pragma once
#ifndef ENGINE_MESH_LOADING_INCLUDE_GUARD
#define ENGINE_MESH_LOADING_INCLUDE_GUARD

#include <core/engine/Engine_Utils.h>
#include <core/engine/mesh/Skeleton.h>

struct VertexData;
class  btShapeHull;
class  btConvexHullShape;
class  btTriangleMesh;
class  btBvhTriangleMeshShape;
class  btSphereShape;
class  btBoxShape;
class  btUniformScalingShape;
class  btScaledBvhTriangleMeshShape;
class  btGImpactMeshShape;

struct aiScene;
struct aiNode;

namespace Engine {
    namespace epriv {
        class MeshLoader final {
            friend class ::Mesh;
            //friend class ::Engine::epriv::MeshSkeleton;
            public:
                static void LoadInternal(epriv::MeshSkeleton* skeleton, epriv::MeshImportedData& data, const std::string& file);
                static void LoadProcessNode(epriv::MeshSkeleton* skeleton, epriv::MeshImportedData& data, const aiScene& scene, const aiNode& node, const aiNode& root, std::unordered_map<std::string, epriv::BoneNode*>& _map);
                static void LoadPopulateGlobalNodes(const aiNode& node, std::unordered_map<std::string, epriv::BoneNode*>& _map);


                static VertexData* LoadFrom_OBJCC(std::string& filename);
                static void SaveTo_OBJCC(VertexData& data, std::string filename);


                static bool IsNear(float& v1, float& v2, const float& threshold);
                static bool IsNear(glm::vec2& v1, glm::vec2& v2, const float& threshold);
                static bool IsNear(glm::vec3& v1, glm::vec3& v2, const float& threshold);
                static bool IsSpecialFloat(float& _float);
                static bool IsSpecialFloat(glm::vec2& _vector);
                static bool IsSpecialFloat(glm::vec3& _vector);
                static bool GetSimilarVertexIndex(glm::vec3& in_pos, glm::vec2& in_uv, glm::vec3& in_norm, std::vector<glm::vec3>& pts, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& norms, unsigned short& result, const float& threshold);
                static void CalculateTBNAssimp(MeshImportedData& data);
        };
        //slightly missleading with the name. its a class (per mesh) that allows for bullet physics collision objects to be created using the mesh data
        class MeshCollisionFactory final {
            friend class ::Mesh;
            private:
                Mesh&                          m_Mesh;
                btShapeHull*                   m_ConvexHullData;
                btConvexHullShape*             m_ConvesHullShape;
                btTriangleMesh*                m_TriangleStaticData;
                btBvhTriangleMeshShape*        m_TriangleStaticShape;

                void  _initConvexData(VertexData& data);
                void  _initTriangleData(VertexData& data);
            public:

                MeshCollisionFactory(Mesh& _mesh, VertexData& data);
                ~MeshCollisionFactory();

                btSphereShape*                 buildSphereShape();
                btBoxShape*                    buildBoxShape();
                btUniformScalingShape*         buildConvexHull();
                btScaledBvhTriangleMeshShape*  buildTriangleShape();
                btGImpactMeshShape*            buildTriangleShapeGImpact();
        };
        enum MeshLoadingLoadWhat {
            LOAD_POINTS = 1,
            LOAD_UVS = 2,
            LOAD_NORMALS = 4,
            LOAD_FACES = 8,
            LOAD_TBN = 16,
            //= 32,
            //= 64,
            //= 128,
            //= 256,
            //= 512,
            //= 1024,
            //= 2048,
            //= 4096,
        };
    };
};


#endif