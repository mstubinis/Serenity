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
struct MeshCPUData;
class  ModelInstance;
namespace Engine::priv {
    class  MeshLoader;
    class  MeshSkeleton;
    class  MeshCollisionFactory;
    class  MeshImportedData;
    struct MeshInfoNode;
    class  AnimationData;
    struct PublicMeshRequest;
    class  ModelInstanceAnimation;
    class  ModelInstanceAnimationContainer;
    class  RenderModule;
};

#include <serenity/resources/mesh/VertexData.h>
#include <serenity/resources/mesh/animation/Skeleton.h>
#include <serenity/resources/mesh/MeshCollisionFactory.h>
#include <serenity/resources/Resource.h>
#include <serenity/events/Observer.h>
#include <serenity/physics/PhysicsIncludes.h>
#include <serenity/renderer/RendererIncludes.h>
#include <vector>
#include <queue>
#include <serenity/dependencies/glm.h>

namespace Engine::priv{
    class PublicMesh final {
        private:
            static btCollisionShape* internal_build_collision(Handle meshHandle, ModelInstance*, CollisionType, bool isCompoundChild) noexcept;
        public:
            static void InitBlankMesh(Mesh&);
            static void LoadGPU(Mesh&);
            static void UnloadCPU(Mesh&);
            static void UnloadGPU(Mesh&);
            static bool SupportsInstancing();
            static btCollisionShape* BuildCollision(ModelInstance*, CollisionType, bool isCompoundChild = false);
            static btCollisionShape* BuildCollision(Handle meshHandle, CollisionType, bool isCompoundChild = false);

            static void FinalizeVertexData(Handle meshHandle, MeshImportedData& data);
            static void FinalizeVertexData(MeshCPUData& cpuData, MeshImportedData& data);

            static void CalculateRadius(Handle meshHandle);
    };
};
struct MeshNodeData final {
    std::vector<uint16_t>                   m_NodeHeirarchy;
    std::vector<Engine::priv::MeshInfoNode> m_Nodes;
    std::vector<glm::mat4>                  m_NodeTransforms;
};
struct MeshCPUData final {
    MeshNodeData                                   m_NodeData;
    std::string                                    m_File;
    glm::vec3                                      m_RadiusBox        = glm::vec3{ 0.0f };
    mutable Engine::priv::MeshSkeleton*            m_Skeleton         = nullptr;
    mutable Engine::priv::MeshCollisionFactory*    m_CollisionFactory = nullptr;
    mutable VertexData*                            m_VertexData       = nullptr;
    float                                          m_Radius           = 0.0f;
    float                                          m_Threshold        = MESH_DEFAULT_THRESHOLD;

    MeshCPUData() = default;
    MeshCPUData(const MeshCPUData&);
    MeshCPUData& operator=(const MeshCPUData&);
    MeshCPUData(MeshCPUData&&) noexcept;
    MeshCPUData& operator=(MeshCPUData&&) noexcept;
    ~MeshCPUData() {
        SAFE_DELETE(m_Skeleton);
        SAFE_DELETE(m_CollisionFactory);
        SAFE_DELETE(m_VertexData);
    }
    void internal_transfer_cpu_datas();
    void internal_calculate_radius();
};

class Mesh final: public Resource, public Observer {
    friend class  Engine::priv::PublicMesh;
    friend struct Engine::priv::PublicMeshRequest;
    friend class  Engine::priv::AnimationData;
    friend class  Engine::priv::MeshSkeleton;
    friend class  Engine::priv::MeshLoader;
    friend class  Engine::priv::MeshCollisionFactory;
    friend class  Engine::priv::ModelInstanceAnimation;
    friend class  Engine::priv::ModelInstanceAnimationContainer;
    friend class  Engine::priv::RenderModule;
    friend class  Collision;
    friend class  Terrain;
    friend class  SMSH_File;

    using BindFunc   = void(*)(Mesh*, const Engine::priv::RenderModule*);
    using UnbindFunc = void(*)(Mesh*, const Engine::priv::RenderModule*);

    private:
        MeshCPUData    m_CPUData;
        BindFunc       m_CustomBindFunctor   = [](Mesh*, const Engine::priv::RenderModule*) {};
        UnbindFunc     m_CustomUnbindFunctor = [](Mesh*, const Engine::priv::RenderModule*) {};

        void internal_recalc_indices_from_terrain(const Terrain& terrain);
        void internal_build_from_terrain(const Terrain& terrain);
    public:
        Mesh();
        Mesh(VertexData&, std::string_view name, float threshold = MESH_DEFAULT_THRESHOLD);
        Mesh(std::string_view name, float width, float height, float threshold); //plane
        Mesh(std::string_view fileOrData, float threshold = MESH_DEFAULT_THRESHOLD); //file or data
        Mesh(std::string_view name, const Terrain& terrain, float threshold);

        Mesh(const Mesh&)                 = delete;
        Mesh& operator=(const Mesh&)      = delete;
        Mesh(Mesh&&) noexcept;
        Mesh& operator=(Mesh&&) noexcept;
        ~Mesh();

        inline void setCustomBindFunctor(const BindFunc& functor) noexcept { m_CustomBindFunctor = functor; }
        inline void setCustomUnbindFunctor(const UnbindFunc& functor) noexcept { m_CustomUnbindFunctor = functor; }
        inline void setCustomBindFunctor(BindFunc&& functor) noexcept { m_CustomBindFunctor = std::move(functor); }
        inline void setCustomUnbindFunctor(UnbindFunc&& functor) noexcept { m_CustomUnbindFunctor = std::move(functor); }

        [[nodiscard]] Engine::priv::MeshSkeleton::AnimationDataMap& getAnimationData();
        [[nodiscard]] inline constexpr const glm::vec3& getRadiusBox() const noexcept { return m_CPUData.m_RadiusBox; }
        [[nodiscard]] inline constexpr float getRadius() const noexcept { return m_CPUData.m_Radius; }
        [[nodiscard]] inline constexpr const VertexData& getVertexData() const noexcept { return *m_CPUData.m_VertexData; }
        [[nodiscard]] inline Engine::priv::MeshSkeleton* getSkeleton() noexcept { return m_CPUData.m_Skeleton; }

        void onEvent(const Event&);

        void load();
        void unload();

        template<typename T> 
        void modifyVertices(uint32_t attrIdx, const T* modifications, size_t bufferCount, uint32_t MeshModifyFlags = MESH_DEFAULT_MODIFICATION_FLAGS) {
            m_CPUData.m_VertexData->setData<T>(attrIdx, modifications, bufferCount, (MeshModifyFlags::Flag)MeshModifyFlags);
        }
        void modifyIndices(const uint32_t* modifiedIndices, size_t bufferCount, uint32_t MeshModifyFlags = MESH_DEFAULT_MODIFICATION_FLAGS) {
            m_CPUData.m_VertexData->setIndices(modifiedIndices, bufferCount, (MeshModifyFlags::Flag)MeshModifyFlags);
        }

        void sortTriangles(const Camera& camera, ModelInstance& instance, const glm::mat4& bodyModelMatrix, SortingMode sortMode);
};
#endif