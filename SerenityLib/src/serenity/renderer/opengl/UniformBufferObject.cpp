
#include <serenity/renderer/opengl/UniformBufferObject.h>
#include <serenity/renderer/Renderer.h>
#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/events/Event.h>
#include <serenity/renderer/opengl/OpenGL.h>

using namespace Engine;
using namespace Engine::priv;

GLint     UniformBufferObject::MAX_UBO_BINDINGS;
uint32_t  UniformBufferObject::CUSTOM_UBO_AUTOMATIC_COUNT = 0;

UniformBufferObject::UniformBufferObject(std::string_view nameInShader, uint32_t sizeofStruct, const int globalBindingPointNumber) 
    : Resource{ ResourceType::UniformBufferObject, nameInShader }
    , m_SizeOfStruct{ sizeofStruct }
{
    if (!Engine::priv::OpenGLState::constants.supportsUBO()) {
        return;
    }
    if (globalBindingPointNumber == -1) {
        m_GlobalBindingPointNumber = (UniformBufferObject::MAX_UBO_BINDINGS - 1) - UniformBufferObject::CUSTOM_UBO_AUTOMATIC_COUNT++;
        if (m_GlobalBindingPointNumber < 0) {
            ENGINE_PRODUCTION_LOG("Warning: Max UBO Limit reached!")
            m_GlobalBindingPointNumber = 0;
        }
    }else{
        m_GlobalBindingPointNumber = globalBindingPointNumber;
    }
    internal_load_CPU();
    internal_load_GPU();
    registerEvent(EventType::WindowFullscreenChanged);
}
UniformBufferObject::~UniformBufferObject() {
    unregisterEvent(EventType::WindowFullscreenChanged);
    internal_unload_GPU();
    internal_unload_CPU();
}
void UniformBufferObject::internal_load_CPU() {
    if (!Engine::priv::OpenGLState::constants.supportsUBO()) {
        return;
    }
    internal_unload_CPU();
}
void UniformBufferObject::internal_unload_CPU() {
    if (!Engine::priv::OpenGLState::constants.supportsUBO()) {
        return;
    }
}
void UniformBufferObject::internal_load_GPU() {
    if (!Engine::priv::OpenGLState::constants.supportsUBO()) {
        return;
    }
    internal_unload_GPU();
    glGenBuffers(1, &m_UBOObject);
    glBindBuffer(GL_UNIFORM_BUFFER, m_UBOObject);                                 // gen and bind buffer
    glBufferData(GL_UNIFORM_BUFFER, m_SizeOfStruct, NULL, GL_STREAM_DRAW);       // create buffer data storage
    glBindBufferBase(GL_UNIFORM_BUFFER, m_GlobalBindingPointNumber, m_UBOObject); // link UBO to it's global numerical index
}
void UniformBufferObject::internal_unload_GPU() {
    if (!Engine::priv::OpenGLState::constants.supportsUBO()) {
        return;
    }
    glDeleteBuffers(1, &m_UBOObject);
}
void UniformBufferObject::updateData(void* data) {
    if (!Engine::priv::OpenGLState::constants.supportsUBO()) {
        return;
    } 
    glBindBuffer(GL_UNIFORM_BUFFER, m_UBOObject);
    //orphaning
    glBufferData(GL_UNIFORM_BUFFER, m_SizeOfStruct, NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, m_SizeOfStruct, data);
}
bool UniformBufferObject::attachToShaderProgram(ShaderProgram& shaderProgram) {
    if (!Engine::priv::OpenGLState::constants.supportsUBO() || shaderProgram.m_AttachedUBOs.contains(m_UBOObject)) {
        return false;
    }
    const uint32_t programBlockIndex = glGetUniformBlockIndex(shaderProgram.m_ShaderProgram, name().c_str());
    if (programBlockIndex == GL_INVALID_INDEX) {
        return false; //error: was not found
    }
    glUniformBlockBinding(shaderProgram.m_ShaderProgram, programBlockIndex, m_GlobalBindingPointNumber);
    shaderProgram.m_AttachedUBOs.emplace(m_UBOObject);
    return true;
}
void UniformBufferObject::onEvent(const Event& e) {
    if (e.type == EventType::WindowFullscreenChanged) {
        internal_load_GPU();
    }
}




UniformBufferObjectMapper::UniformBufferObjectMapper(UniformBufferObject& ubo) {
    if (!Engine::priv::OpenGLState::constants.supportsUBO()) {
        return;
    }
    glBindBuffer(GL_UNIFORM_BUFFER, ubo.m_UBOObject);
    m_Ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, ubo.m_SizeOfStruct, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
}
UniformBufferObjectMapper::~UniformBufferObjectMapper() {
    if (m_Ptr) {
        glUnmapBuffer(GL_UNIFORM_BUFFER);
    }
}