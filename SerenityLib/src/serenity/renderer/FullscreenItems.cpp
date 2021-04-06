
#include <serenity/renderer/FullscreenItems.h>
#include <serenity/events/Event.h>
#include <serenity/resources/Engine_Resources.h>

#pragma region Triangle

void Engine::priv::FullscreenTriangle::init() {
    MeshVertexDataFullscreen v1, v2, v3;
    auto winSize = glm::vec2(Engine::Resources::getWindowSize());

    m_Buffers.emplace_back(0);
    GLCall(glGenBuffers(1, &m_Buffers[0]));

    m_Buffers.emplace_back(0);
    GLCall(glGenBuffers(1, &m_Buffers[1]));

    m_Indices.emplace_back(0);
    m_Indices.emplace_back(1);
    m_Indices.emplace_back(2);

    changeDimensions(winSize.x, winSize.y);

    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[0]));
    GLCall(glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(MeshVertexDataFullscreen), &m_Vertices[0], GL_STATIC_DRAW));

    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[1]));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned short), &m_Indices[0], GL_STATIC_DRAW));

    //vao's
    buildVAO();
    registerEvent(EventType::WindowFullscreenChanged);
}
Engine::priv::FullscreenTriangle::~FullscreenTriangle(){
    for (size_t i = 0; i < m_Buffers.size(); ++i) {
        GLCall(glDeleteBuffers(1, &m_Buffers[i]));
    }
    Engine::Renderer::deleteVAO(m_VAO);
}
void Engine::priv::FullscreenTriangle::changeDimensions(float width, float height) {
    if (m_Indices.size() == 0) {
        return;
    }
    m_Vertices.clear();
    MeshVertexDataFullscreen v1, v2, v3;

    v1.position = glm::vec3(0, 0, 0.0f);
    v2.position = glm::vec3(2.0f * width, 0, 0.0f);
    v3.position = glm::vec3(0, 2.0f * height, 0.0f);

    v1.uv = glm::vec2(0,0);
    v2.uv = glm::vec2(2.0f ,0);
    v3.uv = glm::vec2(0, 2.0f );

    m_Vertices.emplace_back(v1);
    m_Vertices.emplace_back(v2);
    m_Vertices.emplace_back(v3);

    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[0]));
    GLCall(glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(MeshVertexDataFullscreen), &m_Vertices[0], GL_STATIC_DRAW));
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
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[0]));

    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataFullscreen), (void*)0));

    GLCall(glEnableVertexAttribArray(1));
    GLCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataFullscreen), (void*)offsetof(MeshVertexDataFullscreen, uv)));

    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[1]));
}
void Engine::priv::FullscreenTriangle::render(){
    if (m_VAO) {
        Engine::Renderer::bindVAO(m_VAO);
        GLCall(glDrawElements(GL_TRIANGLES, (GLsizei)m_Indices.size(), GL_UNSIGNED_SHORT, 0));
    }else{
        bindToGPU();
        GLCall(glDrawElements(GL_TRIANGLES, (GLsizei)m_Indices.size(), GL_UNSIGNED_SHORT, 0));
        GLCall(glDisableVertexAttribArray(0));
        GLCall(glDisableVertexAttribArray(1));
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
    GLCall(glGenBuffers(1, &m_Buffers[0]));
    m_Buffers.emplace_back(0);
    GLCall(glGenBuffers(1, &m_Buffers[1]));

    m_Indices.emplace_back(0);
    m_Indices.emplace_back(1);
    m_Indices.emplace_back(2);

    m_Indices.emplace_back(0);
    m_Indices.emplace_back(2);
    m_Indices.emplace_back(3);

    changeDimensions(winSize.x, winSize.y);

    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[1]));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned short), &m_Indices[0], GL_STATIC_DRAW));

    //vao's
    buildVAO();
    registerEvent(EventType::WindowFullscreenChanged);
}
Engine::priv::FullscreenQuad::~FullscreenQuad(){
    for (size_t i = 0; i < m_Buffers.size(); ++i) {
        GLCall(glDeleteBuffers(1, &m_Buffers[i]));
    }
    Engine::Renderer::deleteVAO(m_VAO);
}
void Engine::priv::FullscreenQuad::changeDimensions(float width, float height) {
    if (m_Indices.size() == 0) {
        return;
    }
    m_Vertices.clear();
    MeshVertexDataFullscreen v1, v2, v3, v4;

    v1.position = glm::vec3(0, 0, 0.0f);
    v2.position = glm::vec3(width, 0, 0.0f);
    v3.position = glm::vec3(width, height, 0.0f);
    v4.position = glm::vec3(0, height, 0.0f);
    
    v1.uv = glm::vec2(0.0f, 0.0f);
    v2.uv = glm::vec2(1.0f, 0.0f);
    v3.uv = glm::vec2(1.0f, 1.0f);
    v4.uv = glm::vec2(0.0f, 1.0f);
    
    m_Vertices.emplace_back(v1);
    m_Vertices.emplace_back(v2);
    m_Vertices.emplace_back(v3);
    m_Vertices.emplace_back(v4);

    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[0]));
    GLCall(glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(MeshVertexDataFullscreen), &m_Vertices[0], GL_STATIC_DRAW));
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
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[0]));
    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataFullscreen), (void*)0));
    GLCall(glEnableVertexAttribArray(1));
    GLCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataFullscreen), (void*)offsetof(MeshVertexDataFullscreen, uv)));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[1]));
}
void Engine::priv::FullscreenQuad::render(){
    if (m_VAO) {
        Engine::Renderer::bindVAO(m_VAO);
        GLCall(glDrawElements(GL_TRIANGLES, (GLsizei)m_Indices.size(), GL_UNSIGNED_SHORT, 0));
    }else{
        bindToGPU();
        GLCall(glDrawElements(GL_TRIANGLES, (GLsizei)m_Indices.size(), GL_UNSIGNED_SHORT, 0));
        GLCall(glDisableVertexAttribArray(0));
        GLCall(glDisableVertexAttribArray(1));
    }
}
void Engine::priv::FullscreenQuad::onEvent(const Event& e) {
    if (e.type == EventType::WindowFullscreenChanged) {
        buildVAO();
    }
}

#pragma endregion