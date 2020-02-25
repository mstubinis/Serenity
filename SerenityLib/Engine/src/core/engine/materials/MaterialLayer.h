#pragma once
#ifndef ENGINE_MATERIAL_LAYER_H
#define ENGINE_MATERIAL_LAYER_H

class  MaterialLayer;
class  Texture;
struct SimpleUVTranslationFunctor;

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

#include <vector>
#include <string>
#include <functional>
#include <core/engine/materials/MaterialEnums.h>

typedef std::function<void(const float& dt)> std_uv_func;

class MaterialLayer final{
    friend struct SimpleUVTranslationFunctor;
    private:
        Texture*                   m_Texture          = nullptr;
        Texture*                   m_Mask             = nullptr;
        Texture*                   m_Cubemap          = nullptr;

        //x = blend mode? | y = texture enabled? | z = mask enabled? | w = cubemap enabled?
        glm::vec4                  m_Data1            = glm::vec4(0.0f);
        glm::vec4                  m_Data2            = glm::vec4(0.0f);
        glm::vec2                  m_UVModifications  = glm::vec2(0.0f);

        std::vector<std_uv_func>   m_UVModificationQueue;
    public:
        MaterialLayer();
        ~MaterialLayer();

        MaterialLayer(const MaterialLayer&)                      = delete;
        MaterialLayer& operator=(const MaterialLayer&)           = delete;
        MaterialLayer(MaterialLayer&& other) noexcept            = delete;
        MaterialLayer& operator=(MaterialLayer&& other) noexcept = delete;

        Texture* getTexture() const;
        Texture* getMask() const;
        Texture* getCubemap() const;

        const glm::vec4& data1() const;
        const glm::vec4& data2() const;
        const MaterialLayerBlendMode::Mode blendMode() const;

        void addUVModificationFunctor(const std_uv_func& functor);
        void addUVModificationSimpleTranslation(const float& translationX, const float& translationY);

        void setBlendMode(const MaterialLayerBlendMode::Mode& mode);
        void setTexture(Texture* texture);
        void setMask(Texture* mask);
        void setCubemap(Texture* cubemap);
        void setTexture(const std::string& textureFile);
        void setMask(const std::string& maskFile);
        void setCubemap(const std::string& cubemapFile);

        void setData1(const float& x, const float& y, const float& z, const float& w);
        void setData2(const float& x, const float& y, const float& z, const float& w);

        void sendDataToGPU(const std::string& uniform_component_string, const size_t& component_index, const size_t& layer_index, size_t& textureUnit) const;

        const glm::vec2& getUVModifications() const;

        void update(const float& dt);
};

#endif