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
            std::queue<std::function<void()>>    m_CommandQueue;              //for commands that were not available until the texture was properly loaded
            GLuint                               m_TextureAddress = 0;

            [[nodiscard]] GLuint& internal_get_address_for_generation() noexcept { return m_TextureAddress; }
            bool internal_bind_if_not_bound(TextureType, uint32_t requestedAddress) noexcept;
            void internal_anisotropic_filtering(TextureType, float aniso) noexcept;
        public:
            TextureBaseClass() = default;
            TextureBaseClass(const TextureBaseClass&)            = default;
            TextureBaseClass& operator=(const TextureBaseClass&) = default;
            TextureBaseClass(TextureBaseClass&& other) noexcept 
                : m_CommandQueue{ std::move(other.m_CommandQueue) }
                , m_TextureAddress{ std::exchange(other.m_TextureAddress, 0) }
            {}
            TextureBaseClass& operator=(TextureBaseClass&& other) noexcept {
                m_CommandQueue   = std::move(other.m_CommandQueue);
                m_TextureAddress = std::exchange(other.m_TextureAddress, 0);
                return *this;
            }

            [[nodiscard]] inline GLuint address() const noexcept { return m_TextureAddress; }

            static void setXWrapping(TextureType, TextureWrap);
            static void setYWrapping(TextureType, TextureWrap);
            static void setWrapping(TextureType, TextureWrap);
    };
}

#endif