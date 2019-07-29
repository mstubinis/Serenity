#include <core/engine/materials/MaterialLayer.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/renderer/Engine_Renderer.h>

using namespace std;
using namespace Engine;

//TODO: implement the uv modification system
struct SimpleUVTranslationFunctor { glm::vec2 operator()(MaterialLayer* layer, const float& translationX, const float& translationY, const float& dt) const {

} };

MaterialLayer::MaterialLayer() {
    m_Texture = m_Mask = m_Cubemap = nullptr;
    m_Data1 = m_Data2 = glm::vec4(0.0f);
    m_UVModifications = glm::vec2(0.0f);
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
    //SimpleUVTranslationFunctor functor;
    //boost_uv_func f = boost::bind<glm::vec2>(functor, this, translationX, translationY);
}
void MaterialLayer::addUVModificationFunctor(const boost_uv_func& functor) {
    //m_UVModificationQueue.push_back(std::move(functor));
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
void MaterialLayer::setTexture(Texture* _texture) {
    if (!_texture || _texture->type() != GL_TEXTURE_2D)
        return;
    m_Texture = _texture;
    m_Data1.y = m_Texture->compressed() ? 0.5f : 1.0f;
}
void MaterialLayer::setMask(Texture* _mask) {
    if (!_mask || _mask->type() != GL_TEXTURE_2D)
        return;
    m_Mask = _mask;
    m_Data1.z = m_Texture->compressed() ? 0.5f : 1.0f;
}
void MaterialLayer::setCubemap(Texture* _cubemap) {
    if (!_cubemap || _cubemap->type() != GL_TEXTURE_CUBE_MAP)
        return;
    m_Cubemap = _cubemap;
    m_Data1.w = m_Texture->compressed() ? 0.5f : 1.0f;
}

const glm::vec2& MaterialLayer::getUVModifications() const {
    return m_UVModifications;
}

void MaterialLayer::update(const double& dt) {
    //calculate uv modifications
    for (auto& command : m_UVModificationQueue) {
        //m_UVModifications = command(this, m_UVModifications.x, m_UVModifications.y, dt);
    }
}
void MaterialLayer::sendDataToGPU(const string& uniform_component_string, const unsigned int& component_index, const unsigned int& layer_index) {
    const string wholeString = uniform_component_string + "layers[" + to_string(layer_index) + "].";
    const int start = (component_index * 9) + (layer_index * 3);

    Renderer::sendUniform4Safe((wholeString + "data1").c_str(), m_Data1);
    Renderer::sendUniform4Safe((wholeString + "data2").c_str(), m_Data2);
    Renderer::sendUniform2Safe((wholeString + "uvModifications").c_str(), m_UVModifications);

    if (m_Texture && m_Texture->address() != 0) {
        const int slot_texture = start + 0;
        Renderer::sendTextureSafe((wholeString + "texture").c_str(), *m_Texture, slot_texture);
    }
    if (m_Mask && m_Mask->address() != 0) {
        const int slot_mask = start + 1;
        Renderer::sendTextureSafe((wholeString + "mask").c_str(),    *m_Mask, slot_mask);
    }
    if (m_Cubemap && m_Cubemap->address() != 0) {
        const int slot_cubemap = start + 2;
        Renderer::sendTextureSafe((wholeString + "cubemap").c_str(), *m_Cubemap, slot_cubemap);
    }
}