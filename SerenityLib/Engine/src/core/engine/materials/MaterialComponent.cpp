#include <core/engine/materials/MaterialComponent.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/scene/Skybox.h>
#include <core/engine/textures/Texture.h>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <boost/tuple/tuple.hpp>

using namespace Engine;
using namespace std;

MaterialComponent::MaterialComponent(const MaterialComponentType::Type& type, Texture* texture, Texture* mask, Texture* cubemap) {
    m_ComponentType = type;
    m_NumLayers = 0;

    auto& layer = m_Layers[m_NumLayers];
    layer.setTexture(texture);
    layer.setMask(mask);
    layer.setCubemap(cubemap);

    ++m_NumLayers;
}
MaterialComponent::~MaterialComponent() {
}
Texture* MaterialComponent::texture(const uint& index) const {
    return m_Layers[index].getTexture();
}
Texture* MaterialComponent::mask(const uint& index) const {
    return m_Layers[index].getMask();
}
Texture* MaterialComponent::cubemap(const uint& index) const {
    return m_Layers[index].getCubemap();
}
MaterialLayer& MaterialComponent::layer(const uint& index) {
    return m_Layers[index];
}
const MaterialComponentType::Type& MaterialComponent::type() const {
    return m_ComponentType;
}

void MaterialComponent::bind(const uint& component_index) {
    const string wholeString = "components[" + to_string(component_index) + "].";
    Renderer::sendUniform1Safe((wholeString + "numLayers").c_str(), static_cast<int>(m_NumLayers));
    Renderer::sendUniform1Safe((wholeString + "componentType").c_str(), static_cast<int>(m_ComponentType));
    for (unsigned int j = 0; j < m_NumLayers; ++j) {
        m_Layers[j].sendDataToGPU(wholeString, component_index, j);
    }
}
void MaterialComponent::unbind() {
}
void MaterialComponent::update(const float& dt) {
    for (unsigned int i = 0; i < m_NumLayers; ++i) {
        m_Layers[i].update(dt);
    }
}