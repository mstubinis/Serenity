#include <serenity/resources/material/MaterialLayer.h>
#include <serenity/resources/material/MaterialLoader.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/resources/texture/TextureCubemap.h>
#include <serenity/renderer/Renderer.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/system/Engine.h>

MaterialLayer::MaterialLayer(MaterialLayer&& other) noexcept 
    : m_UVModFuncs               { std::move(other.m_UVModFuncs) }
    , m_MaterialLayerBaseData    { std::move(other.m_MaterialLayerBaseData) }
    , m_MaterialLayerTextureData { std::move(other.m_MaterialLayerTextureData) }
    , m_MaterialLayerMiscData    { std::move(other.m_MaterialLayerMiscData) }
    , m_UVModifications          { std::move(other.m_UVModifications) }
    , m_TextureHandle            { std::move(other.m_TextureHandle) }
    , m_MaskHandle               { std::move(other.m_MaskHandle) }
    , m_CubemapHandle            { std::move(other.m_CubemapHandle) }
{}
MaterialLayer& MaterialLayer::operator=(MaterialLayer&& other) noexcept {
    m_UVModFuncs               = std::move(other.m_UVModFuncs);
    m_MaterialLayerBaseData    = std::move(other.m_MaterialLayerBaseData);
    m_MaterialLayerTextureData = std::move(other.m_MaterialLayerTextureData);
    m_MaterialLayerMiscData    = std::move(other.m_MaterialLayerMiscData);
    m_UVModifications          = std::move(other.m_UVModifications);
    m_TextureHandle            = std::move(other.m_TextureHandle);
    m_MaskHandle               = std::move(other.m_MaskHandle);
    m_CubemapHandle            = std::move(other.m_CubemapHandle);
    return *this;
}

void MaterialLayer::addUVModificationSimpleTranslation(float translationX, float translationY) {
    m_UVModFuncs.emplace_back(translationX, translationY, [](const float x, const float y, const float dt, MaterialLayer& layer) {
        auto currentUVs           = glm::vec2(layer.getUVModifications().x, layer.getUVModifications().y);
        auto translatedUVS        = currentUVs + glm::vec2(x * dt, y * dt);
        layer.m_UVModifications.x = translatedUVS.x;
        layer.m_UVModifications.y = translatedUVS.y;
        if (layer.m_UVModifications.x >= 100.0f) {
            layer.m_UVModifications.x -= 100.0f;
        }
        if (layer.m_UVModifications.y >= 100.0f) {
            layer.m_UVModifications.y -= 100.0f;
        }
    });
}
void MaterialLayer::addUVModificationSimpleMultiplication(float mulX, float mulY) {
    m_UVModFuncs.emplace_back(mulX, mulY, [](const float x, const float y, const float dt, MaterialLayer& layer) {
        layer.m_UVModifications.z = x;
        layer.m_UVModifications.w = y;
    });
}

void MaterialLayer::setTexture(std::string_view textureFile) noexcept {
    auto texture = Engine::Resources::getResource<Texture>(textureFile);
    if (!texture.m_Resource) {
        if (!textureFile.empty()) {
            texture.m_Handle   = Engine::Resources::loadTextureAsync(textureFile, ImageInternalFormat::SRGB8_ALPHA8, false);
            texture.m_Resource = texture.m_Handle.get<Texture>();
        }
    }
    setTexture(texture.m_Handle);
}
void MaterialLayer::setMask(std::string_view maskFile) noexcept {
    auto texture = Engine::priv::MaterialLoader::LoadTextureMask(maskFile);
    setMask(texture.m_Handle);
}
void MaterialLayer::setCubemap(std::string_view cubemapFile) noexcept {
    auto texture = Engine::priv::MaterialLoader::LoadTextureCubemap(cubemapFile);
    setCubemap(texture.m_Handle);
}
void MaterialLayer::internal_set_texture_and_property(Handle textureHandleValue, Handle& textureHandle, float& propertyVar, float value) noexcept {
    textureHandle = textureHandleValue;
    propertyVar   = value;
}
void MaterialLayer::setTexture(Handle textureHandle) noexcept {
    internal_set_texture_and_property(textureHandle, m_TextureHandle, m_MaterialLayerTextureData.textureEnabled, internal_get_texture_compression_value<Texture>(textureHandle));
}
void MaterialLayer::setMask(Handle maskHandle) noexcept {
    internal_set_texture_and_property(maskHandle, m_MaskHandle, m_MaterialLayerTextureData.maskEnabled, internal_get_texture_compression_value<Texture>(maskHandle));
}
void MaterialLayer::setCubemap(Handle cubemapHandle) noexcept {
    internal_set_texture_and_property(cubemapHandle, m_CubemapHandle, m_MaterialLayerTextureData.cubemapEnabled, internal_get_texture_compression_value<TextureCubemap>(cubemapHandle));
}

void MaterialLayer::update(const float dt) {
    for (const auto& functionItr : m_UVModFuncs) {
        functionItr.func(functionItr.x, functionItr.y, dt, *this);
    }
}
void MaterialLayer::sendDataToGPU(const std::string& uniform_component_str, size_t component_idx, size_t layer_idx, size_t& textureUnit) const {
    const std::string wholeString = uniform_component_str + "layers[" + std::to_string(layer_idx) + "].";
    if (!m_TextureHandle.null()){
        auto& texture = *m_TextureHandle.get<Texture>();
        if (!texture.isLoaded() || texture.address() != 0U) {
            Engine::Renderer::sendTextureSafe((wholeString + "texture").c_str(), texture, (int)textureUnit);
            ++textureUnit;
        }
    }
    if (!m_MaskHandle.null()){
        auto& mask = *m_MaskHandle.get<Texture>();
        if (!mask.isLoaded() || mask.address() != 0U) {
            Engine::Renderer::sendTextureSafe((wholeString + "mask").c_str(), mask, (int)textureUnit);
            ++textureUnit;
        }
    }
    if (!m_CubemapHandle.null() ){
        auto& cubemap = *m_CubemapHandle.get<TextureCubemap>();
        if (!cubemap.isLoaded() || cubemap.address() != 0U) {
            Engine::Renderer::sendTextureSafe((wholeString + "cubemap").c_str(), cubemap, (int)textureUnit);
            ++textureUnit;
        }
    }
    Engine::Renderer::sendUniform4Safe((wholeString + "data0").c_str(),
        m_MaterialLayerBaseData.enabled,
        m_MaterialLayerBaseData.unused_1,
        m_MaterialLayerBaseData.unused_2,
        m_MaterialLayerBaseData.unused_3
    );
    Engine::Renderer::sendUniform4Safe((wholeString + "data1").c_str(), 
        m_MaterialLayerTextureData.blendMode, 
        m_MaterialLayerTextureData.textureEnabled, 
        m_MaterialLayerTextureData.maskEnabled, 
        m_MaterialLayerTextureData.cubemapEnabled
    );
    Engine::Renderer::sendUniform4Safe((wholeString + "data2").c_str(), 
        m_MaterialLayerMiscData.rMultiplier,
        m_MaterialLayerMiscData.gMultiplier,
        m_MaterialLayerMiscData.bMultiplier,
        m_MaterialLayerMiscData.aMultiplier
    );
    Engine::Renderer::sendUniform4Safe((wholeString + "uvModifications").c_str(), m_UVModifications);
}