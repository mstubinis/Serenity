#include <core/engine/scene/Skybox.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/system/Engine.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/textures/TextureLoader.h>

using namespace Engine;
using namespace std;

GLuint m_Buffer   = 0;
GLuint m_VAO      = 0;
vector<glm::vec3> m_Vertices;

namespace Engine {
    namespace priv {
        class SkyboxImplInterface final {
            friend class ::Skybox;
            static void bindDataToGPU() {
                glBindBuffer(GL_ARRAY_BUFFER, m_Buffer);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            }
            static void buildVAO() {
                Engine::Renderer::deleteVAO(m_VAO);
                if (Engine::priv::Renderer::OPENGL_VERSION >= 30) {
                    Engine::Renderer::genAndBindVAO(m_VAO);
                    Engine::priv::SkyboxImplInterface::bindDataToGPU();
                    Engine::Renderer::bindVAO(0);
                }
            }
            static void initMesh() {
                if (m_Vertices.size() > 0) return;

                vector<glm::vec3> temp;
                temp.emplace_back(-1, 1, 1);//1
                temp.emplace_back(1, 1, 1);//2
                temp.emplace_back(1, -1, 1);//3
                temp.emplace_back(-1, -1, 1);//4
                temp.emplace_back(-1, 1, -1);//5
                temp.emplace_back(-1, -1, -1);//6
                temp.emplace_back(1, -1, -1);//7
                temp.emplace_back(1, 1, -1);//8
                temp.emplace_back(-1, 1, 1);//9
                temp.emplace_back(-1, -1, 1);//10
                temp.emplace_back(-1, -1, -1);//11
                temp.emplace_back(-1, 1, -1);//12
                temp.emplace_back(-1, -1, 1);//13
                temp.emplace_back(1, -1, 1);//14
                temp.emplace_back(1, -1, -1);//15
                temp.emplace_back(-1, -1, -1);//16
                temp.emplace_back(1, -1, 1);//17
                temp.emplace_back(1, 1, 1);//18
                temp.emplace_back(1, 1, -1);//19
                temp.emplace_back(1, -1, -1);//20
                temp.emplace_back(1, 1, 1);//21
                temp.emplace_back(-1, 1, 1);//22
                temp.emplace_back(-1, 1, -1);//23
                temp.emplace_back(1, 1, -1);//24

                for (uint i = 0; i < 6; ++i) {
                    glm::vec3 v1, v2, v3, v4;
                    v1 = temp[0 + (i * 4)];
                    v2 = temp[1 + (i * 4)];
                    v3 = temp[2 + (i * 4)];
                    v4 = temp[3 + (i * 4)];
                    m_Vertices.push_back(v1); m_Vertices.push_back(v2); m_Vertices.push_back(v3);
                    m_Vertices.push_back(v1); m_Vertices.push_back(v3); m_Vertices.push_back(v4);
                }
                glGenBuffers(1, &m_Buffer);
                glBindBuffer(GL_ARRAY_BUFFER, m_Buffer);
                glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(glm::vec3), &m_Vertices[0], GL_STATIC_DRAW);
                Engine::priv::SkyboxImplInterface::buildVAO();
            }
        };
    };
};

Skybox::Skybox(const string* files){
    m_Texture = nullptr;
    Engine::priv::SkyboxImplInterface::initMesh();

    string names[6] = { files[0],files[1],files[2],files[3],files[4],files[5] };
    //instead of using files[0] generate a proper name using the directory?
    m_Texture = NEW Texture(names, files[0] + "Cubemap", false, ImageInternalFormat::SRGB8_ALPHA8);
    priv::TextureLoader::GeneratePBRData(*m_Texture, 32, m_Texture->width() / 4);
    priv::Core::m_Engine->m_ResourceManager._addTexture(m_Texture);
    registerEvent(EventType::WindowFullscreenChanged);
}
Skybox::Skybox(const string& filename){
    m_Texture = nullptr;
    Engine::priv::SkyboxImplInterface::initMesh();

    m_Texture = priv::Core::m_Engine->m_ResourceManager.HasResource<Texture>(filename);
    if (!m_Texture) {
        m_Texture = NEW Texture(filename, false, ImageInternalFormat::SRGB8_ALPHA8);
        priv::TextureLoader::GeneratePBRData(*m_Texture, 32, m_Texture->width() / 4);
        priv::Core::m_Engine->m_ResourceManager._addTexture(m_Texture);
    }
    registerEvent(EventType::WindowFullscreenChanged);
}
Skybox::~Skybox(){
    unregisterEvent(EventType::WindowFullscreenChanged);
}

void Skybox::bindMesh(){
    Engine::priv::SkyboxImplInterface::initMesh();
    if(m_VAO){
        Engine::Renderer::bindVAO(m_VAO);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_Vertices.size()));
    }else{
        Engine::priv::SkyboxImplInterface::bindDataToGPU();
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_Vertices.size()));
        glDisableVertexAttribArray(0);
    }
}
void Skybox::onEvent(const Event& e) {
    if (e.type == EventType::WindowFullscreenChanged) {
        Engine::priv::SkyboxImplInterface::buildVAO();
    }
}
Texture* Skybox::texture() { 
    return m_Texture; 
}
void Skybox::update(){}