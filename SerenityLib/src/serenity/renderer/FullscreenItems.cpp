
#include <serenity/renderer/FullscreenItems.h>
#include <serenity/events/Event.h>
#include <serenity/resources/Engine_Resources.h>

#pragma region Triangle

void Engine::priv::FullscreenTriangle::init() {
    MeshVertexDataFullscreen v1, v2, v3;
    auto winSize = glm::vec2(Engine::Resources::getWindowSize());

    m_Buffers.emplace_back(0);
    glGenBuffers(1, &m_Buffers[0]);

    m_Buffers.emplace_back(0);
    glGenBuffers(1, &m_Buffers[1]);

    m_Indices.emplace_back(0);
    m_Indices.emplace_back(1);
    m_Indices.emplace_back(2);

    changeDimensions(winSize.x, winSize.y);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(MeshVertexDataFullscreen), &m_Vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned short), &m_Indices[0], GL_STATIC_DRAW);

    buildVAO();
    registerEvent(EventType::WindowFullscreenChanged);
}
Engine::priv::FullscreenTriangle::~FullscreenTriangle(){
    for (size_t i = 0; i < m_Buffers.size(); ++i) {
        glDeleteBuffers(1, &m_Buffers[i]);
    }
    Engine::Renderer::deleteVAO(m_VAO);
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

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(MeshVertexDataFullscreen), &m_Vertices[0], GL_STATIC_DRAW);
}
void Engine::priv::FullscreenTriangle::buildVAO() {
    Engine::Renderer::deleteVAO(m_VAO);
    if (Engine::priv::OpenGLState::constants.supportsVAO()) {
        Engine::Renderer::genAndBindVAO(m_VAO);
        bindToGPU();
        Engine::Renderer::bindVAO(0);
    }
}
void Engine::priv::FullscreenTriangle::bindToGPU() {
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[0]);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataFullscreen), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataFullscreen), (void*)offsetof(MeshVertexDataFullscreen, uv));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[1]);
}
void Engine::priv::FullscreenTriangle::render(){
    if (m_VAO) {
        Engine::Renderer::bindVAO(m_VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)m_Indices.size(), GL_UNSIGNED_SHORT, 0);
    }else{
        bindToGPU();
        glDrawElements(GL_TRIANGLES, (GLsizei)m_Indices.size(), GL_UNSIGNED_SHORT, 0);
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
    auto winSize = glm::vec2(Engine::Resources::getWindowSize());

    m_Buffers.emplace_back(0);
    glGenBuffers(1, &m_Buffers[0]);
    m_Buffers.emplace_back(0);
    glGenBuffers(1, &m_Buffers[1]);

    m_Indices.emplace_back(0);
    m_Indices.emplace_back(1);
    m_Indices.emplace_back(2);

    m_Indices.emplace_back(0);
    m_Indices.emplace_back(2);
    m_Indices.emplace_back(3);

    changeDimensions(winSize.x, winSize.y);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned short), &m_Indices[0], GL_STATIC_DRAW);

    buildVAO();
    registerEvent(EventType::WindowFullscreenChanged);
}
Engine::priv::FullscreenQuad::~FullscreenQuad(){
    for (size_t i = 0; i < m_Buffers.size(); ++i) {
        glDeleteBuffers(1, &m_Buffers[i]);
    }
    Engine::Renderer::deleteVAO(m_VAO);
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

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(MeshVertexDataFullscreen), &m_Vertices[0], GL_STATIC_DRAW);
}
void Engine::priv::FullscreenQuad::buildVAO() {
    Engine::Renderer::deleteVAO(m_VAO);
    if (Engine::priv::OpenGLState::constants.supportsVAO()) {
        Engine::Renderer::genAndBindVAO(m_VAO);
        bindToGPU();
        Engine::Renderer::bindVAO(0);
    }
}
void Engine::priv::FullscreenQuad::bindToGPU() {
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataFullscreen), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataFullscreen), (void*)offsetof(MeshVertexDataFullscreen, uv));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[1]);
}
void Engine::priv::FullscreenQuad::render(){
    if (m_VAO) {
        Engine::Renderer::bindVAO(m_VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)m_Indices.size(), GL_UNSIGNED_SHORT, 0);
    }else{
        bindToGPU();
        glDrawElements(GL_TRIANGLES, (GLsizei)m_Indices.size(), GL_UNSIGNED_SHORT, 0);
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