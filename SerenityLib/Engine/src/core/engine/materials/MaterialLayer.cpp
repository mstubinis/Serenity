#include <core/engine/materials/MaterialLayer.h>
#include <core/engine/materials/MaterialLoader.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/system/Engine.h>

using namespace std;
using namespace Engine;
using namespace Engine::priv;

MaterialLayer::MaterialLayer() {
}
MaterialLayer::~MaterialLayer() {
}
void MaterialLayer::addUVModificationSimpleTranslation(float translationX, float translationY) {
    m_UVModificationQueue.push_back([translationX, translationY](const float dt, MaterialLayer& layer) {
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
    m_UVModificationQueue.push_back([mulX, mulY](const float dt, MaterialLayer& layer) {
        layer.m_UVModifications.z = mulX;
        layer.m_UVModifications.w = mulY;
    });
}

void MaterialLayer::addUVModificationFunctor(uv_mod_func functor) {
    m_UVModificationQueue.push_back(std::move(functor));
}
void MaterialLayer::setBlendMode(MaterialLayerBlendMode::Mode mode) {
    m_Data1.x = static_cast<float>(mode);
}
void MaterialLayer::setData1(float x, float y, float z, float w) {
    m_Data1.x = x;
    m_Data1.y = y;
    m_Data1.z = z;
    m_Data1.w = w;
}
void MaterialLayer::setData2(float x, float y, float z, float w) {
    m_Data2.x = x;
    m_Data2.y = y;
    m_Data2.z = z;
    m_Data2.w = w;
}
void MaterialLayer::setTexture(const string& textureFile) {
    Texture* texture = Core::m_Engine->m_ResourceManager.HasResource<Texture>(textureFile);
    if (!texture) {
        if (!textureFile.empty()) {
            texture = NEW Texture(textureFile);
            Core::m_Engine->m_ResourceManager._addTexture(texture);
        }
    }
    setTexture(texture);
}
void MaterialLayer::setMask(const string& maskFile) {
    Texture* texture = MaterialLoader::LoadTextureMask(maskFile);
    setMask(texture);
}
void MaterialLayer::setCubemap(const string& cubemapFile) {
    Texture* texture = MaterialLoader::LoadTextureCubemap(cubemapFile);
    setCubemap(texture);
}
void MaterialLayer::setTexture(Texture* texture) {
    if (!texture /*|| _texture->type() != GL_TEXTURE_2D*/) {
        m_Data1.y = 0.0f;
        return;
    }
    m_Texture = texture;
    m_Data1.y = m_Texture->compressed() ? 0.5f : 1.0f;
}
void MaterialLayer::setMask(Texture* mask) {
    if (!mask /*|| _mask->type() != GL_TEXTURE_2D*/) {
        m_Data1.z = 0.0f;
        return;
    }
    m_Mask = mask;
    m_Data1.z = m_Mask->compressed() ? 0.5f : 1.0f;
}
void MaterialLayer::setCubemap(Texture* cubemap) {
    if (!cubemap /*|| _cubemap->type() != GL_TEXTURE_CUBE_MAP*/) {
        m_Data1.w = 0.0f;
        return;
    }
    m_Cubemap = cubemap;
    m_Data1.w = m_Cubemap->compressed() ? 0.5f : 1.0f;
}

void MaterialLayer::update(const float dt) {
    //calculate uv modifications
    for (auto& command : m_UVModificationQueue) {
        command(dt, *this);
    }
}
void MaterialLayer::sendDataToGPU(const string& uniform_component_string, size_t component_index, size_t layer_index, size_t& textureUnit) const {
    const string wholeString = uniform_component_string + "layers[" + to_string(layer_index) + "].";
    const auto start = (component_index * (MAX_MATERIAL_LAYERS_PER_COMPONENT * 3)) + (layer_index * 3);

    //m_Data1.y = (m_Texture && m_Texture->compressed()) ? 0.5f : 1.0f;
    //m_Data1.z = (m_Mask && m_Texture->compressed()) ? 0.5f : 1.0f;
    //m_Data1.w = (m_Cubemap && m_Texture->compressed()) ? 0.5f : 1.0f;

    if (m_Texture && m_Texture->address() != 0U) {
        Engine::Renderer::sendTextureSafe((wholeString + "texture").c_str(), *m_Texture, static_cast<int>(textureUnit));
        ++textureUnit;
    }
    if (m_Mask && m_Mask->address() != 0U) {
        Engine::Renderer::sendTextureSafe((wholeString + "mask").c_str(),    *m_Mask,    static_cast<int>(textureUnit));
        ++textureUnit;
    }
    if (m_Cubemap && m_Cubemap->address() != 0U) {
        Engine::Renderer::sendTextureSafe((wholeString + "cubemap").c_str(), *m_Cubemap, static_cast<int>(textureUnit));
        ++textureUnit;
    }
    Engine::Renderer::sendUniform4Safe((wholeString + "data1").c_str(), m_Data1);
    Engine::Renderer::sendUniform4Safe((wholeString + "data2").c_str(), m_Data2);
    Engine::Renderer::sendUniform4Safe((wholeString + "uvModifications").c_str(), m_UVModifications);
}