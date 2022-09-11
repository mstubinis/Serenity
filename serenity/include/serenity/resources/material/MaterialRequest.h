#pragma once
#ifndef ENGINE_MATERIAL_MATERIAL_REQUEST_H
#define ENGINE_MATERIAL_MATERIAL_REQUEST_H

class Material;
class Texture;

#include <serenity/resources/Handle.h>
#include <serenity/types/Types.h>

class MaterialRequest final {
    private:
        MaterialRequest() = delete;
    public:
        std::string               m_Name;
        std::vector<Handle>       m_Textures;
        std::vector<bool>         m_TexturesValid;
        Engine::ResourceCallback  m_Callback       = [](Handle) {};
        Handle                    m_Handle;
        bool                      m_Async          = false;

        MaterialRequest(
            bool async,
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
            bool async,
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

        MaterialRequest(const MaterialRequest&)                = default;
        MaterialRequest& operator=(const MaterialRequest&)     = default;
        MaterialRequest(MaterialRequest&&) noexcept            = default;
        MaterialRequest& operator=(MaterialRequest&&) noexcept = default;
};

#endif