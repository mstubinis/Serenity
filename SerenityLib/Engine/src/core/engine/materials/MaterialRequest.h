#pragma once
#ifndef ENGINE_MATERIAL_MATERIAL_REQUEST_H
#define ENGINE_MATERIAL_MATERIAL_REQUEST_H

#include <vector>
#include <string>
#include <core/engine/resources/Handle.h>
#include <core/engine/textures/TextureRequest.h>

class Material;
struct MaterialRequestPart final {
    Material*                     material;
    Handle                        handle;
    std::string                   name;
    std::vector<TextureRequest*>  textureRequests;

    MaterialRequestPart() {
        material = nullptr;
        name = "";
        handle = Handle();
    }
    ~MaterialRequestPart() {
        textureRequests.clear();
    }
};

struct MaterialRequest final {
    MaterialRequestPart  part;
    bool                 async;

    MaterialRequest();
    MaterialRequest(const std::string& name, const std::string& diffuse, const std::string& normal, const std::string& glow, const std::string& specular);
    ~MaterialRequest();

    void request();
    void requestAsync();
};

namespace Engine {
    namespace epriv {
        struct InternalMaterialRequestPublicInterface final {
            friend class  Material;
            static void Request(MaterialRequest&);
            static void LoadGPU(MaterialRequest&);
            static void LoadCPU(MaterialRequest&);
        };
    };
};


#endif