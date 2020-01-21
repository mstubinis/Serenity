#pragma once
#ifndef ENGINE_FULLSCREEN_ITEMS_H
#define ENGINE_FULLSCREEN_ITEMS_H

#include <vector>
#include <core/engine/events/Engine_EventObject.h>
#include <core/engine/renderer/Renderer.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

typedef unsigned short ushort;

namespace Engine{
namespace priv{
    struct MeshVertexDataFullscreen final {
        glm::vec3 position;
        glm::vec2 uv;
    };
    class FullscreenTriangle final: public EventObserver{
        private:
            std::vector<ushort>                     m_Indices;
            std::vector<MeshVertexDataFullscreen>   m_Vertices;
            std::vector<GLuint>                     m_Buffers;
            GLuint                                  m_VAO;

            void bindToGPU();
            void buildVAO();
        public:
            FullscreenTriangle();
            ~FullscreenTriangle();

            void render();
            void onEvent(const Event& e);
    };
    class FullscreenQuad final: public EventObserver{
        private:
            std::vector<ushort>                     m_Indices;
            std::vector<MeshVertexDataFullscreen>   m_Vertices;
            std::vector<GLuint>                     m_Buffers;
            GLuint                                  m_VAO;

            void bindToGPU();
            void buildVAO();
        public:
            FullscreenQuad();
            ~FullscreenQuad();

            void render();
            void onEvent(const Event& e);
    };
};
};
#endif