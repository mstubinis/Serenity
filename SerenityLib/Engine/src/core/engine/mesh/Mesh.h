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
    class  Renderer;
};

#include <core/engine/mesh/VertexData.h>
#include <core/engine/mesh/MeshIncludes.h>

#include <core/engine/resources/Engine_ResourceBasic.h>
#include <core/engine/events/Engine_EventObject.h>
#include <core/engine/physics/PhysicsIncludes.h>
#include <core/engine/model/ModelInstance.h>

#include <unordered_map>
#include <functional>

#include <assimp/scene.h>

namespace Engine::priv{
    struct InternalMeshPublicInterface final {
        static void InitBlankMesh(Mesh&);
        static void LoadGPU(Mesh&);
        static void UnloadCPU(Mesh&);
        static void UnloadGPU(Mesh&);
        static bool SupportsInstancing();
        static btCollisionShape* BuildCollision(ModelInstance*, const CollisionType::Type, const bool isCompoundChild = false);
        static btCollisionShape* BuildCollision(Mesh*, const CollisionType::Type, const bool isCompoundChild = false);

        static void FinalizeVertexData(Mesh&, MeshImportedData& data);
        static void TriangulateComponentIndices(Mesh&, MeshImportedData& data, std::vector<std::vector<uint>>& indices, const unsigned char flags);
        static void CalculateRadius(Mesh&);
    };
};

class Mesh final: public EngineResource, public EventObserver, public Engine::NonCopyable, public Engine::NonMoveable {
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
    private:
        std::function<void(Mesh*)>             m_CustomBindFunctor   = [](Mesh*) {};
        std::function<void(Mesh*)>             m_CustomUnbindFunctor = [](Mesh*) {};

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

        template<typename T>
        void setCustomBindFunctor(const T& functor) {
            m_CustomBindFunctor = std::bind(std::move(functor), std::placeholders::_1);
        }
        template<typename T>
        void setCustomUnbindFunctor(const T& functor) {
            m_CustomUnbindFunctor = std::bind(std::move(functor), std::placeholders::_1);
        }

        bool operator==(const bool rhs) const;
        explicit operator bool() const;

        std::unordered_map<std::string, Engine::priv::AnimationData>& animationData();
        const glm::vec3& getRadiusBox() const;
        const float getRadius() const;
        const VertexData& getVertexData() const;

        void onEvent(const Event& e);

        void load();
        void unload();

        template<typename T> 
        void modifyVertices(const unsigned int attributeIndex, std::vector<T>& modifications, const unsigned int MeshModifyFlags = MeshModifyFlags::Default | MeshModifyFlags::UploadToGPU) {
            auto& vertexDataStructure = const_cast<VertexData&>(*m_VertexData);
            bool uploadToGPU = false;
            bool orphan = false;
            if (MeshModifyFlags & MeshModifyFlags::Orphan) {
                orphan = true;
            }
            if (MeshModifyFlags & MeshModifyFlags::UploadToGPU) {
                uploadToGPU = true;
            }
            vertexDataStructure.setData<T>(attributeIndex, modifications, uploadToGPU, orphan);
        }
        void modifyIndices(std::vector<unsigned int>& modifiedIndices, const unsigned int MeshModifyFlags = MeshModifyFlags::Default | MeshModifyFlags::UploadToGPU) {
            auto& vertexDataStructure = const_cast<VertexData&>(*m_VertexData);
            bool uploadToGPU = false;
            bool orphan = false;
            bool recalcTriangles = false;
            if (MeshModifyFlags & MeshModifyFlags::Orphan) {
                orphan = true;
            }
            if (MeshModifyFlags & MeshModifyFlags::UploadToGPU) {
                uploadToGPU = true;
            }
            if (MeshModifyFlags & MeshModifyFlags::RecalculateTriangles) {
                recalcTriangles = true;
            }
            vertexDataStructure.setIndices(modifiedIndices, uploadToGPU, orphan, recalcTriangles);
        }

        void sortTriangles(const Camera& camera, ModelInstance& instance, const glm::mat4& bodyModelMatrix, const SortingMode::Mode& sortMode);
};
#endif