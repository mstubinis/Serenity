#include "Engine_FullscreenItems.h"
#include <vector>
#include <glm/glm.hpp>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

using namespace Engine;
using namespace std;

typedef unsigned int uint;
typedef unsigned short ushort;

namespace Engine{
	namespace epriv{
		struct MeshVertexDataFullscreen final{
			public:
				glm::vec3 position;
				glm::vec2 uv;

				MeshVertexDataFullscreen(){}
				~MeshVertexDataFullscreen(){}
		};
	};
};
class epriv::FullscreenTriangle::impl final{
    public:
		vector<ushort> m_Indices;
		vector<MeshVertexDataFullscreen> m_Vertices;
		vector<GLuint> m_Buffers;
		void _init(){
			MeshVertexDataFullscreen vertex1,vertex2,vertex3;

			vertex1.position = glm::vec3(-1.0f,-1.0f,0.0f); vertex1.uv = glm::vec2(0.0f,0.0f);
			vertex2.position = glm::vec3(4.0f, -1.0f,0.0f); vertex2.uv = glm::vec2(2.5f,0.0f);
			vertex3.position = glm::vec3(-1.0f, 4.0f,0.0f); vertex3.uv = glm::vec2(0.0f,2.5f);

			m_Vertices.push_back(vertex1);m_Vertices.push_back(vertex2);m_Vertices.push_back(vertex3);

			m_Indices.push_back(0);m_Indices.push_back(1);m_Indices.push_back(2);

            m_Buffers.push_back(0);
            glGenBuffers(1, &m_Buffers.at(0));
            glBindBuffer(GL_ARRAY_BUFFER, m_Buffers.at(0));
			glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(MeshVertexDataFullscreen),&m_Vertices[0], GL_STATIC_DRAW );

            m_Buffers.push_back(0);
            glGenBuffers(1, &m_Buffers.at(1));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers.at(1));
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(ushort), &m_Indices[0] , GL_STATIC_DRAW);
		}
		void _destruct(){
			glDeleteBuffers(1,&m_Buffers.at(0));
			glDeleteBuffers(1,&m_Buffers.at(1));
		}
		void _render(){
			glBindBuffer(GL_ARRAY_BUFFER, m_Buffers.at(0));
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(MeshVertexDataFullscreen),(void*)0);

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(MeshVertexDataFullscreen),(void*)offsetof(MeshVertexDataFullscreen,uv));

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers.at(1));
			glDrawElements(GL_TRIANGLES,m_Indices.size(),GL_UNSIGNED_SHORT,0);
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
		}
};
class epriv::FullscreenQuad::impl final{
    public:
		vector<ushort> m_Indices;
		vector<MeshVertexDataFullscreen> m_Vertices;
		vector<GLuint> m_Buffers;
		void _init(){
			MeshVertexDataFullscreen vertex1,vertex2,vertex3,vertex4;

			vertex1.position = glm::vec3(-1.0f,-1.0f, 0.0f); vertex1.uv = glm::vec2(0.0f,0.0f);
			vertex2.position = glm::vec3( 1.0f,-1.0f, 0.0f); vertex2.uv = glm::vec2(1.0f,0.0f);
			vertex3.position = glm::vec3( 1.0f, 1.0f, 0.0f); vertex3.uv = glm::vec2(1.0f,1.0f);
			vertex4.position = glm::vec3(-1.0f, 1.0f, 0.0f); vertex4.uv = glm::vec2(0.0f,1.0f);

			m_Vertices.push_back(vertex1);m_Vertices.push_back(vertex2);m_Vertices.push_back(vertex3);m_Vertices.push_back(vertex4);

			m_Indices.push_back(0);m_Indices.push_back(1);m_Indices.push_back(2);m_Indices.push_back(3);

            m_Buffers.push_back(0);
            glGenBuffers(1, &m_Buffers.at(0));
            glBindBuffer(GL_ARRAY_BUFFER, m_Buffers.at(0));
			glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(MeshVertexDataFullscreen),&m_Vertices[0], GL_STATIC_DRAW );

            m_Buffers.push_back(0);
            glGenBuffers(1, &m_Buffers.at(1));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers.at(1));
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(ushort), &m_Indices[0] , GL_STATIC_DRAW);
		}
		void _destruct(){
			glDeleteBuffers(1,&m_Buffers.at(0));
			glDeleteBuffers(1,&m_Buffers.at(1));
		}
		void _render(){
			glBindBuffer(GL_ARRAY_BUFFER, m_Buffers.at(0));
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(MeshVertexDataFullscreen),(void*)0);

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(MeshVertexDataFullscreen),(void*)offsetof(MeshVertexDataFullscreen,uv));

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers.at(1));
			glDrawElements(GL_QUADS,m_Indices.size(),GL_UNSIGNED_SHORT,0);
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
		}
};

epriv::FullscreenTriangle::FullscreenTriangle():m_i(new impl){
	m_i->_init();
}
epriv::FullscreenTriangle::~FullscreenTriangle(){
	m_i->_destruct();
}
void epriv::FullscreenTriangle::render(){
	m_i->_render();
}
epriv::FullscreenQuad::FullscreenQuad():m_i(new impl){
	m_i->_init();
}
epriv::FullscreenQuad::~FullscreenQuad(){
	m_i->_destruct();
}
void epriv::FullscreenQuad::render(){
	m_i->_render();
}