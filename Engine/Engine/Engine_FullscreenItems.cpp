#include "Engine_FullscreenItems.h"

using namespace Engine;
using namespace std;

epriv::FullscreenTriangle::FullscreenTriangle(){ 
    MeshVertexDataFullscreen v1, v2, v3;
    m_VAO = 0;
    v1.position = glm::vec3(-1.0f, -1.0f, 0.0f); v1.uv = glm::vec2(0.0f, 0.0f);
    v2.position = glm::vec3(4.0f, -1.0f, 0.0f); v2.uv = glm::vec2(2.5f, 0.0f);
    v3.position = glm::vec3(-1.0f, 4.0f, 0.0f); v3.uv = glm::vec2(0.0f, 2.5f);

    m_Vertices.push_back(v1); m_Vertices.push_back(v2); m_Vertices.push_back(v3);

    m_Indices.push_back(0); m_Indices.push_back(1); m_Indices.push_back(2);

    m_Buffers.push_back(0);
    glGenBuffers(1, &m_Buffers[0]);
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(MeshVertexDataFullscreen), &m_Vertices[0], GL_STATIC_DRAW);

    m_Buffers.push_back(0);
    glGenBuffers(1, &m_Buffers[1]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(ushort), &m_Indices[0], GL_STATIC_DRAW);

    //vao's
    buildVAO();
    registerEvent(EventType::WindowFullscreenChanged);
}
epriv::FullscreenTriangle::~FullscreenTriangle(){ 
    for (uint i = 0; i < m_Buffers.size(); ++i)
        glDeleteBuffers(1, &m_Buffers[i]);
    Renderer::deleteVAO(m_VAO);
}
void epriv::FullscreenTriangle::buildVAO() {
    Renderer::deleteVAO(m_VAO);
    if (RenderManager::OPENGL_VERSION >= 30) {
        Renderer::genAndBindVAO(m_VAO);
        bindToGPU();
        Renderer::bindVAO(0);
    }
}
void epriv::FullscreenTriangle::bindToGPU() {
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataFullscreen), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataFullscreen), (void*)offsetof(MeshVertexDataFullscreen, uv));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[1]);
}
void epriv::FullscreenTriangle::render(){ 
    if (m_VAO) {
        Renderer::bindVAO(m_VAO);
        glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_SHORT, 0);
        //Renderer::bindVAO(0);
    }else{
        bindToGPU();
        glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_SHORT, 0);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }
}
void epriv::FullscreenTriangle::onEvent(const Event& e) {
    if (e.type == EventType::WindowFullscreenChanged) {
        buildVAO();
    }
}








epriv::FullscreenQuad::FullscreenQuad(){ 
    MeshVertexDataFullscreen v1, v2, v3, v4;
    m_VAO = 0;
    v1.position = glm::vec3(-1.0f, -1.0f, 0.0f); v1.uv = glm::vec2(0.0f, 0.0f);
    v2.position = glm::vec3(1.0f, -1.0f, 0.0f); v2.uv = glm::vec2(1.0f, 0.0f);
    v3.position = glm::vec3(1.0f, 1.0f, 0.0f); v3.uv = glm::vec2(1.0f, 1.0f);
    v4.position = glm::vec3(-1.0f, 1.0f, 0.0f); v4.uv = glm::vec2(0.0f, 1.0f);

    m_Vertices.push_back(v1); m_Vertices.push_back(v2); m_Vertices.push_back(v3); m_Vertices.push_back(v4);

    m_Indices.push_back(0); m_Indices.push_back(1); m_Indices.push_back(2); m_Indices.push_back(3);

    m_Buffers.push_back(0);
    glGenBuffers(1, &m_Buffers[0]);
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(MeshVertexDataFullscreen), &m_Vertices[0], GL_STATIC_DRAW);

    m_Buffers.push_back(0);
    glGenBuffers(1, &m_Buffers[1]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(ushort), &m_Indices[0], GL_STATIC_DRAW);

    //vao's
    buildVAO();
    registerEvent(EventType::WindowFullscreenChanged);
}
epriv::FullscreenQuad::~FullscreenQuad(){ 
    for (uint i = 0; i < m_Buffers.size(); ++i)
        glDeleteBuffers(1, &m_Buffers[i]);
    Renderer::deleteVAO(m_VAO);
}
void epriv::FullscreenQuad::buildVAO() {
    Renderer::deleteVAO(m_VAO);
    if (RenderManager::OPENGL_VERSION >= 30) {
        Renderer::genAndBindVAO(m_VAO);
        bindToGPU();
        Renderer::bindVAO(0);
    }
}
void epriv::FullscreenQuad::bindToGPU() {
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataFullscreen), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataFullscreen), (void*)offsetof(MeshVertexDataFullscreen, uv));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[1]);
}
void epriv::FullscreenQuad::render(){ 
    if (m_VAO) {
        Renderer::bindVAO(m_VAO);
        glDrawElements(GL_QUADS, m_Indices.size(), GL_UNSIGNED_SHORT, 0);
        //Renderer::bindVAO(0);
    }else{
        bindToGPU();
        glDrawElements(GL_QUADS, m_Indices.size(), GL_UNSIGNED_SHORT, 0);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }
}
void epriv::FullscreenQuad::onEvent(const Event& e) {
    if (e.type == EventType::WindowFullscreenChanged) {
        buildVAO();
    }
}