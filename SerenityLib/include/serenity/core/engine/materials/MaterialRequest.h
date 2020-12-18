#pragma once
#ifndef ENGINE_MATERIAL_MATERIAL_REQUEST_H
#define ENGINE_MATERIAL_MATERIAL_REQUEST_H

class Material;
class Texture;

namespace Engine::priv {
    struct InternalMaterialRequestPublicInterface;
}

#include <serenity/core/engine/resources/Handle.h>
#include <serenity/core/engine/textures/TextureRequest.h>

struct MaterialRequestPart final {
    Handle                                        m_Handle   = Handle{};
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
    friend struct Engine::priv::InternalMaterialRequestPublicInterface;
private:
    void internal_set_async(bool async) noexcept;
    void internal_init_material_components() noexcept;
    void internal_void_launch_texture_requests(bool async) noexcept;
public:
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
        Handle diffuse,
        Handle normal,
        Handle glow,
        Handle specular,
        Handle ao,
        Handle metalness,
        Handle smoothness,
        std::function<void()>&& callback
    );
    ~MaterialRequest() = default;

    MaterialRequest(const MaterialRequest& other)                = default;
    MaterialRequest& operator=(const MaterialRequest& other)     = default;
    MaterialRequest(MaterialRequest&& other) noexcept            = delete;
    MaterialRequest& operator=(MaterialRequest&& other) noexcept = delete;

    void request(bool async = false);
};

#endif