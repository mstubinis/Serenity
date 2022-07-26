
#include <serenity/scene/Skybox.h>
#include <serenity/system/Engine.h>
#include <serenity/resources/texture/TextureCubemap.h>
#include <serenity/resources/texture/TextureLoaderCubemap.h>
#include <serenity/events/Event.h>
#include <serenity/resources/mesh/gl/VertexArrayObject.h>
#include <serenity/resources/mesh/gl/VertexBufferObject.h>

namespace {
    VertexBufferObject m_VBO;
    VertexArrayObject  m_VAO;

    constexpr const std::array<glm::vec3, 36> VERTICES = {
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

    void bindDataToGPU() noexcept {
        m_VBO.bind();
        m_VBO.specifyVertexAttribute(0, 3, GL_FLOAT, false, 0, 0);
    }
    void buildVAO() noexcept {
        m_VAO.deleteVAO();
        if (Engine::priv::OpenGLState::constants.supportsVAO()) {
            m_VAO.generateVAO();
            m_VAO.bindVAO();
            bindDataToGPU();
            Engine::Renderer::bindVAO(0);
        }
    }
    void initMesh() noexcept {
        if (m_VBO != 0) {
            return;
        }
        m_VBO.generate();
        m_VBO.bind();
        m_VBO.setData(VERTICES.size() * sizeof(glm::vec3), VERTICES.data(), BufferDataDrawType::Static);
        buildVAO();
    }
}

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

Skybox::Skybox(const std::array<std::string_view, 6>& files) {
    initMesh();
    //instead of using files[0] generate a proper name using the directory?

    m_TextureCubemap = Engine::Resources::addResource<TextureCubemap>(files, std::string(files[0]) + "Cubemap", false, ImageInternalFormat::SRGB8_ALPHA8);
    Engine::priv::TextureLoaderCubemap::GeneratePBRData(*m_TextureCubemap.get<TextureCubemap>(), 32, m_TextureCubemap.get<TextureCubemap>()->width() / 4);

    registerEvent(EventType::WindowFullscreenChanged);
}
Skybox::Skybox(std::string_view filename) {
    initMesh();

    m_TextureCubemap = Engine::getResourceManager().m_ResourceModule.get<TextureCubemap>(filename).m_Handle;
    if (!m_TextureCubemap) {
        m_TextureCubemap = Engine::Resources::addResource<TextureCubemap>(filename, false, ImageInternalFormat::SRGB8_ALPHA8);
        Engine::priv::TextureLoaderCubemap::GeneratePBRData(*m_TextureCubemap.get<TextureCubemap>(), 32, m_TextureCubemap.get<TextureCubemap>()->width() / 4);
    }
    registerEvent(EventType::WindowFullscreenChanged);
}
Skybox::~Skybox() {
    unregisterEvent(EventType::WindowFullscreenChanged);
}

void Skybox::bindMesh() {
    initMesh();
    if (m_VAO) {
        m_VAO.bindVAO();
        glDrawArrays( GL_TRIANGLES, 0, GLsizei(VERTICES.size()) );
        m_VAO.unbindVAO();
    } else {
        bindDataToGPU();
        glDrawArrays( GL_TRIANGLES, 0, GLsizei(VERTICES.size()) );
        glDisableVertexAttribArray(0);
    }
}
void Skybox::onEvent(const Event& e) {
    if (e.type == EventType::WindowFullscreenChanged) {
        buildVAO();
    }
}