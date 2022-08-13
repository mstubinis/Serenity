#pragma once
#ifndef ENGINE_TEXTURE_BASE_CLASS_H
#define ENGINE_TEXTURE_BASE_CLASS_H

#include <queue>
#include <functional>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include <serenity/renderer/opengl/GLImageConstants.h>
#include <serenity/resources/texture/TextureIncludes.h>

namespace Engine::priv {
    class TextureBaseClass {
        protected:
#ifdef TEXTURE_COMMAND_QUEUE
            std::queue<std::function<void()>>    m_CommandQueue;              //for commands that were not available until the texture was properly loaded
#endif
            GLuint                               m_TextureAddress = 0;

            [[nodiscard]] GLuint& internal_get_address_for_generation() noexcept { return m_TextureAddress; }
        public:
            TextureBaseClass() = default;
            TextureBaseClass(const TextureBaseClass&)            = default;
            TextureBaseClass& operator=(const TextureBaseClass&) = default;
            TextureBaseClass(TextureBaseClass&& other) noexcept 
#ifdef TEXTURE_COMMAND_QUEUE
                : m_CommandQueue{ std::move(other.m_CommandQueue) }
                , m_TextureAddress{ std::exchange(other.m_TextureAddress, 0) }
#else
                : m_TextureAddress{ std::exchange(other.m_TextureAddress, 0) }
#endif
            {}
            TextureBaseClass& operator=(TextureBaseClass&& other) noexcept {
#ifdef TEXTURE_COMMAND_QUEUE
                m_CommandQueue   = std::move(other.m_CommandQueue);
#endif
                m_TextureAddress = std::exchange(other.m_TextureAddress, 0);
                return *this;
            }

            [[nodiscard]] inline GLuint address() const noexcept { return m_TextureAddress; }
    };
}

#endif