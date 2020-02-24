#pragma once
#ifndef ENGINE_MESH_H
#define ENGINE_MESH_H

class  Mesh;
class  Camera;
class  btHeightfieldTerrainShape;
class  btCollisionShape;
class  Collision;
namespace Engine::priv {
    class  MeshLoader;
    class  MeshSkeleton;
    class  MeshCollisionFactory;
    struct MeshImportedData;
    struct DefaultMeshBindFunctor;
    struct DefaultMeshUnbindFunctor;
    struct BoneNode;
    class  AnimationData;
    struct InternalMeshRequestPublicInterface;
    class  ModelInstanceAnimation;
};

#include <core/engine/mesh/VertexData.h>
#include <core/engine/mesh/MeshIncludes.h>

#include <core/engine/BindableResource.h>
#include <core/engine/events/Engine_EventObject.h>
#include <core/engine/physics/PhysicsIncludes.h>
#include <core/engine/model/ModelInstance.h>

#include <unordered_map>

#include <assimp/scene.h>

namespace Engine::priv{
    struct InternalMeshPublicInterface final {
        static void InitBlankMesh(Mesh&);
        static void LoadGPU(Mesh&);
        static void UnloadCPU(Mesh&);
        static void UnloadGPU(Mesh&);
        static bool SupportsInstancing();
        static btCollisionShape* BuildCollision(ModelInstance*, const CollisionType::Type&, const bool isCompoundChild = false);
        static btCollisionShape* BuildCollision(Mesh*, const CollisionType::Type&, const bool isCompoundChild = false);

        static void FinalizeVertexData(Mesh&, MeshImportedData& data);
        static void TriangulateComponentIndices(Mesh&, MeshImportedData& data, std::vector<std::vector<uint>>& indices, const unsigned char flags);
        static void CalculateRadius(Mesh&);
    };
};

class Mesh final: public BindableResource, public EventObserver, public Engine::NonCopyable, public Engine::NonMoveable {
    friend struct ::Engine::priv::InternalMeshPublicInterface;
    friend struct ::Engine::priv::InternalMeshRequestPublicInterface;
    friend struct ::Engine::priv::DefaultMeshBindFunctor;
    friend struct ::Engine::priv::DefaultMeshUnbindFunctor;
    friend class  ::Engine::priv::AnimationData;
    friend class  ::Engine::priv::MeshSkeleton;
    friend class  ::Engine::priv::MeshLoader;
    friend class  ::Engine::priv::MeshCollisionFactory;
    friend class  ::Engine::priv::ModelInstanceAnimation;
    friend class  Collision;
    private:
        VertexData*                            m_VertexData;
        Engine::priv::MeshCollisionFactory*    m_CollisionFactory;
        Engine::priv::MeshSkeleton*            m_Skeleton;
        std::string                            m_File;
        glm::vec3                              m_radiusBox;
        float                                  m_radius;
        float                                  m_Threshold;

        Mesh();
    public:
        Mesh(const std::string& name, const btHeightfieldTerrainShape& heightfield, float threshold);
        Mesh(VertexData*, const std::string& name, float threshold = 0.0005f);
        Mesh(const std::string& name,float width, float height,float threshold); //plane
        Mesh(const std::string& fileOrData, float threshold = 0.0005f); //file or data
        ~Mesh();

        bool operator==(const bool& rhs) const;
        explicit operator bool() const;

        std::unordered_map<std::string, Engine::priv::AnimationData>& animationData();
        const glm::vec3& getRadiusBox() const;
        const float getRadius() const;
        const VertexData& getVertexData() const;

        void onEvent(const Event& e);

        void load();
        void unload();

        template<typename T> 
        void modifyVertices(const uint& attributeIndex, std::vector<T>& modifications, const uint MeshModifyFlags = MeshModifyFlags::Default | MeshModifyFlags::UploadToGPU) {
            auto& vertexDataStructure = const_cast<VertexData&>(*m_VertexData);
            bool uploadToGPU = false;
            bool orphan = false;
            if (MeshModifyFlags & MeshModifyFlags::Orphan)
                orphan = true;
            if (MeshModifyFlags & MeshModifyFlags::UploadToGPU)
                uploadToGPU = true;
            vertexDataStructure.setData<T>(attributeIndex, modifications, uploadToGPU, orphan);
        }
        void modifyIndices(std::vector<ushort>& modifiedIndices, const uint MeshModifyFlags = MeshModifyFlags::Default | MeshModifyFlags::UploadToGPU) {
            auto& vertexDataStructure = const_cast<VertexData&>(*m_VertexData);
            bool uploadToGPU = false;
            bool orphan = false;
            bool recalcTriangles = false;
            if (MeshModifyFlags & MeshModifyFlags::Orphan)
                orphan = true;
            if (MeshModifyFlags & MeshModifyFlags::UploadToGPU)
                uploadToGPU = true;
            if (MeshModifyFlags & MeshModifyFlags::RecalculateTriangles)
                recalcTriangles = true;
            vertexDataStructure.setIndices(modifiedIndices, uploadToGPU, orphan, recalcTriangles);
        }

        void sortTriangles(const Camera& camera, ModelInstance& instance, const glm::mat4& bodyModelMatrix, const SortingMode::Mode& sortMode);
};
#endif