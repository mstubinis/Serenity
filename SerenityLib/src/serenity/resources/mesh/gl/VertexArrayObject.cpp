#include <serenity/resources/mesh/gl/VertexArrayObject.h>
#include <utility>

VertexArrayObject::VertexArrayObject(VertexArrayObject&& other) noexcept 
	: m_VAOHandle{ std::exchange(other.m_VAOHandle, 0) }
{}
VertexArrayObject& VertexArrayObject::operator=(VertexArrayObject&& other) noexcept {
	if (this != &other) {
		m_VAOHandle = std::exchange(other.m_VAOHandle, 0);
	}
	return *this;
}


VertexArrayObject::~VertexArrayObject() {
	deleteVAO();
}
bool VertexArrayObject::deleteVAO() {
	if (m_VAOHandle != 0) {
		glDeleteVertexArrays(1, &m_VAOHandle);
		m_VAOHandle = 0;
		return true;
	}
	return false;
}
bool VertexArrayObject::generateVAO() {
	if (m_VAOHandle == 0) {
		glGenVertexArrays(1, &m_VAOHandle);
		return true;
	}
	return false;
}
void VertexArrayObject::bindVAO() const noexcept {
	//TODO: cache the already bound vao?
    glBindVertexArray(m_VAOHandle);
}
void VertexArrayObject::unbindVAO() const noexcept {
	//TODO: cache the already bound vao?
	glBindVertexArray(0);
}