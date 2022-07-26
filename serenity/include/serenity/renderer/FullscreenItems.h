#pragma once
#ifndef ENGINE_FULLSCREEN_ITEMS_H
#define ENGINE_FULLSCREEN_ITEMS_H

#include <serenity/events/Observer.h>
#include <serenity/renderer/Renderer.h>
#include <serenity/resources/mesh/gl/VertexArrayObject.h>
#include <serenity/resources/mesh/gl/VertexBufferObject.h>
#include <serenity/resources/mesh/gl/IndexBufferObject.h>

namespace Engine::priv {
    struct MeshVertexDataFullscreen {
        glm::vec3 position = glm::vec3{ 0.0f };
        glm::vec2 uv       = glm::vec2{ 0.0f };
    };
    class FullscreenTriangle final: public Observer {
        private:
            std::vector<uint16_t>                   m_Indices;
            std::vector<MeshVertexDataFullscreen>   m_Vertices;

            IndexBufferObject                       m_IBO;
            VertexBufferObject                      m_VBO;
            VertexArrayObject                       m_VAO;

            void bindToGPU();
            void buildVAO();
        public:
            FullscreenTriangle() = default;
            FullscreenTriangle(const FullscreenTriangle&) = delete;
            FullscreenTriangle& operator=(const FullscreenTriangle&) = delete;
            FullscreenTriangle(FullscreenTriangle&&) noexcept = delete;
            FullscreenTriangle& operator=(FullscreenTriangle&&) noexcept = delete;

            void init();

            void changeDimensions(float width, float height);

            void render();
            void onEvent(const Event&);
    };
    class FullscreenQuad final: public Observer {
        private:
            std::vector<uint16_t>                   m_Indices;
            std::vector<MeshVertexDataFullscreen>   m_Vertices;

            IndexBufferObject                       m_IBO;
            VertexBufferObject                      m_VBO;
            VertexArrayObject                       m_VAO;

            void bindToGPU();
            void buildVAO();
        public:
            FullscreenQuad() = default;
            FullscreenQuad(const FullscreenQuad&) = delete;
            FullscreenQuad& operator=(const FullscreenQuad&) = delete;
            FullscreenQuad(FullscreenQuad&&) noexcept = delete;
            FullscreenQuad& operator=(FullscreenQuad&&) noexcept = delete;

            void init();

            void changeDimensions(float width, float height);

            void render();
            void onEvent(const Event&);
    };
};
#endif