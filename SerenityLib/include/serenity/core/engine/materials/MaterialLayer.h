#pragma once
#ifndef ENGINE_MATERIAL_LAYER_H
#define ENGINE_MATERIAL_LAYER_H

class  MaterialLayer;
class  Texture;
struct SimpleUVTranslationFunctor;

#include <serenity/core/engine/materials/MaterialEnums.h>
#include <serenity/core/engine/resources/Handle.h>
#include <serenity/core/engine/dependencies/glm.h>
#include <vector>
#include <functional>

using uv_mod_func = std::function<void(const float dt, MaterialLayer& layer)>;

struct MaterialLayerTextureData final {
    float blendMode      = (float)MaterialLayerBlendMode::Default;
    float textureEnabled = 0.0f;
    float maskEnabled    = 0.0f;
    float cubemapEnabled = 0.0f;

    MaterialLayerTextureData() = default;
    MaterialLayerTextureData(float blendMode_, float textureEnabled_, float maskEnabled_, float cubemapEnabled_)
        : blendMode{ blendMode_ }
        , textureEnabled{ textureEnabled_ }
        , maskEnabled{ maskEnabled_ }
        , cubemapEnabled{ cubemapEnabled_ }
    {}
};
struct MaterialLayerMiscData final {
    float rMultiplier   = 0.0f;
    float gMultiplier   = 0.0f;
    float bMultiplier   = 0.0f;
    float aMultiplier   = 0.0f;

    MaterialLayerMiscData() = default;
    MaterialLayerMiscData(float rMultiplier_, float gMultiplier_, float bMultiplier_, float aMultiplier_)
        : rMultiplier{ rMultiplier_ }
        , gMultiplier{ gMultiplier_ }
        , bMultiplier{ bMultiplier_ }
        , aMultiplier{ aMultiplier_ }
    {}
};

class MaterialLayer final {
    friend struct SimpleUVTranslationFunctor;
    private:
        std::vector<uv_mod_func>   m_UVModificationQueue;

        MaterialLayerTextureData   m_MaterialLayerTextureData;
        MaterialLayerMiscData      m_MaterialLayerMiscData;

        //x = translationX, y = translationY, z = multX, w = multY
        glm::vec4                  m_UVModifications               = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        Handle                     m_TextureHandle                 = Handle{};
        Handle                     m_MaskHandle                    = Handle{};
        Handle                     m_CubemapHandle                 = Handle{};

        void internal_set_texture_and_property(Handle textureHandleValue, Handle& textureHandle, float& propertyVar, float value) noexcept;
        float internal_get_texture_compression_value(Handle textureHandle) noexcept;
    public:
        MaterialLayer() = default;
        MaterialLayer(const MaterialLayer& other)                 = delete;
        MaterialLayer& operator=(const MaterialLayer& other)      = delete;
        MaterialLayer(MaterialLayer&& other) noexcept;
        MaterialLayer& operator=(MaterialLayer&& other) noexcept;
        ~MaterialLayer() = default;

        inline constexpr bool operator==(const bool other) const noexcept { return (other == true && !m_TextureHandle.null()); }
        inline constexpr bool operator!=(const bool other) const noexcept { return !operator==(other); }

        inline constexpr Handle getTexture() const noexcept { return m_TextureHandle; }
        inline constexpr Handle getMask() const noexcept { return m_MaskHandle; }
        inline constexpr Handle getCubemap() const noexcept { return m_CubemapHandle; }

        inline constexpr const MaterialLayerTextureData& getMaterialLayerTextureData() const noexcept { return m_MaterialLayerTextureData; }
        inline constexpr const MaterialLayerMiscData& getMaterialLayerMiscData() const noexcept { return m_MaterialLayerMiscData; }
        inline constexpr MaterialLayerBlendMode blendMode() const noexcept { return (MaterialLayerBlendMode)((unsigned int)m_MaterialLayerTextureData.blendMode); }
        inline constexpr const glm::vec4& getUVModifications() const noexcept { return m_UVModifications; }

        inline void addUVModificationFunctor(uv_mod_func&& functor) { m_UVModificationQueue.emplace_back(std::move(functor)); }
        void addUVModificationSimpleTranslation(float translationX, float translationY);
        void addUVModificationSimpleMultiplication(float mulX, float mulY);

        inline void setBlendMode(MaterialLayerBlendMode mode) noexcept { m_MaterialLayerTextureData.blendMode = (float)mode; }
        void setTexture(Handle textureHandle) noexcept;
        void setMask(Handle maskHandle) noexcept;
        void setCubemap(Handle cubemapHandle) noexcept;
        void setTexture(const std::string& textureFile) noexcept;
        void setMask(const std::string& maskFile) noexcept;
        void setCubemap(const std::string& cubemapFile) noexcept;

        inline void setData1(float x, float y, float z, float w) noexcept { m_MaterialLayerTextureData = { x, y, z, w }; }
        inline void setData2(float r, float g, float b, float a) noexcept { m_MaterialLayerMiscData = { r, g, b, a }; }

        void sendDataToGPU(const std::string& uniform_component_string, size_t component_index, size_t layer_index, size_t& textureUnit) const;

        void update(const float dt);
};

#endif