#include <core/engine/renderer/opengl/UniformBufferObject.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/events/Event.h>
#include <iostream>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

GLint UniformBufferObject::MAX_UBO_BINDINGS;
unsigned int UniformBufferObject::CUSTOM_UBO_AUTOMATIC_COUNT = 0;
UniformBufferObject* UniformBufferObject::UBO_CAMERA = nullptr;

UniformBufferObject::UniformBufferObject(const char* nameInShader, const unsigned int sizeofStruct, const int globalBindingPointNumber) {
    m_NameInShader = nameInShader;
    if (Engine::priv::Renderer::GLSL_VERSION < 140) {
        return;
    }
    if (globalBindingPointNumber == -1) {
        //automatic assignment
        m_GlobalBindingPointNumber = (UniformBufferObject::MAX_UBO_BINDINGS - 1) - UniformBufferObject::CUSTOM_UBO_AUTOMATIC_COUNT;
        ++UniformBufferObject::CUSTOM_UBO_AUTOMATIC_COUNT;
        if (m_GlobalBindingPointNumber < 0) {
            cout << "Warning: Max UBO Limit reached!" << "\n";
            m_GlobalBindingPointNumber = 0;
        }
    }else{
        m_GlobalBindingPointNumber = globalBindingPointNumber;
    }
    m_SizeOfStruct = sizeofStruct;
    _load_CPU();
    _load_GPU();
    registerEvent(EventType::WindowFullscreenChanged);
}
UniformBufferObject::~UniformBufferObject() {
    unregisterEvent(EventType::WindowFullscreenChanged);
    _unload_GPU();
    _unload_CPU();
}
void UniformBufferObject::_load_CPU() {
    if (Engine::priv::Renderer::GLSL_VERSION < 140) {
        return;
    }
    _unload_CPU();
}
void UniformBufferObject::_unload_CPU() {
    if (Engine::priv::Renderer::GLSL_VERSION < 140) {
        return;
    }
}
void UniformBufferObject::_load_GPU() {
    if (Engine::priv::Renderer::GLSL_VERSION < 140) {
        return;
    }
    _unload_GPU();
    glGenBuffers(1, &m_UBOObject);
    glBindBuffer(GL_UNIFORM_BUFFER, m_UBOObject);//gen and bind buffer
    glBufferData(GL_UNIFORM_BUFFER, m_SizeOfStruct, NULL, GL_DYNAMIC_DRAW); //create buffer data storage
    glBindBufferBase(GL_UNIFORM_BUFFER, m_GlobalBindingPointNumber, m_UBOObject);//link UBO to it's global numerical index
}
void UniformBufferObject::_unload_GPU() {
    if (Engine::priv::Renderer::GLSL_VERSION < 140) {
        return;
    }
    glDeleteBuffers(1, &m_UBOObject);
}
void UniformBufferObject::updateData(void* data) {
    if (Engine::priv::Renderer::GLSL_VERSION < 140) {
        return;
    } 
    glBindBuffer(GL_UNIFORM_BUFFER, m_UBOObject);
    //orphaning
    glBufferData(GL_UNIFORM_BUFFER, m_SizeOfStruct, NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, m_SizeOfStruct, data);
}
void UniformBufferObject::attachToShader(const ShaderProgram& shaderProgram) {
    if (Engine::priv::Renderer::GLSL_VERSION < 140 || shaderProgram.m_AttachedUBOs.count(m_UBOObject)) {
        return;
    }
    const unsigned int programBlockIndex = glGetUniformBlockIndex(shaderProgram.m_ShaderProgram, m_NameInShader);
    glUniformBlockBinding(shaderProgram.m_ShaderProgram, programBlockIndex, m_GlobalBindingPointNumber);
    const_cast<ShaderProgram&>(shaderProgram).m_AttachedUBOs.emplace(m_UBOObject);
}
const GLuint UniformBufferObject::address() const {
    return m_UBOObject;
}
void UniformBufferObject::onEvent(const Event& e) {
    if (e.type == EventType::WindowFullscreenChanged) {
        _load_GPU();
    }
}