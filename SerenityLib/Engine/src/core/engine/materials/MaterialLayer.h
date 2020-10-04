#pragma once
#ifndef ENGINE_MATERIAL_LAYER_H
#define ENGINE_MATERIAL_LAYER_H

class  MaterialLayer;
class  Texture;
struct SimpleUVTranslationFunctor;

#include <core/engine/materials/MaterialEnums.h>
#include <core/engine/resources/Handle.h>

using uv_mod_func = std::function<void(const float dt, MaterialLayer& layer)>;

class MaterialLayer final {
    friend struct SimpleUVTranslationFunctor;
    private:
        std::vector<uv_mod_func>   m_UVModificationQueue;

        //x = blend mode? | y = texture enabled? | z = mask enabled? | w = cubemap enabled?
        glm::vec4                  m_Data1                         = glm::vec4((float)MaterialLayerBlendMode::Default, 0.0f, 0.0f, 0.0f);
        glm::vec4                  m_Data2                         = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

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

        inline CONSTEXPR Handle getTexture() const noexcept { return m_TextureHandle; }
        inline CONSTEXPR Handle getMask() const noexcept { return m_MaskHandle; }
        inline CONSTEXPR Handle getCubemap() const noexcept { return m_CubemapHandle; }

        inline CONSTEXPR const glm::vec4& data1() const noexcept { return m_Data1; }
        inline CONSTEXPR const glm::vec4& data2() const noexcept { return m_Data2; }
        inline CONSTEXPR MaterialLayerBlendMode blendMode() const noexcept { return (MaterialLayerBlendMode)((unsigned int)m_Data1.x); }
        inline CONSTEXPR const glm::vec4& getUVModifications() const noexcept { return m_UVModifications; }

        inline void addUVModificationFunctor(uv_mod_func&& functor) { m_UVModificationQueue.emplace_back(std::move(functor)); }
        void addUVModificationSimpleTranslation(float translationX, float translationY);
        void addUVModificationSimpleMultiplication(float mulX, float mulY);

        inline void setBlendMode(MaterialLayerBlendMode mode) noexcept { m_Data1.x = (float)mode; }
        void setTexture(Handle textureHandle) noexcept;
        void setMask(Handle maskHandle) noexcept;
        void setCubemap(Handle cubemapHandle) noexcept;
        void setTexture(const std::string& textureFile) noexcept;
        void setMask(const std::string& maskFile) noexcept;
        void setCubemap(const std::string& cubemapFile) noexcept;

        inline void setData1(float x, float y, float z, float w) noexcept { m_Data1.x = x; m_Data1.y = y; m_Data1.z = z; m_Data1.w = w; }
        inline void setData2(float x, float y, float z, float w) noexcept { m_Data2.x = x; m_Data2.y = y; m_Data2.z = z; m_Data2.w = w; }

        void sendDataToGPU(const std::string& uniform_component_string, size_t component_index, size_t layer_index, size_t& textureUnit) const;

        void update(const float dt);
};

#endif