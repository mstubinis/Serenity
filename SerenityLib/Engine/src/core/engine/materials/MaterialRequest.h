#pragma once
#ifndef ENGINE_MATERIAL_MATERIAL_REQUEST_H
#define ENGINE_MATERIAL_MATERIAL_REQUEST_H

#include <vector>
#include <string>
#include <memory>
#include <core/engine/resources/Handle.h>
#include <core/engine/textures/TextureRequest.h>

class Material;
class Texture;
struct MaterialRequestPart final {
    Material*                                     material = nullptr;
    Handle                                        handle   = Handle();
    std::string                                   name     = "";
    std::vector<std::shared_ptr<TextureRequest>>  textureRequests;

    MaterialRequestPart();
    ~MaterialRequestPart();

    MaterialRequestPart(const MaterialRequestPart&);
    MaterialRequestPart& operator=(const MaterialRequestPart&);
    MaterialRequestPart(MaterialRequestPart&& other) noexcept = delete;
    MaterialRequestPart& operator=(MaterialRequestPart&& other) noexcept = delete;

};

struct MaterialRequest final {
    MaterialRequestPart  part;
    bool                 async = false;

    MaterialRequest() = delete;
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

    MaterialRequest(const MaterialRequest&);
    MaterialRequest& operator=(const MaterialRequest&);

    void request();
    void requestAsync();
};

namespace Engine::priv {
    struct InternalMaterialRequestPublicInterface final {
        friend class  Material;
        static void Request(MaterialRequest&);
        static void LoadGPU(MaterialRequest&);
        static void LoadCPU(MaterialRequest&);
    };
};


#endif