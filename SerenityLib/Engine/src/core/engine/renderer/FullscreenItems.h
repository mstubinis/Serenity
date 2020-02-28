#pragma once
#ifndef ENGINE_FULLSCREEN_ITEMS_H
#define ENGINE_FULLSCREEN_ITEMS_H

#include <vector>
#include <core/engine/events/Engine_EventObject.h>
#include <core/engine/renderer/Renderer.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

typedef unsigned short ushort;

namespace Engine::priv{
    struct MeshVertexDataFullscreen final {
        glm::vec3 position;
        glm::vec2 uv;

        MeshVertexDataFullscreen() {}
        ~MeshVertexDataFullscreen() {}
    };
    class FullscreenTriangle final: public EventObserver{
        private:
            std::vector<ushort>                     m_Indices;
            std::vector<MeshVertexDataFullscreen>   m_Vertices;
            std::vector<GLuint>                     m_Buffers;
            GLuint                                  m_VAO = 0;

            void bindToGPU();
            void buildVAO();
        public:
            FullscreenTriangle() = default;
            ~FullscreenTriangle();

            void init();

            void changeDimensions(const float width, const float height);

            void render();
            void onEvent(const Event& e);
    };
    class FullscreenQuad final: public EventObserver{
        private:
            std::vector<ushort>                     m_Indices;
            std::vector<MeshVertexDataFullscreen>   m_Vertices;
            std::vector<GLuint>                     m_Buffers;
            GLuint                                  m_VAO = 0;

            void bindToGPU();
            void buildVAO();
        public:
            FullscreenQuad() = default;
            ~FullscreenQuad();

            void init();

            void changeDimensions(const float width, const float height);

            void render();
            void onEvent(const Event& e);
    };
};
#endif