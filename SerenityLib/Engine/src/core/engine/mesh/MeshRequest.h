#pragma once
#ifndef ENGINE_MESH_REQUEST_H
#define ENGINE_MESH_REQUEST_H

#include <core/engine/mesh/Mesh.h>
#include <core/engine/mesh/AnimationIncludes.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace Engine::priv {
    struct AssimpSceneImport final {
        std::shared_ptr<Assimp::Importer>  m_Importer_ptr;
        aiScene*                           m_AIScene        = nullptr;
        aiNode*                            m_AIRoot         = nullptr;

        AssimpSceneImport();

        AssimpSceneImport(const AssimpSceneImport& other)                = default;
        AssimpSceneImport& operator=(const AssimpSceneImport& other)     = default;
        AssimpSceneImport(AssimpSceneImport&& other) noexcept            = delete;
        AssimpSceneImport& operator=(AssimpSceneImport&& other) noexcept = delete;
    };
};

struct MeshRequestPart final {
    MeshCPUData  cpuData;
    std::string  name     = "";
    Handle       handle   = Handle();

    MeshRequestPart() = default;
    MeshRequestPart(const MeshRequestPart& other)                = default;
    MeshRequestPart& operator=(const MeshRequestPart& other)     = default;
    MeshRequestPart(MeshRequestPart&& other) noexcept            = default;
    MeshRequestPart& operator=(MeshRequestPart&& other) noexcept = default;
};

using MeshRequestCallback = std::function<void(const std::vector<Handle>&)>;
struct MeshRequest final {
    MeshCollisionLoadingFlag::Flag     m_CollisionLoadingFlags = MESH_COLLISION_FACTORY_DEFAULT_LOAD_FLAG;
    Engine::priv::AssimpSceneImport    m_Importer;
    MeshNodeMap                        m_MeshNodeMap;
    MeshRequestCallback                m_Callback;
    std::vector<MeshRequestPart>       m_Parts;
    std::string                        m_FileOrData            = "";
    std::string                        m_FileExtension         = "";
    float                              m_Threshold             = MESH_DEFAULT_THRESHOLD;
    bool                               m_FileExists            = false;
    bool                               m_Async                 = false;

    MeshRequest() = delete;
    MeshRequest(std::string filenameOrData, float threshold, MeshCollisionLoadingFlag::Flag, MeshRequestCallback&& callback);

    MeshRequest(const MeshRequest& other)                 = default;
    MeshRequest& operator=(const MeshRequest& other)      = default;
    MeshRequest(MeshRequest&& other) noexcept;
    MeshRequest& operator=(MeshRequest&& other) noexcept;

    void request(bool async = false);
    bool populate();
};

namespace Engine::priv {
    struct InternalMeshRequestPublicInterface final {
        friend class  Mesh;
        static void LoadCPU(MeshRequest& meshRequest);
    };
};


#endif