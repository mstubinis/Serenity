#pragma once
#ifndef ENGINE_MATERIAL_LAYER_H
#define ENGINE_MATERIAL_LAYER_H

class  MaterialLayer;
class  Texture;

#include <serenity/resources/material/MaterialEnums.h>
#include <serenity/resources/Handle.h>
#include <serenity/dependencies/glm.h>
#include <vector>
#include <functional>

struct MaterialLayerTextureData final {
    float blendMode      = (float)MaterialLayerBlendMode::Default;
    float textureEnabled = 0.0f;
    float maskEnabled    = 0.0f;
    float cubemapEnabled = 0.0f;

    constexpr MaterialLayerTextureData() = default;
    constexpr MaterialLayerTextureData(const float blendMode_, const float textureEnabled_, const float maskEnabled_, const float cubemapEnabled_)
        : blendMode      { blendMode_ }
        , textureEnabled { textureEnabled_ }
        , maskEnabled    { maskEnabled_ }
        , cubemapEnabled { cubemapEnabled_ }
    {}
};
struct MaterialLayerMiscData final {
    float rMultiplier   = 0.0f;
    float gMultiplier   = 0.0f;
    float bMultiplier   = 0.0f;
    float aMultiplier   = 0.0f;

    constexpr MaterialLayerMiscData() = default;
    constexpr MaterialLayerMiscData(const float rMultiplier_, const float gMultiplier_, const float bMultiplier_, const float aMultiplier_)
        : rMultiplier{ rMultiplier_ }
        , gMultiplier{ gMultiplier_ }
        , bMultiplier{ bMultiplier_ }
        , aMultiplier{ aMultiplier_ }
    {}
};

class MaterialLayer final {
    struct FuncModifyUVs final {
        using FuncPtr = void(*)(const float x, const float y, const float dt, MaterialLayer& layer);

        float   x    = 0.0f;
        float   y    = 0.0f;
        FuncPtr func = nullptr;

        FuncModifyUVs(float x_, float y_, FuncPtr func_)
            : x { x_ }
            , y { y_ }
            , func{ func_ }
        {}
    };
    private:
        std::vector<FuncModifyUVs> m_UVModFuncs;

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
        MaterialLayer(const MaterialLayer&)                 = delete;
        MaterialLayer& operator=(const MaterialLayer&)      = delete;
        MaterialLayer(MaterialLayer&&) noexcept;
        MaterialLayer& operator=(MaterialLayer&&) noexcept;

        inline constexpr bool operator==(const bool other) const noexcept { return (other == true && !m_TextureHandle.null()); }
        inline constexpr bool operator!=(const bool other) const noexcept { return !operator==(other); }

        [[nodiscard]] inline constexpr Handle getTexture() const noexcept { return m_TextureHandle; }
        [[nodiscard]] inline constexpr Handle getMask() const noexcept { return m_MaskHandle; }
        [[nodiscard]] inline constexpr Handle getCubemap() const noexcept { return m_CubemapHandle; }

        [[nodiscard]] inline constexpr const MaterialLayerTextureData& getMaterialLayerTextureData() const noexcept { return m_MaterialLayerTextureData; }
        [[nodiscard]] inline constexpr const MaterialLayerMiscData& getMaterialLayerMiscData() const noexcept { return m_MaterialLayerMiscData; }
        [[nodiscard]] inline constexpr MaterialLayerBlendMode getBlendMode() const noexcept { return (MaterialLayerBlendMode)((unsigned int)m_MaterialLayerTextureData.blendMode); }
        [[nodiscard]] inline constexpr const glm::vec4& getUVModifications() const noexcept { return m_UVModifications; }

        inline void addUVModificationFunctor(FuncModifyUVs&& functor) { m_UVModFuncs.emplace_back(std::move(functor)); }
        void addUVModificationSimpleTranslation(float translationX, float translationY);
        void addUVModificationSimpleMultiplication(float mulX, float mulY);

        inline void setBlendMode(MaterialLayerBlendMode mode) noexcept { m_MaterialLayerTextureData.blendMode = (float)mode; }
        void setTexture(Handle textureHandle) noexcept;
        void setMask(Handle maskHandle) noexcept;
        void setCubemap(Handle cubemapHandle) noexcept;
        void setTexture(std::string_view textureFile) noexcept;
        void setMask(std::string_view maskFile) noexcept;
        void setCubemap(std::string_view cubemapFile) noexcept;

        inline void setTextureData(float x, float y, float z, float w) noexcept { m_MaterialLayerTextureData = { x, y, z, w }; }
        inline void setMiscData(float r, float g, float b, float a) noexcept { m_MaterialLayerMiscData = { r, g, b, a }; }
        inline void setTextureData(const glm::vec4& data) noexcept { setTextureData(data.r, data.g, data.b, data.a); }
        inline void setMiscData(const glm::vec4& data) noexcept { setMiscData(data.r, data.g, data.b, data.a); }

        void sendDataToGPU(const std::string& uniform_component_string, size_t component_index, size_t layer_index, size_t& textureUnit) const;

        void update(const float dt);
};

#endif