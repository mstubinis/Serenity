#pragma once
#ifndef ENGINE_MATERIAL_MATERIAL_REQUEST_H
#define ENGINE_MATERIAL_MATERIAL_REQUEST_H

class Material;
class Texture;

#include <serenity/resources/Handle.h>
#include <serenity/resources/texture/TextureRequest.h>
#include <serenity/types/Types.h>

struct MaterialRequestPart final {
    Handle                                        m_Handle   = Handle{};
    std::string                                   m_Name;
    std::vector<std::shared_ptr<TextureRequest>>  m_TextureRequests;

    MaterialRequestPart() = default;
    //~MaterialRequestPart() = default;

    MaterialRequestPart(const MaterialRequestPart&)                = default;
    MaterialRequestPart& operator=(const MaterialRequestPart&)     = default;
    MaterialRequestPart(MaterialRequestPart&&) noexcept            = delete;
    MaterialRequestPart& operator=(MaterialRequestPart&&) noexcept = delete;
};

class MaterialRequest final {
    private:
        void internal_set_async(bool async) noexcept;
        void internal_init_material_components() noexcept;
        void internal_void_launch_texture_requests(bool async) noexcept;
    public:
        MaterialRequestPart       m_Part;
        bool                      m_Async    = false;
        Engine::ResourceCallback  m_Callback = [](Handle) {};

        MaterialRequest() = delete;
        MaterialRequest(
            std::string_view name,
            std::string_view diffuse,
            std::string_view normal,
            std::string_view glow,
            std::string_view specular,
            std::string_view ao,
            std::string_view metalness,
            std::string_view smoothness,
            Engine::ResourceCallback&& callback
        );
        MaterialRequest(
            std::string_view name,
            Handle diffuse,
            Handle normal,
            Handle glow,
            Handle specular,
            Handle ao,
            Handle metalness,
            Handle smoothness,
            Engine::ResourceCallback&& callback
        );
        //~MaterialRequest() = default;

        MaterialRequest(const MaterialRequest&)                = default;
        MaterialRequest& operator=(const MaterialRequest&)     = default;
        MaterialRequest(MaterialRequest&&) noexcept            = delete;
        MaterialRequest& operator=(MaterialRequest&&) noexcept = delete;

        void request(bool async = false);
};

#endif