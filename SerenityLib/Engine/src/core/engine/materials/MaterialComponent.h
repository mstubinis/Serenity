#pragma once
#ifndef ENGINE_MATERIAL_COMPONENT_H
#define ENGINE_MATERIAL_COMPONENT_H

#include <core/engine/utils/Utils.h>
#include <core/engine/materials/MaterialEnums.h>
#include <core/engine/materials/MaterialLayer.h>
#include <glm/vec4.hpp>

class MaterialComponent {
    protected:
        MaterialLayer                m_Layers[MAX_MATERIAL_LAYERS_PER_COMPONENT];
        uint                         m_NumLayers;
        MaterialComponentType::Type  m_ComponentType;
    public:
        MaterialComponent(const MaterialComponentType::Type& type, Texture* texture, Texture* mask = nullptr, Texture* cubemap = nullptr);
        virtual ~MaterialComponent();

        virtual void bind(const uint& component_index);
        virtual void unbind();

        MaterialLayer* addLayer(const std::string& textureFile, const std::string& maskFile = "", const std::string& cubemapFile = "");
        MaterialLayer* addLayer(Texture* texture = nullptr, Texture* mask = nullptr, Texture* cubemap = nullptr);

        Texture* texture(const uint& index = 0) const;
        Texture* mask(const uint& index = 0) const;
        Texture* cubemap(const uint& index = 0) const;
        MaterialLayer& layer(const uint& index = 0);

        const MaterialComponentType::Type& type() const;

        void update(const float& dt);
};
#endif