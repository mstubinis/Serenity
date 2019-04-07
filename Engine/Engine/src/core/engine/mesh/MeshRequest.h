#pragma once
#ifndef ENGINE_MESH_REQUEST_INCLUDE_GUARD
#define ENGINE_MESH_REQUEST_INCLUDE_GUARD

#include <vector>
#include <string>
#include <core/engine/resources/Handle.h>
#include <core/engine/mesh/Mesh.h>


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
    float                          threshold;
    std::string                    fileOrData;
    std::vector<MeshRequestPart>   parts;
    bool                           async;

    MeshRequest();
    MeshRequest(const std::string& _filenameOrData,float _threshold);
    ~MeshRequest();

    void request();
    void requestAsync();
};



#endif