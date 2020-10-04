#pragma once
#ifndef ENGINE_MATERIAL_COMPONENT_H
#define ENGINE_MATERIAL_COMPONENT_H

#include <core/engine/materials/MaterialEnums.h>
#include <core/engine/materials/MaterialLayer.h>

class MaterialComponent final {
    protected:
        std::array<MaterialLayer, MAX_MATERIAL_LAYERS_PER_COMPONENT>   m_Layers;
        unsigned int                                                   m_NumLayers      = 0U;
        MaterialComponentType                                          m_ComponentType  = MaterialComponentType::Diffuse;
    public:
        MaterialComponent() = default;
        MaterialComponent(MaterialComponentType type, Handle textureHandle, Handle maskHandle = Handle{}, Handle cubemapHandle = Handle{});
        MaterialComponent(const MaterialComponent& other)                  = delete;
        MaterialComponent& operator=(const MaterialComponent& other)       = delete;
        MaterialComponent(MaterialComponent&& other) noexcept;
        MaterialComponent& operator=(MaterialComponent&& other) noexcept;
        ~MaterialComponent() = default;

        inline constexpr bool operator==(const bool other) const noexcept { return (other == true && m_NumLayers > 0); }
        inline constexpr bool operator!=(const bool other) const noexcept { return !operator==(other); }

        void bind(size_t component_index, size_t& inTextureUnit) const;
        //void unbind() {}

        MaterialLayer* addLayer(const std::string& textureFile, const std::string& maskFile = "", const std::string& cubemapFile = "");
        MaterialLayer* addLayer(Handle textureHandle, Handle maskHandle = Handle{}, Handle cubemapHandle = Handle{});

        inline Handle texture(size_t index = 0) const noexcept { return m_Layers[index].getTexture(); }
        inline Handle mask(size_t index = 0) const noexcept { return m_Layers[index].getMask(); }
        inline Handle cubemap(size_t index = 0) const noexcept { return m_Layers[index].getCubemap(); }
        inline MaterialLayer& layer(size_t index = 0) noexcept { return m_Layers[index]; }

        inline CONSTEXPR unsigned int numLayers() const { return m_NumLayers; }
        inline CONSTEXPR MaterialComponentType type() const noexcept { return m_ComponentType; }

        void update(const float dt);
};
#endif