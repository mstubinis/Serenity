#include <core/engine/materials/MaterialLayer.h>
#include <core/engine/textures/Texture.h>

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
void MaterialLayer::setTexture(Texture* _texture) {
    if (!_texture || _texture->type() != GL_TEXTURE_2D)
        return;
    m_Texture = _texture;
}
void MaterialLayer::setMask(Texture* _mask) {
    if (!_mask || _mask->type() != GL_TEXTURE_2D)
        return;
    m_Mask = _mask;
}
void MaterialLayer::setCubemap(Texture* _cubemap) {
    if (!_cubemap || _cubemap->type() != GL_TEXTURE_CUBE_MAP)
        return;
    m_Cubemap = _cubemap;
}

const glm::vec2& MaterialLayer::getUVModifications() const {
    return m_UVModifications;
}

void MaterialLayer::update(const double& dt) {
    //calculate uv modifications
}
void MaterialLayer::sendDataToGPU() {

}