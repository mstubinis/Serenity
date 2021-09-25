#include <serenity/renderer/opengl/BindTextureRAII.h>
#include <serenity/renderer/Renderer.h>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <serenity/resources/texture/Texture.h>
#include <serenity/resources/texture/TextureCubemap.h>

Engine::priv::OpenGLBindTextureRAII::OpenGLBindTextureRAII(std::string_view location, GLuint address, GLuint textureTypeAsGL, int slot, bool safe)
    : m_TextureTypeAsGL{ textureTypeAsGL }
    , m_Location{ location }
    , m_Slot{ slot }
    , m_Safe{ safe }
{
    bind(address);
}
Engine::priv::OpenGLBindTextureRAII::OpenGLBindTextureRAII(std::string_view location, const Texture& texture, int slot, bool safe)
    : OpenGLBindTextureRAII{ location, texture.address(), texture.getTextureType().toGLType(), slot, safe }
{}
Engine::priv::OpenGLBindTextureRAII::OpenGLBindTextureRAII(std::string_view location, const TextureCubemap& cubemap, int slot, bool safe)
    : OpenGLBindTextureRAII{ location, cubemap.address(), cubemap.getTextureType().toGLType(), slot, safe }
{}
Engine::priv::OpenGLBindTextureRAII::OpenGLBindTextureRAII(std::string_view location, GLuint address, TextureType textureType, int slot, bool safe) 
    : OpenGLBindTextureRAII{ location, address, textureType.toGLType(), slot, safe }
{}
Engine::priv::OpenGLBindTextureRAII::~OpenGLBindTextureRAII() {
    unbind();
}
void Engine::priv::OpenGLBindTextureRAII::bind(GLuint address) {
    if (m_Safe) {
        Engine::Renderer::sendTextureSafe(m_Location.data(), address, m_Slot, m_TextureTypeAsGL);
    } else {
        Engine::Renderer::sendTexture(m_Location.data(), address, m_Slot, m_TextureTypeAsGL);
    }
}
void Engine::priv::OpenGLBindTextureRAII::unbind() {
    if (m_Safe) {
        Engine::Renderer::sendTextureSafe(m_Location.data(), 0, m_Slot, m_TextureTypeAsGL);
    } else {
        Engine::Renderer::sendTexture(m_Location.data(), 0, m_Slot, m_TextureTypeAsGL);
    }
}