
#include <serenity/renderer/opengl/UniformBufferObject.h>
#include <serenity/renderer/Renderer.h>
#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/events/Event.h>

#include <serenity/renderer/opengl/APIStateOpenGL.h>

uint32_t  UniformBufferObject::CUSTOM_UBO_AUTOMATIC_COUNT = 0;

namespace {
    [[nodiscard]] inline bool internal_is_supported() noexcept {
        return Engine::priv::APIState<Engine::priv::OpenGL>::supportsUBO();
    }
}

UniformBufferObject::UniformBufferObject(std::string_view nameInShader, uint32_t sizeofStruct, const int globalBindingPointNumber) 
    : Resource{ ResourceType::UniformBufferObject, nameInShader }
    , m_SizeOfStruct{ sizeofStruct }
{
    if (!internal_is_supported()) {
        return;
    }
    if (globalBindingPointNumber == -1) {
        const auto MAX_UBO_BINDINGS = Engine::priv::APIState<Engine::priv::OpenGL>::getConstants().MAX_UNIFORM_BUFFER_BINDINGS;
        assert(MAX_UBO_BINDINGS != 0);
        m_GlobalBindingPointNumber = (MAX_UBO_BINDINGS - 1) - UniformBufferObject::CUSTOM_UBO_AUTOMATIC_COUNT++;
        if (m_GlobalBindingPointNumber < 0) {
            ENGINE_LOG("Warning: Max UBO Limit reached!")
            m_GlobalBindingPointNumber = 0;
        }
    } else {
        m_GlobalBindingPointNumber = globalBindingPointNumber;
    }
    internal_load_CPU();
    internal_load_GPU();
}
UniformBufferObject::~UniformBufferObject() {
    internal_unload_GPU();
    internal_unload_CPU();
}
void UniformBufferObject::internal_load_CPU() {
    if (!internal_is_supported()) {
        return;
    }
    internal_unload_CPU();
}
void UniformBufferObject::internal_unload_CPU() {
    if (!internal_is_supported()) {
        return;
    }
}
void UniformBufferObject::internal_load_GPU() {
    if (!internal_is_supported()) {
        return;
    }
    internal_unload_GPU();
    glGenBuffers(1, &m_UBO_GLHandle);
    glBindBuffer(GL_UNIFORM_BUFFER, m_UBO_GLHandle);                                 // gen and bind buffer
    glBufferData(GL_UNIFORM_BUFFER, m_SizeOfStruct, NULL, GL_STREAM_DRAW);       // create buffer data storage
    glBindBufferBase(GL_UNIFORM_BUFFER, m_GlobalBindingPointNumber, m_UBO_GLHandle); // link UBO to it's global numerical index
}
void UniformBufferObject::internal_unload_GPU() {
    if (!internal_is_supported()) {
        return;
    }
    glDeleteBuffers(1, &m_UBO_GLHandle);
}
void UniformBufferObject::updateData(void* data) {
    if (!internal_is_supported()) {
        return;
    } 
    glBindBuffer(GL_UNIFORM_BUFFER, m_UBO_GLHandle);
    //orphaning
    glBufferData(GL_UNIFORM_BUFFER, m_SizeOfStruct, NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, m_SizeOfStruct, data);
}
bool UniformBufferObject::attachToShaderProgram(ShaderProgram& shaderProgram) {
    if (!internal_is_supported() || shaderProgram.m_AttachedUBOs.contains(m_UBO_GLHandle)) {
        return false;
    }
    const uint32_t programBlockIndex = glGetUniformBlockIndex(shaderProgram.m_ShaderProgram, name().c_str());
    if (programBlockIndex == GL_INVALID_INDEX) {
        return false; //error: was not found
    }
    glUniformBlockBinding(shaderProgram.m_ShaderProgram, programBlockIndex, m_GlobalBindingPointNumber);
    shaderProgram.m_AttachedUBOs.emplace(m_UBO_GLHandle);
    return true;
}





UniformBufferObjectMapper::UniformBufferObjectMapper(UniformBufferObject& ubo) {
    if (!internal_is_supported()) {
        return;
    }
    glBindBuffer(GL_UNIFORM_BUFFER, ubo.m_UBO_GLHandle);
    m_Ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, ubo.m_SizeOfStruct, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
}
UniformBufferObjectMapper::~UniformBufferObjectMapper() {
    if (m_Ptr) {
        glUnmapBuffer(GL_UNIFORM_BUFFER);
    }
}