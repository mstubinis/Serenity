#pragma once
#ifndef ENGINE_MATERIAL_COMPONENT_H
#define ENGINE_MATERIAL_COMPONENT_H

#include <core/engine/materials/MaterialEnums.h>
#include <core/engine/materials/MaterialLayer.h>

class MaterialComponent {
    protected:
        std::array<MaterialLayer, MAX_MATERIAL_LAYERS_PER_COMPONENT>   m_Layers;
        unsigned int                                                   m_NumLayers      = 0U;
        MaterialComponentType                                          m_ComponentType  = MaterialComponentType::Diffuse;
    public:
        MaterialComponent(MaterialComponentType type, Texture* texture, Texture* mask = nullptr, Texture* cubemap = nullptr);
        virtual ~MaterialComponent() {}

        MaterialComponent(const MaterialComponent&)                      = delete;
        MaterialComponent& operator=(const MaterialComponent&)           = delete;
        MaterialComponent(MaterialComponent&& other) noexcept            = delete;
        MaterialComponent& operator=(MaterialComponent&& other) noexcept = delete;

        virtual void bind(size_t component_index, size_t& inTextureUnit) const;
        virtual void unbind() {}

        MaterialLayer* addLayer(const std::string& textureFile, const std::string& maskFile = "", const std::string& cubemapFile = "");
        MaterialLayer* addLayer(Texture* texture = nullptr, Texture* mask = nullptr, Texture* cubemap = nullptr);

        Texture* texture(size_t index = 0) const;
        Texture* mask(size_t index = 0) const;
        Texture* cubemap(size_t index = 0) const;
        MaterialLayer& layer(size_t index = 0);

        inline CONSTEXPR unsigned int numLayers() const { return m_NumLayers; }
        inline CONSTEXPR MaterialComponentType type() const noexcept { return m_ComponentType; }

        void update(const float dt);
};
#endif