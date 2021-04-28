#pragma once
#ifndef ENGINE_MATERIAL_COMPONENT_H
#define ENGINE_MATERIAL_COMPONENT_H

#include <serenity/resources/material/MaterialEnums.h>
#include <serenity/resources/material/MaterialLayer.h>
#include <array>

class MaterialComponent final {
    protected:
        std::array<MaterialLayer, MAX_MATERIAL_LAYERS_PER_COMPONENT>   m_Layers;
        uint32_t                                                       m_NumLayers      = 0U;
        MaterialComponentType                                          m_ComponentType  = MaterialComponentType::Empty;
    public:
        MaterialComponent() = default;
        MaterialComponent(MaterialComponentType type, Handle textureHandle, Handle maskHandle = Handle{}, Handle cubemapHandle = Handle{});

        MaterialComponent(const MaterialComponent&)                  = delete;
        MaterialComponent& operator=(const MaterialComponent&)       = delete;
        MaterialComponent(MaterialComponent&&) noexcept;
        MaterialComponent& operator=(MaterialComponent&&) noexcept;

        inline constexpr bool operator==(const bool other) const noexcept { return (other && m_ComponentType != MaterialComponentType::Empty && m_NumLayers > 0); }
        inline constexpr bool operator!=(const bool other) const noexcept { return !operator==(other); }

        void bind(size_t component_index, size_t& inTextureUnit) const;
        //void unbind() {}

        MaterialLayer* addLayer(const std::string& textureFile, const std::string& maskFile = "", const std::string& cubemapFile = "");
        MaterialLayer* addLayer(Handle textureHandle, Handle maskHandle = Handle{}, Handle cubemapHandle = Handle{});

        [[nodiscard]] inline Handle getTexture(size_t index = 0) const noexcept { return m_Layers[index].getTexture(); }
        [[nodiscard]] inline Handle getMask(size_t index = 0) const noexcept { return m_Layers[index].getMask(); }
        [[nodiscard]] inline Handle getCubemap(size_t index = 0) const noexcept { return m_Layers[index].getCubemap(); }
        [[nodiscard]] inline MaterialLayer& getLayer(size_t index = 0) noexcept { return m_Layers[index]; }
        [[nodiscard]] inline uint32_t getNumLayers() const { return m_NumLayers; }
        [[nodiscard]] inline MaterialComponentType getType() const noexcept { return m_ComponentType; }

        void update(const float dt);
};
#endif