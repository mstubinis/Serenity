#pragma once
#ifndef ENGINE_MESH_REQUEST_INCLUDE_GUARD
#define ENGINE_MESH_REQUEST_INCLUDE_GUARD

#include <vector>
#include <string>
#include <core/engine/resources/Handle.h>
#include <core/engine/mesh/MeshImpl.h>


struct MeshRequestPart final {
    Engine::epriv::MeshImpl*  impl;
    Handle                    handle;
    std::string               name;

    MeshRequestPart() {
        impl = new Engine::epriv::MeshImpl();
        name = "";
        handle = Handle();
    }
    ~MeshRequestPart() {
        impl = nullptr;
        name = "";
        handle = Handle();
    }
};

struct MeshRequest final {
    float                          threshold;
    std::string                    fileOrData;
    std::vector<MeshRequestPart>   parts;

    MeshRequest();
    MeshRequest(const std::string& _filenameOrData,float _threshold);
    ~MeshRequest();

    void request();
};



#endif