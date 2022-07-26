#include <serenity/resources/mesh/gl/VertexArrayObject.h>
#include <serenity/renderer/Renderer.h>
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
	//return Engine::Renderer::deleteVAO(m_VAOHandle);
	if (m_VAOHandle) {
		glDeleteVertexArrays(1, &m_VAOHandle);
		m_VAOHandle = 0;
		return true;
	}
	return false;
}
bool VertexArrayObject::generateVAO() {
	if (m_VAOHandle == 0) {
		//Engine::Renderer::generateVAO(m_VAOHandle);
		glGenVertexArrays(1, &m_VAOHandle);
		return m_VAOHandle >= 1;
	}
	return false;
}
void VertexArrayObject::bindVAO() const noexcept {
	Engine::Renderer::bindVAO(m_VAOHandle);
	//glBindVertexArray(m_VAOHandle);
}
void VertexArrayObject::unbindVAO() const noexcept {
	Engine::Renderer::bindVAO(0);
	//glBindVertexArray(0);
}