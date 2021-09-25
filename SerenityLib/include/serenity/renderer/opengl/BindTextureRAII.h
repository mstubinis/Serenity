#pragma once
#ifndef ENGINE_RENDERER_OPENGL_BIND_TEXTURE_RAII_H
#define ENGINE_RENDERER_OPENGL_BIND_TEXTURE_RAII_H

class Texture;
class TextureCubemap;

#include <string_view>
#include <serenity/resources/texture/TextureIncludes.h>

namespace Engine::priv {
    class OpenGLBindTextureRAII {
        private:
            std::string_view  m_Location;
            GLuint            m_TextureTypeAsGL;
            int               m_Slot              = 0;
            bool              m_Safe              = true;

            void bind(GLuint address);
            void unbind();

            OpenGLBindTextureRAII() = delete;
        public:
            OpenGLBindTextureRAII(const OpenGLBindTextureRAII&)                = delete;
            OpenGLBindTextureRAII& operator=(const OpenGLBindTextureRAII&)     = delete;
            OpenGLBindTextureRAII(OpenGLBindTextureRAII&&) noexcept            = delete;
            OpenGLBindTextureRAII& operator=(OpenGLBindTextureRAII&&) noexcept = delete;

            OpenGLBindTextureRAII(std::string_view location, const Texture&, int slot, bool safe = true);
            OpenGLBindTextureRAII(std::string_view location, const TextureCubemap&, int slot, bool safe = true);
            OpenGLBindTextureRAII(std::string_view location, GLuint address, TextureType, int slot, bool safe = true);
            OpenGLBindTextureRAII(std::string_view location, GLuint address, GLuint textureTypeAsGL, int slot, bool safe = true);

            ~OpenGLBindTextureRAII();
    };
}

#endif