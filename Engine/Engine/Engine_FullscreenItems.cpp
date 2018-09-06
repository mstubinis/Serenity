#include "Engine_FullscreenItems.h"
#include "Engine_Renderer.h"
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
		GLuint m_VAO;
        void _init(){
            MeshVertexDataFullscreen v1,v2,v3;
			m_VAO = 0;
            v1.position = glm::vec3(-1.0f,-1.0f,0.0f); v1.uv = glm::vec2(0.0f,0.0f);
            v2.position = glm::vec3(4.0f, -1.0f,0.0f); v2.uv = glm::vec2(2.5f,0.0f);
            v3.position = glm::vec3(-1.0f, 4.0f,0.0f); v3.uv = glm::vec2(0.0f,2.5f);

            m_Vertices.push_back(v1);m_Vertices.push_back(v2);m_Vertices.push_back(v3);

            m_Indices.push_back(0);m_Indices.push_back(1);m_Indices.push_back(2);

            m_Buffers.push_back(0);
            glGenBuffers(1, &m_Buffers.at(0));
            glBindBuffer(GL_ARRAY_BUFFER, m_Buffers.at(0));
            glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(MeshVertexDataFullscreen),&m_Vertices[0], GL_STATIC_DRAW );

            m_Buffers.push_back(0);
            glGenBuffers(1, &m_Buffers.at(1));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers.at(1));
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(ushort), &m_Indices[0] , GL_STATIC_DRAW);

			//vao's
			_buildVAO();
        }
		void _buildVAO() {
			Renderer::deleteVAO(m_VAO);
			if (RenderManager::OPENGL_VERSION >= 30) {
				Renderer::genAndBindVAO(m_VAO);
				_bindDataToGPU();
				Renderer::bindVAO(0);
			}
		}
        void _destruct(){
            for(uint i = 0; i < m_Buffers.size(); ++i)
                glDeleteBuffers(1,&m_Buffers.at(i));
			Renderer::deleteVAO(m_VAO);
        }
		void _bindDataToGPU() {
			glBindBuffer(GL_ARRAY_BUFFER, m_Buffers.at(0));
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataFullscreen), (void*)0);

			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataFullscreen), (void*)offsetof(MeshVertexDataFullscreen, uv));

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers.at(1));
		}
        void _render(){
			if(m_VAO){
				Renderer::bindVAO(m_VAO);
				glDrawElements(GL_TRIANGLES,m_Indices.size(),GL_UNSIGNED_SHORT,0);
				//Renderer::bindVAO(0);
			}else{
				_bindDataToGPU();
				glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_SHORT, 0);
				glDisableVertexAttribArray(0);
				glDisableVertexAttribArray(1);
			}
        }
};
class epriv::FullscreenQuad::impl final{
    public:
        vector<ushort> m_Indices;
        vector<MeshVertexDataFullscreen> m_Vertices;
        vector<GLuint> m_Buffers;
		GLuint m_VAO;
        void _init(){
            MeshVertexDataFullscreen v1,v2,v3,v4;
			m_VAO = 0;
            v1.position = glm::vec3(-1.0f,-1.0f, 0.0f); v1.uv = glm::vec2(0.0f,0.0f);
            v2.position = glm::vec3( 1.0f,-1.0f, 0.0f); v2.uv = glm::vec2(1.0f,0.0f);
            v3.position = glm::vec3( 1.0f, 1.0f, 0.0f); v3.uv = glm::vec2(1.0f,1.0f);
            v4.position = glm::vec3(-1.0f, 1.0f, 0.0f); v4.uv = glm::vec2(0.0f,1.0f);

            m_Vertices.push_back(v1);m_Vertices.push_back(v2);m_Vertices.push_back(v3);m_Vertices.push_back(v4);

            m_Indices.push_back(0);m_Indices.push_back(1);m_Indices.push_back(2);m_Indices.push_back(3);

            m_Buffers.push_back(0);
            glGenBuffers(1, &m_Buffers.at(0));
            glBindBuffer(GL_ARRAY_BUFFER, m_Buffers.at(0));
            glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(MeshVertexDataFullscreen),&m_Vertices[0], GL_STATIC_DRAW );

            m_Buffers.push_back(0);
            glGenBuffers(1, &m_Buffers.at(1));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers.at(1));
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(ushort), &m_Indices[0] , GL_STATIC_DRAW);

			//vao's
			_buildVAO();
        }
		void _buildVAO() {
			Renderer::deleteVAO(m_VAO);
			if (RenderManager::OPENGL_VERSION >= 30) {
				Renderer::genAndBindVAO(m_VAO);
				_bindDataToGPU();
				Renderer::bindVAO(0);
			}
		}
        void _destruct(){
            for(uint i = 0; i < m_Buffers.size(); ++i)
                glDeleteBuffers(1,&m_Buffers.at(i));
			Renderer::deleteVAO(m_VAO);
        }
		void _bindDataToGPU() {
			glBindBuffer(GL_ARRAY_BUFFER, m_Buffers.at(0));
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataFullscreen), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertexDataFullscreen), (void*)offsetof(MeshVertexDataFullscreen, uv));
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers.at(1));
		}
        void _render(){
			if(m_VAO){
				Renderer::bindVAO(m_VAO);
				glDrawElements(GL_TRIANGLES,m_Indices.size(),GL_UNSIGNED_SHORT,0);
				//Renderer::bindVAO(0);
			}else{
				_bindDataToGPU();
				glDrawElements(GL_QUADS,m_Indices.size(),GL_UNSIGNED_SHORT,0);
				glDisableVertexAttribArray(0);
				glDisableVertexAttribArray(1);
			}
        }
};
epriv::FullscreenTriangle::FullscreenTriangle():m_i(new impl){ 
	m_i->_init();
	registerEvent(EventType::WindowFullscreenChanged);
}
epriv::FullscreenTriangle::~FullscreenTriangle(){ 
	unregisterEvent(EventType::WindowFullscreenChanged);
	m_i->_destruct(); 
}
void epriv::FullscreenTriangle::render(){ m_i->_render(); }
void epriv::FullscreenTriangle::onEvent(const Event& e) {
	if (e.type == EventType::WindowFullscreenChanged) {
		auto& i = *m_i;
		i._buildVAO();
	}
}

epriv::FullscreenQuad::FullscreenQuad():m_i(new impl){ 
	m_i->_init(); 
	registerEvent(EventType::WindowFullscreenChanged);
}
epriv::FullscreenQuad::~FullscreenQuad(){ 
	unregisterEvent(EventType::WindowFullscreenChanged);
	m_i->_destruct(); 
}
void epriv::FullscreenQuad::render(){ m_i->_render(); }
void epriv::FullscreenQuad::onEvent(const Event& e) {
	if (e.type == EventType::WindowFullscreenChanged) {
		auto& i = *m_i;
		i._buildVAO();
	}
}