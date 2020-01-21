#pragma once
#ifndef ENGINE_MESH_REQUEST_INCLUDE_GUARD
#define ENGINE_MESH_REQUEST_INCLUDE_GUARD

#include <vector>
#include <string>
#include <core/engine/resources/Handle.h>
//#include <core/engine/mesh/Mesh.h>
//#include <core/engine/mesh/Skeleton.h>

#include <unordered_map>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

class Mesh;
namespace Engine {
    namespace priv {
        struct BoneNode;
        struct AssimpSceneImport final {
            Assimp::Importer importer;
            aiScene*         scene;
            aiNode*          root;
            AssimpSceneImport() { 
                scene = 0;
                root = 0; 
            }
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
};

struct MeshRequest final {
    float                                                       threshold;
    std::string                                                 fileOrData;
    std::string                                                 fileExtension;
    bool                                                        fileExists;
    std::vector<MeshRequestPart>                                parts;
    bool                                                        async;
    Engine::priv::AssimpSceneImport                            importer;
    std::unordered_map<std::string, Engine::priv::BoneNode*>   map;

    MeshRequest();
    MeshRequest(const std::string& filenameOrData, float threshold);
    ~MeshRequest();

    void request();
    void requestAsync();
};


namespace Engine {
    namespace priv {
        struct InternalMeshRequestPublicInterface final {
            friend class  Mesh;
            static void Request(MeshRequest&);
            static bool Populate(MeshRequest&);
            static void LoadGPU(MeshRequest&);
            static void LoadCPU(MeshRequest&);
        };
    };
};


#endif