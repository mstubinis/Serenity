#pragma once
#ifndef ENGINE_MESH_H
#define ENGINE_MESH_H

#include <core/engine/mesh/VertexData.h>
#include <core/engine/mesh/MeshIncludes.h>

#include <core/engine/BindableResource.h>
#include <core/engine/events/Engine_EventObject.h>
#include <core/engine/physics/PhysicsIncludes.h>
#include <core/engine/model/ModelInstance.h>

#include <unordered_map>

#include <assimp/scene.h>

class  Mesh;
class  Camera;
class  btHeightfieldTerrainShape;
class  btCollisionShape;
class  Collision;
namespace Engine{
    namespace epriv{
        class  MeshLoader;
        class  MeshSkeleton;
        class  MeshCollisionFactory;
        struct MeshImportedData;
        struct DefaultMeshBindFunctor;
        struct DefaultMeshUnbindFunctor;
        struct BoneNode;
        class  AnimationData;
        struct InternalMeshRequestPublicInterface;
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
};

class Mesh final: public BindableResource, public EventObserver{
    friend struct ::Engine::epriv::InternalMeshPublicInterface;
    friend struct ::Engine::epriv::InternalMeshRequestPublicInterface;
    friend struct ::Engine::epriv::DefaultMeshBindFunctor;
    friend struct ::Engine::epriv::DefaultMeshUnbindFunctor;
    friend class  ::Engine::epriv::AnimationData;
    friend class  ::Engine::epriv::MeshSkeleton;
    friend class  ::Engine::epriv::MeshLoader;
    friend class  ::Engine::epriv::MeshCollisionFactory;
    private:
        VertexData*                            m_VertexData;
        Engine::epriv::MeshCollisionFactory*   m_CollisionFactory;
        Engine::epriv::MeshSkeleton*           m_Skeleton;
        std::string                            m_File;
        glm::vec3                              m_radiusBox;
        float                                  m_radius;
        float                                  m_threshold;

        Mesh();
    public:
        static Mesh *Plane, *Cube, *Triangle; //loaded in renderer

        Mesh(const std::string& name, const btHeightfieldTerrainShape& heightfield, float threshold);
        Mesh(VertexData*, const std::string& name, float threshold = 0.0005f);
        Mesh(const std::string& name,float width, float height,float threshold); //plane
        Mesh(const std::string& fileOrData, float threshold = 0.0005f); //file or data
        ~Mesh();

        Mesh(const Mesh&)                      = delete;
        Mesh& operator=(const Mesh&)           = delete;
        Mesh(Mesh&& other) noexcept            = delete;
        Mesh& operator=(Mesh&& other) noexcept = delete;


        std::unordered_map<std::string, Engine::epriv::AnimationData>& animationData();
        const glm::vec3& getRadiusBox() const;
        const float getRadius() const;
        const VertexData& getVertexData() const;

        void onEvent(const Event& e);

        void load();
        void unload();

        template<typename T> void modifyVertices(const uint& attributeIndex, std::vector<T>& modifications, const uint MeshModifyFlags = MeshModifyFlags::Default | MeshModifyFlags::UploadToGPU) {
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

        void sortTriangles(Camera& camera, ModelInstance& instance, const glm::mat4& bodyModelMatrix, const SortingMode::Mode& sortMode);

        void render(const bool instancing = false, const ModelDrawingMode::Mode = ModelDrawingMode::Triangles);
        void playAnimation(std::vector<glm::mat4>&,const std::string& animationName,float time);
};
#endif