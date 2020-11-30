#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/materials/MaterialLayer.h>
#include <core/engine/materials/MaterialLoader.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/system/Engine.h>


MaterialLayer::MaterialLayer(MaterialLayer&& other) noexcept 
    : m_UVModificationQueue      { std::move(other.m_UVModificationQueue) }
    , m_MaterialLayerTextureData { std::move(other.m_MaterialLayerTextureData) }
    , m_MaterialLayerMiscData    { std::move(other.m_MaterialLayerMiscData) }
    , m_UVModifications          { std::move(other.m_UVModifications) }
    , m_TextureHandle            { std::move(other.m_TextureHandle) }
    , m_MaskHandle               { std::move(other.m_MaskHandle) }
    , m_CubemapHandle            { std::move(other.m_CubemapHandle) }
{}
MaterialLayer& MaterialLayer::operator=(MaterialLayer&& other) noexcept {
    m_UVModificationQueue      = std::move(other.m_UVModificationQueue);
    m_MaterialLayerTextureData = std::move(other.m_MaterialLayerTextureData);
    m_MaterialLayerMiscData    = std::move(other.m_MaterialLayerMiscData);
    m_UVModifications          = std::move(other.m_UVModifications);
    m_TextureHandle            = std::move(other.m_TextureHandle);
    m_MaskHandle               = std::move(other.m_MaskHandle);
    m_CubemapHandle            = std::move(other.m_CubemapHandle);
    return *this;
}

void MaterialLayer::addUVModificationSimpleTranslation(float translationX, float translationY) {
    m_UVModificationQueue.emplace_back([translationX, translationY](const float dt, MaterialLayer& layer) {
        auto currentUVs           = glm::vec2(layer.getUVModifications().x, layer.getUVModifications().y);
        auto translatedUVS        = currentUVs + glm::vec2(translationX * dt, translationY * dt);
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
    m_UVModificationQueue.emplace_back([mulX, mulY](const float dt, MaterialLayer& layer) {
        layer.m_UVModifications.z = mulX;
        layer.m_UVModifications.w = mulY;
    });
}

void MaterialLayer::setTexture(const std::string& textureFile) noexcept {
    auto texture = Engine::Resources::getResource<Texture>(textureFile);
    if (!texture.first) {
        if (!textureFile.empty()) {
            texture.second = Engine::Resources::loadTextureAsync(textureFile, ImageInternalFormat::SRGB8_ALPHA8, false);
            texture.first  = texture.second.get<Texture>();
        }
    }
    setTexture(texture.second);
}
void MaterialLayer::setMask(const std::string& maskFile) noexcept {
    auto texture = Engine::priv::MaterialLoader::LoadTextureMask(maskFile);
    setMask(texture.second);
}
void MaterialLayer::setCubemap(const std::string& cubemapFile) noexcept {
    auto texture = Engine::priv::MaterialLoader::LoadTextureCubemap(cubemapFile);
    setCubemap(texture.second);
}
void MaterialLayer::internal_set_texture_and_property(Handle textureHandleValue, Handle& textureHandle, float& propertyVar, float value) noexcept {
    textureHandle = textureHandleValue;
    propertyVar   = value;
}
float MaterialLayer::internal_get_texture_compression_value(Handle textureHandle) noexcept {
    if (!textureHandle.null()) {
        auto mutex = textureHandle.getMutex();
        bool isCompressed = false;
        if (mutex) {
            std::lock_guard lock(*mutex);
            isCompressed = textureHandle.get<Texture>()->compressed();
        }
        return isCompressed ? 0.5f : 1.0f;
    }
    return 0.0f;
}
void MaterialLayer::setTexture(Handle textureHandle) noexcept {
    internal_set_texture_and_property(textureHandle, m_TextureHandle, m_MaterialLayerTextureData.textureEnabled, internal_get_texture_compression_value(textureHandle));
}
void MaterialLayer::setMask(Handle maskHandle) noexcept {
    internal_set_texture_and_property(maskHandle, m_MaskHandle, m_MaterialLayerTextureData.maskEnabled, internal_get_texture_compression_value(maskHandle));
}
void MaterialLayer::setCubemap(Handle cubemapHandle) noexcept {
    internal_set_texture_and_property(cubemapHandle, m_CubemapHandle, m_MaterialLayerTextureData.cubemapEnabled, internal_get_texture_compression_value(cubemapHandle));
}

void MaterialLayer::update(const float dt) {
    std::for_each(std::cbegin(m_UVModificationQueue), std::cend(m_UVModificationQueue), [this, dt](auto& command) {
        command(dt, *this);
    });
}
void MaterialLayer::sendDataToGPU(const std::string& uniform_component_string, size_t component_index, size_t layer_index, size_t& textureUnit) const {
    std::string wholeString = uniform_component_string + "layers[" + std::to_string(layer_index) + "].";
    //auto start            = (component_index * (MAX_MATERIAL_LAYERS_PER_COMPONENT * 3)) + (layer_index * 3);
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
        auto& cubemap = *m_CubemapHandle.get<Texture>();
        if (!cubemap.isLoaded() || cubemap.address() != 0U) {
            Engine::Renderer::sendTextureSafe((wholeString + "cubemap").c_str(), cubemap, (int)textureUnit);
            ++textureUnit;
        }
    }
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