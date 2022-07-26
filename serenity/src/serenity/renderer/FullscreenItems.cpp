
#include <serenity/renderer/FullscreenItems.h>
#include <serenity/events/Event.h>
#include <serenity/system/Engine.h>

#pragma region Triangle

void Engine::priv::FullscreenTriangle::init() {
    const auto winSize = glm::vec2{ Engine::getWindowSize() };

    m_VBO.generate();
    m_IBO.generate();

    m_Indices.emplace_back(0);
    m_Indices.emplace_back(1);
    m_Indices.emplace_back(2);

    changeDimensions(winSize.x, winSize.y);

    m_IBO.bind();
    m_IBO.setData(m_Indices.size() * sizeof(uint16_t), m_Indices.data(), BufferDataDrawType::Static);

    buildVAO();
    registerEvent(EventType::WindowFullscreenChanged);
}
void Engine::priv::FullscreenTriangle::changeDimensions(float width, float height) {
    if (m_Indices.size() == 0) {
        return;
    }
    m_Vertices.clear();
    MeshVertexDataFullscreen v1, v2, v3;

    //v1.position = glm::vec3{ -width / 2.0f,                   -height / 2.0f,                    0.0f };
    //v2.position = glm::vec3{ (2.0f * width) - (width / 2.0f), -height / 2.0f,                    0.0f };
    //v3.position = glm::vec3{ -width / 2.0f,                   (2.0f * height) - (height / 2.0f), 0.0f };
    v1.position = glm::vec3{ 0.0f,            0.0f,            0.0f };
    v2.position = glm::vec3{(2.0f * width),   0.0f,            0.0f };
    v3.position = glm::vec3{ 0.0f,           (2.0f * height),  0.0f };

    v1.uv = glm::vec2{ 0.0f, 0.0f };
    v2.uv = glm::vec2{ 2.0f, 0.0f };
    v3.uv = glm::vec2{ 0.0f, 2.0f };

    m_Vertices.emplace_back(v1);
    m_Vertices.emplace_back(v2);
    m_Vertices.emplace_back(v3);

    m_VBO.bind();
    m_VBO.setData(m_Vertices.size() * sizeof(MeshVertexDataFullscreen), m_Vertices.data(), BufferDataDrawType::Static);
}
void Engine::priv::FullscreenTriangle::buildVAO() {
    m_VAO.deleteVAO();
    if (Engine::priv::OpenGLState::constants.supportsVAO()) {
        m_VAO.generateVAO();
        m_VAO.bindVAO();
        bindToGPU();
        m_VAO.unbindVAO();
    }
}
void Engine::priv::FullscreenTriangle::bindToGPU() {
    m_VBO.bind();

    m_VBO.specifyVertexAttribute(0, 3, GL_FLOAT, false, sizeof(MeshVertexDataFullscreen), 0);
    m_VBO.specifyVertexAttribute(1, 2, GL_FLOAT, false, sizeof(MeshVertexDataFullscreen), offsetof(MeshVertexDataFullscreen, uv));

    m_IBO.bind();
}
void Engine::priv::FullscreenTriangle::render(){
    if (m_VAO) {
        m_VAO.bindVAO();
        glDrawElements(GL_TRIANGLES, GLsizei(m_Indices.size()), GL_UNSIGNED_SHORT, 0);
        m_VAO.unbindVAO();
    } else {
        bindToGPU();
        glDrawElements(GL_TRIANGLES, GLsizei(m_Indices.size()), GL_UNSIGNED_SHORT, 0);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }
}
void Engine::priv::FullscreenTriangle::onEvent(const Event& e) {
    if (e.type == EventType::WindowFullscreenChanged) {
        buildVAO();
    }
}

#pragma endregion


#pragma region Quad

void Engine::priv::FullscreenQuad::init() {
    auto winSize = glm::vec2{ Engine::getWindowSize() };

    m_VBO.generate();
    m_IBO.generate();

    m_Indices.emplace_back(0);
    m_Indices.emplace_back(1);
    m_Indices.emplace_back(2);

    m_Indices.emplace_back(0);
    m_Indices.emplace_back(2);
    m_Indices.emplace_back(3);

    changeDimensions(winSize.x, winSize.y);

    m_IBO.bind();
    m_IBO.setData(m_Indices.size() * sizeof(uint16_t), m_Indices.data(), BufferDataDrawType::Static);

    buildVAO();
    registerEvent(EventType::WindowFullscreenChanged);
}
void Engine::priv::FullscreenQuad::changeDimensions(float width, float height) {
    if (m_Indices.size() == 0) {
        return;
    }
    m_Vertices.clear();
    MeshVertexDataFullscreen v1, v2, v3, v4;

    //v1.position = glm::vec3{ -width / 2.0f,  -height / 2.0f,  0.0f };
    //v2.position = glm::vec3{ width / 2.0f,   -height / 2.0f,  0.0f };
    //v3.position = glm::vec3{ width / 2.0f,   height / 2.0f,   0.0f };
    //v4.position = glm::vec3{ -width / 2.0f,  height / 2.0f,   0.0f };
    v1.position = glm::vec3{ 0.0f,   0.0f,     0.0f };
    v2.position = glm::vec3{ width,  0.0f,     0.0f };
    v3.position = glm::vec3{ width,  height,   0.0f };
    v4.position = glm::vec3{ 0.0f,   height,   0.0f };

    v1.uv = glm::vec2{ 0.0f, 0.0f };
    v2.uv = glm::vec2{ 1.0f, 0.0f };
    v3.uv = glm::vec2{ 1.0f, 1.0f };
    v4.uv = glm::vec2{ 0.0f, 1.0f };
    
    m_Vertices.emplace_back(v1);
    m_Vertices.emplace_back(v2);
    m_Vertices.emplace_back(v3);
    m_Vertices.emplace_back(v4);

    m_VBO.bind();
    m_VBO.setData(m_Vertices.size() * sizeof(MeshVertexDataFullscreen), m_Vertices.data(), BufferDataDrawType::Static);
}
void Engine::priv::FullscreenQuad::buildVAO() {
    m_VAO.deleteVAO();
    if (Engine::priv::OpenGLState::constants.supportsVAO()) {
        m_VAO.generateVAO();
        m_VAO.bindVAO();
        bindToGPU();
        m_VAO.unbindVAO();
    }
}
void Engine::priv::FullscreenQuad::bindToGPU() {
    m_VBO.bind();
    m_VBO.specifyVertexAttribute(0, 3, GL_FLOAT, false, sizeof(MeshVertexDataFullscreen), 0);
    m_VBO.specifyVertexAttribute(1, 2, GL_FLOAT, false, sizeof(MeshVertexDataFullscreen), offsetof(MeshVertexDataFullscreen, uv));

    m_IBO.bind();
}
void Engine::priv::FullscreenQuad::render(){
    if (m_VAO) {
        m_VAO.bindVAO();
        glDrawElements(GL_TRIANGLES, GLsizei(m_Indices.size()), GL_UNSIGNED_SHORT, 0);
        m_VAO.unbindVAO();
    } else {
        bindToGPU();
        glDrawElements(GL_TRIANGLES, GLsizei(m_Indices.size()), GL_UNSIGNED_SHORT, 0);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }
}
void Engine::priv::FullscreenQuad::onEvent(const Event& e) {
    if (e.type == EventType::WindowFullscreenChanged) {
        buildVAO();
    }
}

#pragma endregion