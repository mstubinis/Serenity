#pragma once
#ifndef ENGINE_MATERIAL_MATERIAL_REQUEST_H
#define ENGINE_MATERIAL_MATERIAL_REQUEST_H

class Material;
class Texture;

#include <vector>
#include <string>
#include <memory>
#include <core/engine/resources/Handle.h>
#include <core/engine/textures/TextureRequest.h>

struct MaterialRequestPart final {
    Material*                                     m_Material = nullptr;
    Handle                                        m_Handle   = Handle();
    std::string                                   m_Name     = "";
    std::vector<std::shared_ptr<TextureRequest>>  m_TextureRequests;

    MaterialRequestPart();
    ~MaterialRequestPart();

    MaterialRequestPart(const MaterialRequestPart&);
    MaterialRequestPart& operator=(const MaterialRequestPart&);
    MaterialRequestPart(MaterialRequestPart&& other) noexcept = delete;
    MaterialRequestPart& operator=(MaterialRequestPart&& other) noexcept = delete;
};

struct MaterialRequest final {
    MaterialRequestPart  m_Part;
    bool                 m_Async = false;

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

    void request(bool async = false);
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