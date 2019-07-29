#include <core/engine/materials/MaterialLayer.h>
#include <core/engine/textures/Texture.h>

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
    for (auto& command : m_UVModificationQueue) {
        //m_UVModifications = command(this, m_UVModifications.x, m_UVModifications.y, dt);
    }
}
void MaterialLayer::sendDataToGPU() {

}