#pragma once
#ifndef ENGINE_FULLSCREEN_ITEMS_H
#define ENGINE_FULLSCREEN_ITEMS_H

#include <core/engine/events/Observer.h>
#include <core/engine/renderer/Renderer.h>

namespace Engine::priv{
    struct MeshVertexDataFullscreen final {
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec2 uv       = glm::vec2(0.0f);

        MeshVertexDataFullscreen() = default;
        ~MeshVertexDataFullscreen() = default;
    };
    class FullscreenTriangle final: public Observer{
        private:
            std::vector<unsigned short>             m_Indices;
            std::vector<MeshVertexDataFullscreen>   m_Vertices;
            std::vector<GLuint>                     m_Buffers;
            GLuint                                  m_VAO = 0;

            void bindToGPU();
            void buildVAO();
        public:
            FullscreenTriangle() = default;
            ~FullscreenTriangle();

            void init();

            void changeDimensions(float width, float height);

            void render();
            void onEvent(const Event& e);
    };
    class FullscreenQuad final: public Observer{
        private:
            std::vector<unsigned short>             m_Indices;
            std::vector<MeshVertexDataFullscreen>   m_Vertices;
            std::vector<GLuint>                     m_Buffers;
            GLuint                                  m_VAO = 0;

            void bindToGPU();
            void buildVAO();
        public:
            FullscreenQuad() = default;
            ~FullscreenQuad();

            void init();

            void changeDimensions(float width, float height);

            void render();
            void onEvent(const Event& e);
    };
};
#endif