#pragma once
#ifndef ENGINE_MATERIAL_MATERIAL_REQUEST_H
#define ENGINE_MATERIAL_MATERIAL_REQUEST_H

class Material;
class Texture;

#include <core/engine/resources/Handle.h>
#include <core/engine/textures/TextureRequest.h>

struct MaterialRequestPart final {
    Material*                                     m_Material = nullptr;
    Handle                                        m_Handle   = Handle();
    std::string                                   m_Name     = "";
    std::vector<std::shared_ptr<TextureRequest>>  m_TextureRequests;

    MaterialRequestPart() = default;
    ~MaterialRequestPart() = default;

    MaterialRequestPart(const MaterialRequestPart& other)                = default;
    MaterialRequestPart& operator=(const MaterialRequestPart& other)     = default;
    MaterialRequestPart(MaterialRequestPart&& other) noexcept            = delete;
    MaterialRequestPart& operator=(MaterialRequestPart&& other) noexcept = delete;
};

struct MaterialRequest final {
    MaterialRequestPart    m_Part;
    bool                   m_Async = false;
    std::function<void()>  m_Callback;

    MaterialRequest() = delete;
    MaterialRequest(
        const std::string& name,
        const std::string& diffuse,
        const std::string& normal,
        const std::string& glow,
        const std::string& specular,
        const std::string& ao,
        const std::string& metalness,
        const std::string& smoothness,
        std::function<void()>&& callback
    );
    MaterialRequest(
        const std::string& name,
        Texture* diffuse,
        Texture* normal,
        Texture* glow,
        Texture* specular,
        Texture* ao,
        Texture* metalness,
        Texture* smoothness,
        std::function<void()>&& callback
    );
    ~MaterialRequest();

    MaterialRequest(const MaterialRequest& other)                = default;
    MaterialRequest& operator=(const MaterialRequest& other)     = default;
    MaterialRequest(MaterialRequest&& other) noexcept            = delete;
    MaterialRequest& operator=(MaterialRequest&& other) noexcept = delete;

    void request(bool async = false);
};

namespace Engine::priv {
    struct InternalMaterialRequestPublicInterface final {
        friend class  Material;
        static void Request(MaterialRequest& materialRequest);
        static void LoadGPU(MaterialRequest& materialRequest);
        static void LoadCPU(MaterialRequest& materialRequest);
    };
};


#endif