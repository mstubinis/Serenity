#pragma once
#ifndef ENGINE_MESH_GL_VERTEX_ARRAY_OBJECT
#define ENGINE_MESH_GL_VERTEX_ARRAY_OBJECT

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

class VertexArrayObject {
	private:
		GLuint m_VAOHandle = 0;
	public:

		VertexArrayObject() = default;
		VertexArrayObject(const VertexArrayObject&) = delete;
		VertexArrayObject& operator=(const VertexArrayObject&) = delete;
		VertexArrayObject(VertexArrayObject&&) noexcept;
		VertexArrayObject& operator=(VertexArrayObject&&) noexcept;

		~VertexArrayObject();
		
		bool deleteVAO();
		bool generateVAO();
		void bindVAO() const noexcept;
		void unbindVAO() const noexcept;

		[[nodiscard]] inline explicit operator bool() const noexcept { return m_VAOHandle != 0; }
};

#endif