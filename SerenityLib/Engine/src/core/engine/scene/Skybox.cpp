#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/scene/Skybox.h>
#include <core/engine/system/Engine.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/textures/TextureLoader.h>
#include <core/engine/events/Event.h>

using namespace Engine;

GLuint m_Buffer   = 0;
GLuint m_VAO      = 0;

constexpr std::array<glm::vec3, 36> VERTICES = {
    glm::vec3(-1, 1, 1),
    glm::vec3(1, 1, 1),
    glm::vec3(1, -1, 1),
    glm::vec3(-1, 1, 1),
    glm::vec3(1, -1, 1),
    glm::vec3(-1, -1, 1),
    glm::vec3(-1, 1, -1),
    glm::vec3(-1, -1, -1),
    glm::vec3(1, -1, -1),
    glm::vec3(-1, 1, -1),
    glm::vec3(1, -1, -1),
    glm::vec3(1, 1, -1),
    glm::vec3(-1, 1, 1),
    glm::vec3(-1, -1, 1),
    glm::vec3(-1, -1, -1),
    glm::vec3(-1, 1, 1),
    glm::vec3(-1, -1, -1),
    glm::vec3(-1, 1, -1),
    glm::vec3(-1, -1, 1),
    glm::vec3(1, -1, 1),
    glm::vec3(1, -1, -1),
    glm::vec3(-1, -1, 1),
    glm::vec3(1, -1, -1),
    glm::vec3(-1, -1, -1),
    glm::vec3(1, -1, 1),
    glm::vec3(1, 1, 1),
    glm::vec3(1, 1, -1),
    glm::vec3(1, -1, 1),
    glm::vec3(1, 1, -1),
    glm::vec3(1, -1, -1),
    glm::vec3(1, 1, 1),
    glm::vec3(-1, 1, 1),
    glm::vec3(-1, 1, -1),
    glm::vec3(1, 1, 1),
    glm::vec3(-1, 1, -1),
    glm::vec3(1, 1, -1),
};

/*
when saving a skybox in gimp, the layer ordering should be:
    - left
    - right
    - top
    - bottom
    - back    - should be front file
    - front   - should be back file

(switch the front and back files)

*/

namespace Engine::priv {
    class SkyboxImplInterface final {
        friend class Skybox;
        static void bindDataToGPU() noexcept {
            GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_Buffer));
            GLCall(glEnableVertexAttribArray(0));
            GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
        }
        static void buildVAO() noexcept {
            Engine::Renderer::deleteVAO(m_VAO);
            if (Engine::priv::Renderer::OPENGL_VERSION >= 30) {
                Engine::Renderer::genAndBindVAO(m_VAO);
                Engine::priv::SkyboxImplInterface::bindDataToGPU();
                Engine::Renderer::bindVAO(0);
            }
        }
        static void initMesh() noexcept {
            if (m_Buffer != 0U) {
                return;
            }
            GLCall(glGenBuffers(1, &m_Buffer));
            GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_Buffer));
            GLCall(glBufferData(GL_ARRAY_BUFFER, VERTICES.size() * sizeof(glm::vec3), &VERTICES[0], GL_STATIC_DRAW));
            Engine::priv::SkyboxImplInterface::buildVAO();
        }
    };
};

Skybox::Skybox(const std::string* files) {
    Engine::priv::SkyboxImplInterface::initMesh();

    std::string names[6] = { files[0], files[1], files[2], files[3], files[4], files[5] };
    //instead of using files[0] generate a proper name using the directory?
    m_Texture = NEW Texture(names, files[0] + "Cubemap", false, ImageInternalFormat::SRGB8_ALPHA8);
    Engine::priv::TextureLoader::GeneratePBRData(*m_Texture, 32, m_Texture->width() / 4);
    Engine::priv::Core::m_Engine->m_ResourceManager._addTexture(m_Texture);
    registerEvent(EventType::WindowFullscreenChanged);
}
Skybox::Skybox(const std::string& filename) {
    Engine::priv::SkyboxImplInterface::initMesh();

    m_Texture = Engine::priv::Core::m_Engine->m_ResourceManager.HasResource<Texture>(filename);
    if (!m_Texture) {
        m_Texture = NEW Texture(filename, false, ImageInternalFormat::SRGB8_ALPHA8);
        Engine::priv::TextureLoader::GeneratePBRData(*m_Texture, 32, m_Texture->width() / 4);
        Engine::priv::Core::m_Engine->m_ResourceManager._addTexture(m_Texture);
    }
    registerEvent(EventType::WindowFullscreenChanged);
}
Skybox::~Skybox() {
    unregisterEvent(EventType::WindowFullscreenChanged);
}

void Skybox::bindMesh() {
    Engine::priv::SkyboxImplInterface::initMesh();
    if(m_VAO){
        Engine::Renderer::bindVAO(m_VAO);
        GLCall(glDrawArrays( GL_TRIANGLES, 0, (GLsizei)VERTICES.size() ));
    }else{
        Engine::priv::SkyboxImplInterface::bindDataToGPU();
        GLCall(glDrawArrays( GL_TRIANGLES, 0, (GLsizei)VERTICES.size() ));
        GLCall(glDisableVertexAttribArray(0));
    }
}
void Skybox::onEvent(const Event& e) {
    if (e.type == EventType::WindowFullscreenChanged) {
        Engine::priv::SkyboxImplInterface::buildVAO();
    }
}