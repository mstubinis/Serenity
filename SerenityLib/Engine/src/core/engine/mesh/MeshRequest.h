#pragma once
#ifndef ENGINE_MESH_REQUEST_INCLUDE_GUARD
#define ENGINE_MESH_REQUEST_INCLUDE_GUARD

#include <vector>
#include <string>
#include <core/engine/resources/Handle.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/mesh/Skeleton.h>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace Engine {
namespace epriv {
    struct AssimpSceneImport final {
        Assimp::Importer importer;
        aiScene*         scene;
        aiNode*          root;

        AssimpSceneImport() { scene = 0; root = 0; }
    };
};
};
struct MeshRequestPart final {
    Mesh*        mesh;
    Handle       handle;
    std::string  name;

    MeshRequestPart() {
        mesh = nullptr;
        name = "";
        handle = Handle();
    }
    ~MeshRequestPart() {
        mesh = nullptr;
        name = "";
        handle = Handle();
    }
};

struct MeshRequest final {
    float                                                       threshold;
    std::string                                                 fileOrData;
    std::string                                                 fileExtension;
    bool                                                        fileExists;
    std::vector<MeshRequestPart>                                parts;
    bool                                                        async;
    Engine::epriv::AssimpSceneImport                            importer;
    std::unordered_map<std::string, Engine::epriv::BoneNode*>   map;

    MeshRequest();
    MeshRequest(const std::string& _filenameOrData,float _threshold);
    ~MeshRequest();

    void request();
    void requestAsync();
};


namespace Engine {
namespace epriv {
    struct InternalMeshRequestPublicInterface final {
        friend class  ::Mesh;
        static void Request(MeshRequest&);
        static bool Populate(MeshRequest&);
        static void LoadGPU(MeshRequest&);
        static void LoadCPU(MeshRequest&);
    };
};
};


#endif