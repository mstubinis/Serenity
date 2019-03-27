#pragma once
#ifndef ENGINE_MESH_REQUEST_INCLUDE_GUARD
#define ENGINE_MESH_REQUEST_INCLUDE_GUARD

#include <vector>
#include <string>
#include <core/engine/resources/Handle.h>

class  Mesh;
struct MeshRequest final {
    std::string          file;
    std::vector<Mesh*>   meshes;
    std::vector<Handle>  handles;

    MeshRequest();
    MeshRequest(std::string filename);
    ~MeshRequest();

    void request();
};
#endif