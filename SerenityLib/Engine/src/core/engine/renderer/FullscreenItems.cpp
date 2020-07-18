#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/renderer/FullscreenItems.h>
#include <core/engine/events/Event.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/system/window/Window.h>

using namespace Engine;
using namespace std;

#pragma region Triangle

void priv::FullscreenTriangle::init() {
    MeshVertexDataFullscreen v1, v2, v3;
    const auto winSize = glm::vec2(Resources::getWindowSize());

    m_Buffers.push_back(0);
    glGenBuffers(1, &m_Buffers[0]);

    m_Buffers.push_back(0);
    glGenBuffers(1, &m_Buffers[1]);

    m_Indices.push_back(0);
    m_Indices.push_back(1);
    m_Indices.push_back(2);

    changeDimensions(winSize.x, winSize.y);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(MeshVertexDataFullscreen), &m_Vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(ushort), &m_Indices[0], GL_STATIC_DRAW);

    //vao's
    buildVAO();
    registerEvent(EventType::WindowFullscreenChanged);
}
priv::FullscreenTriangle::~FullscreenTriangle(){ 
    for (uint i = 0; i < m_Buffers.size(); ++i)
        glDeleteBuffers(1, &m_Buffers[i]);
    Engine::Renderer::deleteVAO(m_VAO);
}
void priv::FullscreenTriangle::changeDimensions(const float width, const float height) {
    if (m_Indices.size() == 0)
        return;
    m_Vertices.clear();
    MeshVertexDataFullscreen v1, v2, v3;

    v1.position = glm::vec3(0, 0, 0.0f);
    v2.position = glm::vec3(2.0f * width, 0, 0.0f);
    v3.position = glm::vec3(0, 2.0f * height, 0.0f);

    v1.uv = glm::vec2(0,0);
    v2.uv = glm::vec2(2.0f ,0);
    v3.uv = glm::vec2(0, 2.0f );

    m_Vertices.push_back(v1);
    m_Vertices.push_back(v2);
    m_Vertices.push_back(v3);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(MeshVertexDataFullscreen), &m_Vertices[0], GL_STATIC_DRAW);
}
void priv::FullscreenTriangle::buildVAO() {
    Engine::Renderer::deleteVAO(m_VAO);
    if (Engine::priv::Renderer::OPENGL_VERSION >= 30) {
        Engine::Renderer::genAndBindVAO(m_VAO);
        bindToGPU();
        Engine::Renderer::bindVAO(0);
    }
}
void priv::FullscreenTriangle::bindToGPU() {
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[0]);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataFullscreen), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataFullscreen), (void*)offsetof(MeshVertexDataFullscreen, uv));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[1]);
}
void priv::FullscreenTriangle::render(){ 
    if (m_VAO) {
        Engine::Renderer::bindVAO(m_VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_Indices.size()), GL_UNSIGNED_SHORT, 0);
    }else{
        bindToGPU();
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_Indices.size()), GL_UNSIGNED_SHORT, 0);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }
}
void priv::FullscreenTriangle::onEvent(const Event& e) {
    if (e.type == EventType::WindowFullscreenChanged) {
        buildVAO();
    }
}

#pragma endregion



#pragma region Quad

void priv::FullscreenQuad::init() {
    const auto winSize = glm::vec2(Resources::getWindowSize());

    m_Buffers.push_back(0);
    glGenBuffers(1, &m_Buffers[0]);
    m_Buffers.push_back(0);
    glGenBuffers(1, &m_Buffers[1]);

    m_Indices.push_back(0);
    m_Indices.push_back(1);
    m_Indices.push_back(2);

    m_Indices.push_back(0);
    m_Indices.push_back(2);
    m_Indices.push_back(3);

    changeDimensions(winSize.x, winSize.y);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(ushort), &m_Indices[0], GL_STATIC_DRAW);

    //vao's
    buildVAO();
    registerEvent(EventType::WindowFullscreenChanged);
}
priv::FullscreenQuad::~FullscreenQuad(){ 
    for (uint i = 0; i < m_Buffers.size(); ++i)
        glDeleteBuffers(1, &m_Buffers[i]);
    Engine::Renderer::deleteVAO(m_VAO);
}
void priv::FullscreenQuad::changeDimensions(const float width, const float height) {
    if (m_Indices.size() == 0)
        return;
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
    
    m_Vertices.push_back(v1);
    m_Vertices.push_back(v2);
    m_Vertices.push_back(v3);
    m_Vertices.push_back(v4);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(MeshVertexDataFullscreen), &m_Vertices[0], GL_STATIC_DRAW);
}
void priv::FullscreenQuad::buildVAO() {
    Engine::Renderer::deleteVAO(m_VAO);
    if (Engine::priv::Renderer::OPENGL_VERSION >= 30) {
        Engine::Renderer::genAndBindVAO(m_VAO);
        bindToGPU();
        Engine::Renderer::bindVAO(0);
    }
}
void priv::FullscreenQuad::bindToGPU() {
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataFullscreen), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataFullscreen), (void*)offsetof(MeshVertexDataFullscreen, uv));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[1]);
}
void priv::FullscreenQuad::render(){ 
    if (m_VAO) {
        Engine::Renderer::bindVAO(m_VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_Indices.size()), GL_UNSIGNED_SHORT, 0);
    }else{
        bindToGPU();
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_Indices.size()), GL_UNSIGNED_SHORT, 0);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }
}
void priv::FullscreenQuad::onEvent(const Event& e) {
    if (e.type == EventType::WindowFullscreenChanged) {
        buildVAO();
    }
}

#pragma endregion