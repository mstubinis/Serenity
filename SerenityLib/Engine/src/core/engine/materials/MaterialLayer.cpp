#include <core/engine/materials/MaterialLayer.h>
#include <core/engine/materials/MaterialLoader.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/system/Engine.h>

using namespace std;
using namespace Engine;
using namespace Engine::priv;

//TODO: implement the uv modification system
struct SimpleUVTranslationFunctor { void operator()(const float& dt, MaterialLayer& layer, const float& translationX, const float& translationY) const {
    auto& currentUVs = layer.getUVModifications();
    layer.m_UVModifications = currentUVs + glm::vec2(translationX * dt, translationY * dt);
    if (layer.m_UVModifications.x >= 5.0f)
        layer.m_UVModifications.x = 0.0f;
    if (layer.m_UVModifications.y > 5.0f)
        layer.m_UVModifications.y = 0.0f;
} };

MaterialLayer::MaterialLayer() {
    m_Texture = m_Mask = m_Cubemap = nullptr;
    m_Data1 = m_Data2 = glm::vec4(0.0f);
    m_UVModifications = glm::vec2(0.0f);
    setBlendMode(MaterialLayerBlendMode::Default);
}
MaterialLayer::~MaterialLayer() {

}
Texture* MaterialLayer::getTexture() const {
    return m_Texture;
}
Texture* MaterialLayer::getMask() const {
    return m_Mask;
}
Texture* MaterialLayer::getCubemap() const {
    return m_Cubemap;
}
const glm::vec4& MaterialLayer::data1() const {
    return m_Data1;
}
const glm::vec4& MaterialLayer::data2() const {
    return m_Data2;
}
void MaterialLayer::addUVModificationSimpleTranslation(const float& translationX, const float& translationY) {
    SimpleUVTranslationFunctor functor;
    std_uv_func f = std::bind<void>(functor, std::placeholders::_1, std::ref(*this), translationX, translationY);
    m_UVModificationQueue.push_back(std::move(f));
}
void MaterialLayer::addUVModificationFunctor(const std_uv_func& functor) {
    m_UVModificationQueue.push_back(std::move(functor));
}
void MaterialLayer::setBlendMode(const MaterialLayerBlendMode::Mode& mode) {
    m_Data1.x = static_cast<float>(mode);
}
const MaterialLayerBlendMode::Mode MaterialLayer::blendMode() const {
    return static_cast<MaterialLayerBlendMode::Mode>(static_cast<int>(m_Data1.x));
}
void MaterialLayer::setData1(const float& x, const float& y, const float& z, const float& w) {
    m_Data1.x = x;
    m_Data1.y = y;
    m_Data1.z = z;
    m_Data1.w = w;
}
void MaterialLayer::setData2(const float& x, const float& y, const float& z, const float& w) {
    m_Data2.x = x;
    m_Data2.y = y;
    m_Data2.z = z;
    m_Data2.w = w;
}
void MaterialLayer::setTexture(const string& textureFile) {
    Texture* _texture = Core::m_Engine->m_ResourceManager.HasResource<Texture>(textureFile);
    if (!_texture) {
        if (!textureFile.empty()) {
            _texture = NEW Texture(textureFile);
            Core::m_Engine->m_ResourceManager._addTexture(_texture);
        }
    }
    setTexture(_texture);
}
void MaterialLayer::setMask(const string& maskFile) {
    Texture* _texture = MaterialLoader::LoadTextureMask(maskFile);
    setMask(_texture);
}
void MaterialLayer::setCubemap(const string& cubemapFile) {
    Texture* _texture = MaterialLoader::LoadTextureCubemap(cubemapFile);
    setCubemap(_texture);
}
void MaterialLayer::setTexture(Texture* _texture) {
    if (!_texture /*|| _texture->type() != GL_TEXTURE_2D*/) {
        m_Data1.y = 0.0f;
        return;
    }
    m_Texture = _texture;
    m_Data1.y = m_Texture->compressed() ? 0.5f : 1.0f;
}
void MaterialLayer::setMask(Texture* _mask) {
    if (!_mask /*|| _mask->type() != GL_TEXTURE_2D*/) {
        m_Data1.z = 0.0f;
        return;
    }
    m_Mask = _mask;
    m_Data1.z = m_Mask->compressed() ? 0.5f : 1.0f;
}
void MaterialLayer::setCubemap(Texture* _cubemap) {
    if (!_cubemap /*|| _cubemap->type() != GL_TEXTURE_CUBE_MAP*/) {
        m_Data1.w = 0.0f;
        return;
    }
    m_Cubemap = _cubemap;
    m_Data1.w = m_Cubemap->compressed() ? 0.5f : 1.0f;
}

const glm::vec2& MaterialLayer::getUVModifications() const {
    return m_UVModifications;
}

void MaterialLayer::update(const float& dt) {
    const float& fDT = static_cast<float>(dt);
    //calculate uv modifications
    for (auto& command : m_UVModificationQueue) {
        command(fDT);
    }
}
void MaterialLayer::sendDataToGPU(const string& uniform_component_string, const size_t& component_index, const size_t& layer_index, size_t& textureUnit) {
    const string wholeString = uniform_component_string + "layers[" + to_string(layer_index) + "].";
    const auto start = (component_index * (MAX_MATERIAL_LAYERS_PER_COMPONENT * 3)) + (layer_index * 3);

    //m_Data1.y = (m_Texture && m_Texture->compressed()) ? 0.5f : 1.0f;
    //m_Data1.z = (m_Mask && m_Texture->compressed()) ? 0.5f : 1.0f;
    //m_Data1.w = (m_Cubemap && m_Texture->compressed()) ? 0.5f : 1.0f;

    if (m_Texture && m_Texture->address() != 0) {
        Engine::Renderer::sendTextureSafe((wholeString + "texture").c_str(), *m_Texture, int(textureUnit));
        ++textureUnit;
    }
    if (m_Mask && m_Mask->address() != 0) {
        Engine::Renderer::sendTextureSafe((wholeString + "mask").c_str(),    *m_Mask,    int(textureUnit));
        ++textureUnit;
    }
    if (m_Cubemap && m_Cubemap->address() != 0) {
        Engine::Renderer::sendTextureSafe((wholeString + "cubemap").c_str(), *m_Cubemap, int(textureUnit));
        ++textureUnit;
    }
    Engine::Renderer::sendUniform4Safe((wholeString + "data1").c_str(), m_Data1);
    Engine::Renderer::sendUniform4Safe((wholeString + "data2").c_str(), m_Data2);
    Engine::Renderer::sendUniform2Safe((wholeString + "uvModifications").c_str(), m_UVModifications);
}