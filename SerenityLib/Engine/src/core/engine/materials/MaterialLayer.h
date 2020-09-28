#pragma once
#ifndef ENGINE_MATERIAL_LAYER_H
#define ENGINE_MATERIAL_LAYER_H

class  MaterialLayer;
class  Texture;
struct SimpleUVTranslationFunctor;

#include <core/engine/materials/MaterialEnums.h>

using uv_mod_func = std::function<void(const float dt, MaterialLayer& layer)>;

class MaterialLayer final{
    friend struct SimpleUVTranslationFunctor;
    private:
        Texture*                   m_Texture                       = nullptr;
        Texture*                   m_Mask                          = nullptr;
        Texture*                   m_Cubemap                       = nullptr;

        //x = blend mode? | y = texture enabled? | z = mask enabled? | w = cubemap enabled?
        glm::vec4                  m_Data1                         = glm::vec4((float)MaterialLayerBlendMode::Default, 0.0f, 0.0f, 0.0f);
        glm::vec4                  m_Data2                         = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

        //x = translationX, y = translationY, z = multX, w = multY
        glm::vec4                  m_UVModifications               = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

        std::vector<uv_mod_func>   m_UVModificationQueue;
    public:
        MaterialLayer() = default;
        ~MaterialLayer() = default;

        MaterialLayer(const MaterialLayer&)                      = delete;
        MaterialLayer& operator=(const MaterialLayer&)           = delete;
        MaterialLayer(MaterialLayer&& other) noexcept            = delete;
        MaterialLayer& operator=(MaterialLayer&& other) noexcept = delete;

        inline CONSTEXPR Texture* getTexture() const noexcept { return m_Texture; }
        inline CONSTEXPR Texture* getMask() const noexcept { return m_Mask; }
        inline CONSTEXPR Texture* getCubemap() const noexcept { return m_Cubemap; }

        inline CONSTEXPR const glm::vec4& data1() const noexcept { return m_Data1; }
        inline CONSTEXPR const glm::vec4& data2() const noexcept { return m_Data2; }
        inline CONSTEXPR MaterialLayerBlendMode blendMode() const noexcept { return (MaterialLayerBlendMode)((unsigned int)m_Data1.x); }
        inline CONSTEXPR const glm::vec4& getUVModifications() const noexcept { return m_UVModifications; }

        void addUVModificationFunctor(uv_mod_func&& functor);
        void addUVModificationSimpleTranslation(float translationX, float translationY);
        void addUVModificationSimpleMultiplication(float mulX, float mulY);

        void setBlendMode(MaterialLayerBlendMode mode);
        void setTexture(Texture* texture);
        void setMask(Texture* mask);
        void setCubemap(Texture* cubemap);
        void setTexture(const std::string& textureFile);
        void setMask(const std::string& maskFile);
        void setCubemap(const std::string& cubemapFile);

        void setData1(float x, float y, float z, float w);
        void setData2(float x, float y, float z, float w);

        void sendDataToGPU(const std::string& uniform_component_string, size_t component_index, size_t layer_index, size_t& textureUnit) const;

        void update(const float dt);
};

#endif