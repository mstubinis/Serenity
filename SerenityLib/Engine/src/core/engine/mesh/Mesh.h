#pragma once
#ifndef ENGINE_MESH_H
#define ENGINE_MESH_H

class  Mesh;
class  Camera;
class  btHeightfieldTerrainShape;
class  btCollisionShape;
class  Collision;
class  Terrain;
class  TerrainHeightfieldShape;
class  SMSH_File;
namespace Engine::priv {
    class  MeshLoader;
    class  MeshSkeleton;
    class  MeshCollisionFactory;
    struct MeshImportedData;
    struct DefaultMeshBindFunctor;
    struct DefaultMeshUnbindFunctor;
    struct MeshInfoNode;
    class  AnimationData;
    struct InternalMeshRequestPublicInterface;
    class  ModelInstanceAnimation;
    class  Renderer;
};

#include <core/engine/mesh/VertexData.h>
#include <core/engine/mesh/MeshIncludes.h>

#include <core/engine/resources/Resource.h>
#include <core/engine/events/Observer.h>
#include <core/engine/physics/PhysicsIncludes.h>
#include <core/engine/model/ModelInstance.h>

#include <assimp/scene.h>

namespace Engine::priv{
    struct InternalMeshPublicInterface final {
        static void InitBlankMesh(Mesh&);
        static void LoadGPU(Mesh&);
        static void UnloadCPU(Mesh&);
        static void UnloadGPU(Mesh&);
        static bool SupportsInstancing();
        static btCollisionShape* BuildCollision(ModelInstance*, CollisionType::Type, bool isCompoundChild = false);
        static btCollisionShape* BuildCollision(Mesh*, CollisionType::Type, bool isCompoundChild = false);

        static void FinalizeVertexData(Mesh&, MeshImportedData& data);
        static void TriangulateComponentIndices(Mesh&, MeshImportedData& data, std::vector<std::vector<uint>>& indices, unsigned char flags);
        static void CalculateRadius(Mesh&);
    };
};

class Mesh final: public Resource, public Observer, public Engine::NonCopyable, public Engine::NonMoveable {
    friend struct Engine::priv::InternalMeshPublicInterface;
    friend struct Engine::priv::InternalMeshRequestPublicInterface;
    friend struct Engine::priv::DefaultMeshBindFunctor;
    friend struct Engine::priv::DefaultMeshUnbindFunctor;
    friend class  Engine::priv::AnimationData;
    friend class  Engine::priv::MeshSkeleton;
    friend class  Engine::priv::MeshLoader;
    friend class  Engine::priv::MeshCollisionFactory;
    friend class  Engine::priv::ModelInstanceAnimation;
    friend class  Engine::priv::Renderer;
    friend class  Collision;
    friend class  Terrain;
    friend class  SMSH_File;
    private:
        std::function<void(Mesh*, const Engine::priv::Renderer*)> m_CustomBindFunctor   = [](Mesh*, const Engine::priv::Renderer*) {};
        std::function<void(Mesh*, const Engine::priv::Renderer*)> m_CustomUnbindFunctor = [](Mesh*, const Engine::priv::Renderer*) {};
        Engine::priv::MeshInfoNode*            m_RootNode            = nullptr;
        VertexData*                            m_VertexData          = nullptr;
        Engine::priv::MeshCollisionFactory*    m_CollisionFactory    = nullptr;
        Engine::priv::MeshSkeleton*            m_Skeleton            = nullptr;
        std::string                            m_File                = "";
        glm::vec3                              m_radiusBox           = glm::vec3(0.0f);
        float                                  m_radius              = 0.0f;
        float                                  m_Threshold           = 0.0005f;

        void internal_recalc_indices_from_terrain(const Terrain& terrain);
        void internal_build_from_terrain(const Terrain& terrain);

        Mesh();
        Mesh(const std::string& name, const Terrain& terrain, float threshold);
    public:
        Mesh(VertexData*, const std::string& name, float threshold = 0.0005f);
        Mesh(const std::string& name, float width, float height, float threshold); //plane
        Mesh(const std::string& fileOrData, float threshold = 0.0005f); //file or data
        ~Mesh();

        template<typename T> void setCustomBindFunctor(const T& functor) {
            m_CustomBindFunctor = std::bind(std::move(functor), std::placeholders::_1, std::placeholders::_2);
        }
        template<typename T> void setCustomUnbindFunctor(const T& functor) {
            m_CustomUnbindFunctor = std::bind(std::move(functor), std::placeholders::_1, std::placeholders::_2);
        }

        bool operator==(bool rhs) const { return (rhs) ? (bool)m_VertexData : (bool)!m_VertexData; }
        explicit operator bool() const { return (bool)m_VertexData; }

        std::unordered_map<std::string, Engine::priv::AnimationData>& animationData();
        const glm::vec3& getRadiusBox() const;
        float getRadius() const;
        const VertexData& getVertexData() const;

        void onEvent(const Event& e);

        void load();
        void unload();

        template<typename T> 
        void modifyVertices(unsigned int attributeIndex, const T* modifications, size_t bufferCount, unsigned int MeshModifyFlags = MeshModifyFlags::Default | MeshModifyFlags::UploadToGPU) {
            m_VertexData->setData<T>(attributeIndex, modifications, bufferCount, 
                MeshModifyFlags & MeshModifyFlags::UploadToGPU, 
                MeshModifyFlags & MeshModifyFlags::Orphan
            );
        }
        void modifyIndices(const unsigned int* modifiedIndices, size_t bufferCount, unsigned int MeshModifyFlags = MeshModifyFlags::Default | MeshModifyFlags::UploadToGPU) {
            m_VertexData->setIndices(modifiedIndices, bufferCount, 
                MeshModifyFlags & MeshModifyFlags::UploadToGPU, 
                MeshModifyFlags & MeshModifyFlags::Orphan, 
                MeshModifyFlags & MeshModifyFlags::RecalculateTriangles
            );
        }

        void sortTriangles(const Camera& camera, ModelInstance& instance, const glm::mat4& bodyModelMatrix, SortingMode sortMode);
};
#endif