#pragma once
#ifndef ENGINE_MATERIAL_MATERIAL_REQUEST_H
#define ENGINE_MATERIAL_MATERIAL_REQUEST_H

#include <vector>
#include <string>
#include <core/engine/resources/Handle.h>
#include <core/engine/textures/TextureRequest.h>

class Material;
class Texture;
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
    MaterialRequestPart(const MaterialRequestPart&)                      = delete;
    MaterialRequestPart& operator=(const MaterialRequestPart&)           = delete;
    MaterialRequestPart(MaterialRequestPart&& other) noexcept            = delete;
    MaterialRequestPart& operator=(MaterialRequestPart&& other) noexcept = delete;

};

struct MaterialRequest final {
    MaterialRequestPart  part;
    bool                 async;

    MaterialRequest();
    MaterialRequest(
        const std::string& name,
        const std::string& diffuse,
        const std::string& normal,
        const std::string& glow,
        const std::string& specular,
        const std::string& ao,
        const std::string& metalness,
        const std::string& smoothness
    );
    MaterialRequest(
        const std::string& name,
        Texture* diffuse,
        Texture* normal,
        Texture* glow,
        Texture* specular,
        Texture* ao,
        Texture* metalness,
        Texture* smoothness
    );
    ~MaterialRequest();

    MaterialRequest(const MaterialRequest&)                      = delete;
    MaterialRequest& operator=(const MaterialRequest&)           = delete;
    MaterialRequest(MaterialRequest&& other) noexcept            = delete;
    MaterialRequest& operator=(MaterialRequest&& other) noexcept = delete;

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