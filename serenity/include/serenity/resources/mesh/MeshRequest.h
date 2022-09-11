#pragma once
#ifndef ENGINE_MESH_REQUEST_H
#define ENGINE_MESH_REQUEST_H

#include <serenity/resources/mesh/Mesh.h>
#include <serenity/resources/mesh/animation/AnimationIncludes.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace Engine::priv {
    struct AssimpSceneImport final {
        std::shared_ptr<Assimp::Importer>  m_Importer_ptr;
        aiScene*                           m_AIScene        = nullptr;

        AssimpSceneImport();

        AssimpSceneImport(const AssimpSceneImport&)                = default;
        AssimpSceneImport& operator=(const AssimpSceneImport&)     = default;
        AssimpSceneImport(AssimpSceneImport&&) noexcept            = delete;
        AssimpSceneImport& operator=(AssimpSceneImport&&) noexcept = delete;
    };
};

struct MeshRequestPart final {
    MeshCPUData              cpuData;
    MeshNodeData             nodesData;
    std::vector<std::string> nodesNames;
    std::vector<aiNode*>     leafNodes;
    std::string              name;
    std::string              fileOrData;
    std::string              fileExtension;
    aiNode*                  rootNode = nullptr;
    aiMesh*                  aiMesh   = nullptr;
    aiNode*                  aiMeshNode = nullptr;
    Handle                   handle;
    bool                     fileExists = false;

    MeshRequestPart() = default;

    MeshRequestPart(const MeshRequestPart&)                = default;
    MeshRequestPart& operator=(const MeshRequestPart&)     = default;
    MeshRequestPart(MeshRequestPart&&) noexcept            = default;
    MeshRequestPart& operator=(MeshRequestPart&&) noexcept = default;
};

using MeshRequestCallback = std::function<void(const std::vector<Handle>&)>;
struct MeshRequest final {
    MeshCollisionLoadingFlag::Flag     m_CollisionLoadingFlags = MESH_COLLISION_FACTORY_DEFAULT_LOAD_FLAG;
    Engine::priv::AssimpSceneImport    m_AssimpImporter;
    MeshRequestCallback                m_Callback;
    std::string                        m_FileOrData;
    std::string                        m_FileExtension;
    std::vector<MeshRequestPart>       m_Parts;
    float                              m_Threshold             = MESH_DEFAULT_THRESHOLD;
    bool                               m_Async                 = false;
    bool                               m_FileExists            = false;

    MeshRequest() = delete;
    MeshRequest(std::string_view filenameOrData, float threshold, MeshCollisionLoadingFlag::Flag, MeshRequestCallback&& callback);

    MeshRequest(const MeshRequest&)                 = default;
    MeshRequest& operator=(const MeshRequest&)      = default;
    MeshRequest(MeshRequest&&) noexcept;
    MeshRequest& operator=(MeshRequest&&) noexcept;

    void request(bool async = false);
    bool populate();
};

namespace Engine::priv {
    struct PublicMeshRequest final {
        friend class  ::Mesh;
        static void LoadCPU(MeshRequest& meshRequest);
    };
};


#endif