#pragma once
#ifndef ENGINE_MESH_REQUEST_INCLUDE_GUARD
#define ENGINE_MESH_REQUEST_INCLUDE_GUARD

class Mesh;

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <core/engine/resources/Handle.h>

namespace Engine::priv {
    struct BoneNode;
    struct AssimpSceneImport final {
        std::shared_ptr<Assimp::Importer>  importer_ptr;
        aiScene*                           scene;
        aiNode*                            root;

        AssimpSceneImport();
        ~AssimpSceneImport();

        AssimpSceneImport(const AssimpSceneImport& other);
        AssimpSceneImport& operator=(const AssimpSceneImport& other);
        AssimpSceneImport(AssimpSceneImport&& other) noexcept            = delete;
        AssimpSceneImport& operator=(AssimpSceneImport&& other) noexcept = delete;
    };
};

struct MeshRequest final {
    struct MeshRequestPart final {
        Mesh*        mesh;
        Handle       handle;
        std::string  name;

        MeshRequestPart();
        ~MeshRequestPart();

        MeshRequestPart(const MeshRequestPart& other);
        MeshRequestPart& operator=(const MeshRequestPart& other);
        MeshRequestPart(MeshRequestPart&& other) noexcept;
        MeshRequestPart& operator=(MeshRequestPart&& other) noexcept;

    };


    float                                                       threshold;
    std::string                                                 fileOrData;
    std::string                                                 fileExtension;
    bool                                                        fileExists;
    std::vector<MeshRequest::MeshRequestPart>                   parts;
    bool                                                        async;
    Engine::priv::AssimpSceneImport                             importer;
    std::unordered_map<std::string, Engine::priv::BoneNode*>    map;

    MeshRequest() = delete;
    MeshRequest(const std::string& filenameOrData, float threshold);
    ~MeshRequest();

    MeshRequest(const MeshRequest& other);
    MeshRequest& operator=(const MeshRequest& other);
    MeshRequest(MeshRequest&& other) noexcept;
    MeshRequest& operator=(MeshRequest&& other) noexcept;

    void request();
    void requestAsync();
};

namespace Engine::priv {
    struct InternalMeshRequestPublicInterface final {
        friend class  Mesh;
        static void Request(MeshRequest&);
        static bool Populate(MeshRequest&);
        static void LoadGPU(MeshRequest&);
        static void LoadCPU(MeshRequest&);
    };
};


#endif