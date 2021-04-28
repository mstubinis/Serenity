#include <serenity/resources/texture/TextureBaseClass.h>
#include <serenity/renderer/Renderer.h>


bool Engine::priv::TextureBaseClass::internal_bind_if_not_bound(TextureType type, uint32_t requestedAddress) noexcept {
    auto whichID = Engine::Renderer::getCurrentlyBoundTextureOfType(requestedAddress);
    if (whichID != requestedAddress) {
        Engine::Renderer::bindTextureForModification(type, requestedAddress);
        return true;
    }
    return false;
}
void Engine::priv::TextureBaseClass::internal_anisotropic_filtering(TextureType type , float anisotropicFiltering) noexcept {
    if (Engine::priv::OpenGLState::constants.supportsAniosotropicFiltering()) {
        glTexParameterf(type.toGLType(), GL_TEXTURE_MAX_ANISOTROPY, anisotropicFiltering);
    }else{
        if (Engine::priv::OpenGLExtensions::supported(Engine::priv::OpenGLExtensions::ARB_texture_filter_anisotropic)) {
            glTexParameterf(type.toGLType(), GL_TEXTURE_MAX_ANISOTROPY, anisotropicFiltering);
        }else if (Engine::priv::OpenGLExtensions::supported(Engine::priv::OpenGLExtensions::EXT_texture_filter_anisotropic)) {
            glTexParameterf(type.toGLType(), GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropicFiltering);
        }
    }
}
void Engine::priv::TextureBaseClass::setXWrapping(TextureType type, TextureWrap wrap) {
    glTexParameteri(type.toGLType(), GL_TEXTURE_WRAP_S, wrap.toGLType());
}
void Engine::priv::TextureBaseClass::setYWrapping(TextureType type, TextureWrap wrap) {
    glTexParameteri(type.toGLType(), GL_TEXTURE_WRAP_T, wrap.toGLType());
}
void Engine::priv::TextureBaseClass::setWrapping(TextureType type, TextureWrap wrap) {
    Engine::priv::TextureBaseClass::setXWrapping(type, wrap);
    Engine::priv::TextureBaseClass::setYWrapping(type, wrap);
}