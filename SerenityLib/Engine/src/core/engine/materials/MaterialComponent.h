#pragma once
#ifndef ENGINE_MATERIAL_COMPONENT_H
#define ENGINE_MATERIAL_COMPONENT_H

#include <core/engine/utils/Utils.h>
#include <core/engine/materials/MaterialEnums.h>
#include <core/engine/materials/MaterialLayer.h>
#include <glm/vec4.hpp>

namespace Engine {
    namespace epriv {
        const uint MAX_MATERIAL_LAYERS_PER_COMPONENT = 3;
    };
};

class MaterialComponent {
    protected:
        MaterialLayer                m_Layers[Engine::epriv::MAX_MATERIAL_LAYERS_PER_COMPONENT];
        uint                         m_NumLayers;
        MaterialComponentType::Type  m_ComponentType;
    public:
        MaterialComponent(const uint& type, Texture* texture);
        virtual ~MaterialComponent();

        virtual void bind(glm::vec4&);
        virtual void unbind();

        Texture* texture(const uint& index = 0) const;
        const MaterialComponentType::Type& type() const;

        void update(const float& dt);
};

class MaterialComponentReflection : public MaterialComponent {
    protected:
        float m_MixFactor;
    public:
        MaterialComponentReflection(const uint& type, Texture* cubemap, Texture* mask, const float& mixFactor);
        ~MaterialComponentReflection();

        virtual void bind(glm::vec4&);
        void unbind();
        void setMixFactor(const float&);

        const float& mixFactor() const { return m_MixFactor; }
};
class MaterialComponentRefraction : public MaterialComponentReflection {
    private:
        float m_RefractionIndex;
    public:
        MaterialComponentRefraction(Texture* cubemap, Texture* mask, const float& mixFactor, const float& ratio);
        ~MaterialComponentRefraction();

        void bind(glm::vec4&);

        void setRefractionIndex(const float&);
        const float& refractionIndex() const { return m_RefractionIndex; }
};
class MaterialComponentParallaxOcclusion : public MaterialComponent {
    protected:
        float m_HeightScale;
    public:
        MaterialComponentParallaxOcclusion(Texture* heightmap, const float& heightScale);
        ~MaterialComponentParallaxOcclusion();

        void bind(glm::vec4&);
        void unbind();
        void setHeightScale(const float&);

        const float& heightScale() const { return m_HeightScale; }
};

#endif