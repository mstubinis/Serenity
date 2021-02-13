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
    struct MeshImportedData;
    struct MeshInfoNode;
    class  AnimationData;
    struct PublicMeshRequest;
    class  ModelInstanceAnimation;
    class  RenderModule;
};

#include <serenity/resources/mesh/VertexData.h>
#include <serenity/resources/mesh/Skeleton.h>
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

struct MeshCPUData final {
    std::string                                    m_File;
    glm::vec3                                      m_RadiusBox        = glm::vec3(0.0f);
    mutable Engine::priv::MeshSkeleton*            m_Skeleton         = nullptr;
    mutable Engine::priv::MeshInfoNode*            m_RootNode         = nullptr;
    mutable Engine::priv::MeshCollisionFactory*    m_CollisionFactory = nullptr;
    mutable VertexData*                            m_VertexData       = nullptr;
    float                                          m_Radius           = 0.0f;
    float                                          m_Threshold        = MESH_DEFAULT_THRESHOLD;

    MeshCPUData() = default;
    MeshCPUData(const MeshCPUData& other);
    MeshCPUData& operator=(const MeshCPUData& other);
    MeshCPUData(MeshCPUData&& other) noexcept;
    MeshCPUData& operator=(MeshCPUData&& other) noexcept;
    ~MeshCPUData() {
        if (m_RootNode) {
            std::vector<Engine::priv::MeshInfoNode*> sortedNodes;
            std::queue<Engine::priv::MeshInfoNode*> q;
            q.push(m_RootNode);
            while (!q.empty()) {
                auto qSize = q.size();
                while (qSize--) {
                    auto* front = q.front();
                    sortedNodes.push_back(front);
                    for (const auto& child : front->Children) {
                        q.push(child.get());
                    }
                    q.pop();
                }
            }
            SAFE_DELETE_VECTOR(sortedNodes);
            m_RootNode = nullptr;
        }
        SAFE_DELETE(m_Skeleton);
        //SAFE_DELETE(m_RootNode);
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
    friend class  Engine::priv::RenderModule;
    friend class  Collision;
    friend class  Terrain;
    friend class  SMSH_File;

    using bind_func   = void(*)(Mesh*, const Engine::priv::RenderModule*);
    using unbind_func = void(*)(Mesh*, const Engine::priv::RenderModule*);

    private:
        bind_func       m_CustomBindFunctor   = [](Mesh*, const Engine::priv::RenderModule*) {};
        unbind_func     m_CustomUnbindFunctor = [](Mesh*, const Engine::priv::RenderModule*) {};

        MeshCPUData     m_CPUData;

        void internal_recalc_indices_from_terrain(const Terrain& terrain);
        void internal_build_from_terrain(const Terrain& terrain);
    public:
        Mesh();
        Mesh(VertexData&, const std::string& name, float threshold = MESH_DEFAULT_THRESHOLD);
        Mesh(const std::string& name, float width, float height, float threshold); //plane
        Mesh(const std::string& fileOrData, float threshold = MESH_DEFAULT_THRESHOLD); //file or data
        Mesh(const std::string& name, const Terrain& terrain, float threshold);

        Mesh(const Mesh& other)                 = delete;
        Mesh& operator=(const Mesh& other)      = delete;
        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;
        ~Mesh();

        inline void setCustomBindFunctor(const bind_func& functor) noexcept { m_CustomBindFunctor = functor; }
        inline void setCustomUnbindFunctor(const unbind_func& functor) noexcept { m_CustomUnbindFunctor = functor; }
        inline void setCustomBindFunctor(bind_func&& functor) noexcept { m_CustomBindFunctor = std::move(functor); }
        inline void setCustomUnbindFunctor(unbind_func&& functor) noexcept { m_CustomUnbindFunctor = std::move(functor); }

        [[nodiscard]] std::unordered_map<std::string, Engine::priv::AnimationData>& getAnimationData();
        [[nodiscard]] inline constexpr const glm::vec3& getRadiusBox() const noexcept { return m_CPUData.m_RadiusBox; }
        [[nodiscard]] inline constexpr float getRadius() const noexcept { return m_CPUData.m_Radius; }
        [[nodiscard]] inline constexpr const VertexData& getVertexData() const noexcept { return *m_CPUData.m_VertexData; }
        [[nodiscard]] inline constexpr const Engine::priv::MeshSkeleton* getSkeleton() const noexcept { return m_CPUData.m_Skeleton; }

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