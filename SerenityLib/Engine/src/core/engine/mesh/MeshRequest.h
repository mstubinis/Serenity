#pragma once
#ifndef ENGINE_MESH_REQUEST_H
#define ENGINE_MESH_REQUEST_H

class Mesh;

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <core/engine/resources/Handle.h>
#include <core/engine/mesh/AnimationIncludes.h>

namespace Engine::priv {
    struct MeshInfoNode;
    struct AssimpSceneImport final {
        std::shared_ptr<Assimp::Importer>  m_Importer_ptr;
        aiScene*                           m_AIScene        = nullptr;
        aiNode*                            m_AIRoot         = nullptr;

        AssimpSceneImport();
        ~AssimpSceneImport() = default;

        AssimpSceneImport(const AssimpSceneImport& other)                = default;
        AssimpSceneImport& operator=(const AssimpSceneImport& other)     = default;
        AssimpSceneImport(AssimpSceneImport&& other) noexcept            = delete;
        AssimpSceneImport& operator=(AssimpSceneImport&& other) noexcept = delete;
    };
};

struct MeshRequestPart final {
    //Mesh*        mesh   = nullptr;
    Handle       handle = Handle();
    std::string  name   = "";

    MeshRequestPart() = default;
    ~MeshRequestPart() = default;

    MeshRequestPart(const MeshRequestPart& other)                = default;
    MeshRequestPart& operator=(const MeshRequestPart& other)     = default;
    MeshRequestPart(MeshRequestPart&& other) noexcept            = default;
    MeshRequestPart& operator=(MeshRequestPart&& other) noexcept = default;

};

struct MeshRequest final {
    float                                                       m_Threshold      = 0.005f;
    std::string                                                 m_FileOrData     = "";
    std::string                                                 m_FileExtension  = "";
    bool                                                        m_FileExists     = false;
    std::vector<MeshRequestPart>                                m_Parts;
    bool                                                        m_Async          = false;
    Engine::priv::AssimpSceneImport                             m_Importer;
    MeshNodeMap                                                 m_MeshNodeMap;
    std::function<void()>                                       m_Callback;

    MeshRequest() = delete;
    MeshRequest(const std::string& filenameOrData, float threshold, std::function<void()>&& callback);
    ~MeshRequest() = default;

    MeshRequest(const MeshRequest& other) = default;
    MeshRequest& operator=(const MeshRequest& other) = default;
    MeshRequest(MeshRequest&& other) noexcept;
    MeshRequest& operator=(MeshRequest&& other) noexcept;

    void request(bool async = false);
};

namespace Engine::priv {
    struct InternalMeshRequestPublicInterface final {
        friend class  Mesh;
        static void Request(MeshRequest& meshRequest);
        static bool Populate(MeshRequest& meshRequest);
        static void LoadGPU(MeshRequest& meshRequest);
        static void LoadCPU(MeshRequest& meshRequest);
    };
};


#endif