#pragma once
#ifndef ENGINE_MATERIAL_COMPONENT_H
#define ENGINE_MATERIAL_COMPONENT_H

#include <core/engine/materials/MaterialEnums.h>
#include <core/engine/materials/MaterialLayer.h>

class MaterialComponent {
    protected:
        MaterialLayer                m_Layers[MAX_MATERIAL_LAYERS_PER_COMPONENT];
        unsigned int                 m_NumLayers;
        MaterialComponentType::Type  m_ComponentType;
    public:
        MaterialComponent(const MaterialComponentType::Type& type, Texture* texture, Texture* mask = nullptr, Texture* cubemap = nullptr);
        virtual ~MaterialComponent();

        MaterialComponent(const MaterialComponent&)                      = delete;
        MaterialComponent& operator=(const MaterialComponent&)           = delete;
        MaterialComponent(MaterialComponent&& other) noexcept            = delete;
        MaterialComponent& operator=(MaterialComponent&& other) noexcept = delete;

        virtual void bind(const unsigned int& component_index, unsigned int& textureUnit);
        virtual void unbind();

        MaterialLayer* addLayer(const std::string& textureFile, const std::string& maskFile = "", const std::string& cubemapFile = "");
        MaterialLayer* addLayer(Texture* texture = nullptr, Texture* mask = nullptr, Texture* cubemap = nullptr);

        Texture* texture(const unsigned int& index = 0) const;
        Texture* mask(const unsigned int& index = 0) const;
        Texture* cubemap(const unsigned int& index = 0) const;
        MaterialLayer& layer(const unsigned int& index = 0);

        const MaterialComponentType::Type& type() const;

        void update(const double& dt);
};
#endif