#pragma once
#ifndef ENGINE_MATERIAL_LAYER_H
#define ENGINE_MATERIAL_LAYER_H

class  MaterialComponent;
class  MaterialLayer;
class  Texture;

#include <serenity/resources/material/MaterialEnums.h>
#include <serenity/resources/Handle.h>
#include <serenity/dependencies/glm.h>
#include <vector>
#include <functional>

struct MaterialLayerBaseData final {
    float enabled  = 0.0f;
    float unused_1 = 0.0f;
    float unused_2 = 0.0f;
    float unused_3 = 0.0f;

    constexpr MaterialLayerBaseData() = default;
    constexpr MaterialLayerBaseData(const float enabled_, const float unused_1_, const float unused_2_, const float unused_3_)
        : enabled{ enabled_ }
        , unused_1{ unused_1_ }
        , unused_2{ unused_2_ }
        , unused_3{ unused_3_ }
    {}

    operator glm::vec4() const noexcept {
        return glm::vec4{ enabled, unused_1, unused_2, unused_3 };
    }
};
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

    operator glm::vec4() const noexcept {
        return glm::vec4{ blendMode, textureEnabled, maskEnabled, cubemapEnabled };
    }
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

    operator glm::vec4() const noexcept {
        return glm::vec4{ rMultiplier, gMultiplier, bMultiplier, aMultiplier };
    }
};

class MaterialLayer final {
    friend class MaterialComponent;
    struct FuncModifyUVs final {
        using FuncPtr = void(*)(const float x, const float y, const float dt, MaterialLayer&);

        FuncPtr func = nullptr;
        float   x    = 0.0f;
        float   y    = 0.0f;

        FuncModifyUVs(float x_, float y_, FuncPtr func_)
            : func{ func_ }
            , x { x_ }
            , y { y_ }
        {}
    };
    private:
        std::vector<FuncModifyUVs> m_UVModFuncs;

        MaterialLayerBaseData      m_MaterialLayerBaseData;
        MaterialLayerTextureData   m_MaterialLayerTextureData;
        MaterialLayerMiscData      m_MaterialLayerMiscData;

        //x = translationX, y = translationY, z = multX, w = multY
        glm::vec4                  m_UVModifications            = glm::vec4{ 0.0f, 0.0f, 1.0f, 1.0f };

        Handle                     m_TextureHandle;
        Handle                     m_MaskHandle;
        Handle                     m_CubemapHandle;
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

        [[nodiscard]] inline constexpr const MaterialLayerBaseData& getMaterialLayerBaseData() const noexcept { return m_MaterialLayerBaseData; }
        [[nodiscard]] inline constexpr const MaterialLayerTextureData& getMaterialLayerTextureData() const noexcept { return m_MaterialLayerTextureData; }
        [[nodiscard]] inline constexpr const MaterialLayerMiscData& getMaterialLayerMiscData() const noexcept { return m_MaterialLayerMiscData; }
        [[nodiscard]] inline constexpr MaterialLayerBlendMode getBlendMode() const noexcept { return MaterialLayerBlendMode(uint32_t(m_MaterialLayerTextureData.blendMode)); }
        [[nodiscard]] inline constexpr const glm::vec4& getUVModifications() const noexcept { return m_UVModifications; }

        inline void addUVModificationFunctor(FuncModifyUVs&& functor) { m_UVModFuncs.emplace_back(std::move(functor)); }
        void addUVModificationSimpleTranslation(float translationX, float translationY);
        void addUVModificationSimpleMultiplication(float mulX, float mulY);

        inline void setBlendMode(MaterialLayerBlendMode mode) noexcept { m_MaterialLayerTextureData.blendMode = float(mode); }
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

        void sendDataToGPU(const std::string& uniform_component_string, size_t layer_index, int& textureUnit) const;

        void update(const float dt);
};

#endif