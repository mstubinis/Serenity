#include <serenity/renderer/opengl/APIStateOpenGL.h>
//#include <serenity/renderer/opengl/OpenGLContext.h>
#include <serenity/system/Engine.h>

#include <serenity/resources/texture/Texture.h>
#include <serenity/resources/texture/TextureCubemap.h>

#include <serenity/renderer/FramebufferObject.h>

namespace {
    bool internal_setAnisotropicFiltering(TextureType textureType, float anisotropicFiltering) {
        if (Engine::priv::APIState<Engine::priv::OpenGL>::supportsAniosotropicFiltering()) {
            anisotropicFiltering = glm::clamp(anisotropicFiltering, 1.0f, Engine::priv::APIState<Engine::priv::OpenGL>::getConstants().MAX_TEXTURE_MAX_ANISOTROPY);
            glTexParameterf(textureType.toGLType(), GL_TEXTURE_MAX_ANISOTROPY, anisotropicFiltering);
            return true;
        }
        return false;
    }
}


namespace Engine::priv::detail::opengl {
    class Impl {
    public:
        static void setMinFilter(auto& texture, TextureFilter filter) noexcept {
            Engine::priv::Core::m_APIManager->getOpenGL().GL_glBindTextureForModification(texture.getTextureType().toGLType(), texture.address());
            Engine::opengl::setMinFilter(texture.getTextureType(), filter);
        }
        static void setFilter(auto& texture, TextureFilter filter) noexcept {
            Engine::priv::Core::m_APIManager->getOpenGL().GL_glBindTextureForModification(texture.getTextureType().toGLType(), texture.address());
            glTexParameteri(texture.getTextureType().toGLType(), GL_TEXTURE_MIN_FILTER, filter.toGLType(true));
            glTexParameteri(texture.getTextureType().toGLType(), GL_TEXTURE_MAG_FILTER, filter.toGLType(false));
        }
        static bool generateMipmaps(auto& texture) {
            if (texture.m_CPUData.m_Mipmapped || texture.address() == 0) {
                return false;
            }
            Engine::priv::Core::m_APIManager->getOpenGL().GL_glBindTextureForModification(texture.getTextureType().toGLType(), texture.address());
            glTexParameteri(texture.getTextureType().toGLType(), GL_TEXTURE_BASE_LEVEL, 0);
            GLint minFilterGL = 0;
            glGetTexParameteriv(texture.getTextureType().toGLType(), GL_TEXTURE_MIN_FILTER, &minFilterGL);
            if (minFilterGL == GL_LINEAR) {
                minFilterGL = GL_LINEAR_MIPMAP_LINEAR;
            } else if (minFilterGL == GL_NEAREST) {
                minFilterGL = GL_NEAREST_MIPMAP_LINEAR;
            }
            glTexParameteri(texture.getTextureType().toGLType(), GL_TEXTURE_MIN_FILTER, minFilterGL);
            glGenerateMipmap(texture.getTextureType().toGLType());
            texture.m_CPUData.m_Mipmapped = true;
            return true;
        }
        static const uint8_t* getPixels(auto& texture, uint32_t imageIndex, uint32_t mipmapLevel) {
            auto& image = texture.m_CPUData.m_ImagesDatas[imageIndex];
            auto& pxls  = image.m_Mipmaps[mipmapLevel].pixels;
            pxls.resize(image.m_Mipmaps[mipmapLevel].width * image.m_Mipmaps[mipmapLevel].height * 4);
            Engine::priv::Core::m_APIManager->getOpenGL().GL_glBindTextureForModification(texture.getTextureType().toGLType(), texture.address());
            glGetTexImage(texture.getTextureType().toGLType(), 0, image.m_PixelFormat, image.m_PixelType, pxls.data());
            return pxls.data();
        }
        static const uint8_t* getPixels(auto& texture) {
            return getPixels(texture, 0, 0);
        }
        static void createTexImage2D(auto& texture, const Engine::priv::ImageMipmap& mipmap, TextureType textureType) {
            auto& imageData = texture.m_CPUData.m_ImagesDatas[0];
            if (imageData.m_InternalFormat.isCompressedType() && mipmap.compressedSize != 0) {
                glCompressedTexImage2D(textureType.toGLType(), mipmap.level, imageData.m_InternalFormat, mipmap.width, mipmap.height, 0, mipmap.compressedSize, mipmap.pixels.data());
            } else {
                glTexImage2D(textureType.toGLType(), mipmap.level, imageData.m_InternalFormat, mipmap.width, mipmap.height, 0, imageData.m_PixelFormat, imageData.m_PixelType, mipmap.pixels.data());
            }
        }
        static void deleteTexture(auto& texture) {
            glDeleteTextures(1, &texture.m_TextureAddress);
        }
        static bool genTexture(auto& texture) {
            if (texture.m_TextureAddress == 0) {
                glGenTextures(1, &texture.m_TextureAddress);
                return texture.m_TextureAddress != 0;
            }
            return false;
        }
        static void bindTexture(auto& texture) {
            glBindTexture(texture.getTextureType().toGLType(), texture.m_TextureAddress);
        }
    };
}

namespace Engine::opengl {
    bool supportsInstancing() noexcept {
        return Engine::priv::APIState<Engine::priv::OpenGL>::supportsInstancing();
    }
    bool supportsInstancingAdvanced() noexcept {
        return Engine::priv::APIState<Engine::priv::OpenGL>::isVersionGreaterOrEqualTo(3, 2);
    }

    bool setAnisotropicFiltering(Texture& texture, float anisotropicFiltering) noexcept {
        Engine::priv::Core::m_APIManager->getOpenGL().GL_glBindTextureForModification(texture.getTextureType().toGLType(), texture.address());
        return internal_setAnisotropicFiltering(texture.getTextureType(), anisotropicFiltering);
    }
    bool setAnisotropicFiltering(TextureCubemap& cubemap, float anisotropicFiltering) noexcept {
        Engine::priv::Core::m_APIManager->getOpenGL().GL_glBindTextureForModification(cubemap.getTextureType().toGLType(), cubemap.address());
        return internal_setAnisotropicFiltering(cubemap.getTextureType(), anisotropicFiltering);
    }

    void setXWrapping(Texture& texture, TextureWrap wrap) noexcept {
        Engine::priv::Core::m_APIManager->getOpenGL().GL_glBindTextureForModification(texture.getTextureType().toGLType(), texture.address());
        glTexParameteri(texture.getTextureType().toGLType(), GL_TEXTURE_WRAP_S, wrap.toGLType());
    }
    void setXWrapping(TextureCubemap& cubemap, TextureWrap wrap) noexcept {
        Engine::priv::Core::m_APIManager->getOpenGL().GL_glBindTextureForModification(cubemap.getTextureType().toGLType(), cubemap.address());
        glTexParameteri(cubemap.getTextureType().toGLType(), GL_TEXTURE_WRAP_S, wrap.toGLType());
    }
    void setYWrapping(Texture& texture, TextureWrap wrap) noexcept {
        Engine::priv::Core::m_APIManager->getOpenGL().GL_glBindTextureForModification(texture.getTextureType().toGLType(), texture.address());
        glTexParameteri(texture.getTextureType().toGLType(), GL_TEXTURE_WRAP_T, wrap.toGLType());
    }
    void setYWrapping(TextureCubemap& cubemap, TextureWrap wrap) noexcept {
        Engine::priv::Core::m_APIManager->getOpenGL().GL_glBindTextureForModification(cubemap.getTextureType().toGLType(), cubemap.address());
        glTexParameteri(cubemap.getTextureType().toGLType(), GL_TEXTURE_WRAP_T, wrap.toGLType());
    }
    void setZWrapping(TextureCubemap& cubemap, TextureWrap wrap) noexcept {
        Engine::priv::Core::m_APIManager->getOpenGL().GL_glBindTextureForModification(cubemap.getTextureType().toGLType(), cubemap.address());
        glTexParameteri(cubemap.getTextureType().toGLType(), GL_TEXTURE_WRAP_R, wrap.toGLType());
    }
    void setWrapping(Texture& texture, TextureWrap wrap) noexcept {
        Engine::priv::Core::m_APIManager->getOpenGL().GL_glBindTextureForModification(texture.getTextureType().toGLType(), texture.address());
        glTexParameteri(texture.getTextureType().toGLType(), GL_TEXTURE_WRAP_S, wrap.toGLType());
        glTexParameteri(texture.getTextureType().toGLType(), GL_TEXTURE_WRAP_T, wrap.toGLType());
    }
    void setWrapping(TextureCubemap& cubemap, TextureWrap wrap) noexcept {
        Engine::priv::Core::m_APIManager->getOpenGL().GL_glBindTextureForModification(cubemap.getTextureType().toGLType(), cubemap.address());
        glTexParameteri(cubemap.getTextureType().toGLType(), GL_TEXTURE_WRAP_S, wrap.toGLType());
        glTexParameteri(cubemap.getTextureType().toGLType(), GL_TEXTURE_WRAP_T, wrap.toGLType());
        glTexParameteri(cubemap.getTextureType().toGLType(), GL_TEXTURE_WRAP_R, wrap.toGLType());
    }



    void setMinFilter(TextureType type, TextureFilter filter) noexcept {
        glTexParameteri(type.toGLType(), GL_TEXTURE_MIN_FILTER, filter.toGLType(true));
    }
    void setMaxFilter(TextureType type, TextureFilter filter) noexcept {
        glTexParameteri(type.toGLType(), GL_TEXTURE_MAG_FILTER, filter.toGLType(false));
    }
    void setFilter(TextureType type, TextureFilter filter) noexcept {
        glTexParameteri(type.toGLType(), GL_TEXTURE_MIN_FILTER, filter.toGLType(true));
        glTexParameteri(type.toGLType(), GL_TEXTURE_MAG_FILTER, filter.toGLType(false));
    }
    void setMinFilter(Texture& texture, TextureFilter filter) noexcept {
        Engine::priv::detail::opengl::Impl::setMinFilter(texture, filter);
    }
    void setMinFilter(TextureCubemap& cubemap, TextureFilter filter) noexcept {
        Engine::priv::detail::opengl::Impl::setMinFilter(cubemap, filter);
    }
    void setMaxFilter(Texture& texture, TextureFilter filter) noexcept {
        Engine::priv::Core::m_APIManager->getOpenGL().GL_glBindTextureForModification(texture.getTextureType().toGLType(), texture.address());
        glTexParameteri(texture.getTextureType().toGLType(), GL_TEXTURE_MAG_FILTER, filter.toGLType(false));
    }
    void setMaxFilter(TextureCubemap& cubemap, TextureFilter filter) noexcept {
        Engine::priv::Core::m_APIManager->getOpenGL().GL_glBindTextureForModification(cubemap.getTextureType().toGLType(), cubemap.address());
        glTexParameteri(cubemap.getTextureType().toGLType(), GL_TEXTURE_MAG_FILTER, filter.toGLType(false));
    }
    void setFilter(Texture& texture, TextureFilter filter) noexcept {
        Engine::priv::detail::opengl::Impl::setFilter(texture, filter);
    }
    void setFilter(TextureCubemap& cubemap, TextureFilter filter) noexcept {
        Engine::priv::detail::opengl::Impl::setFilter(cubemap, filter);
    }

    bool generateMipmaps(Texture& texture) {
        return Engine::priv::detail::opengl::Impl::generateMipmaps(texture);
    }
    bool generateMipmaps(TextureCubemap& cubemap) {
        return Engine::priv::detail::opengl::Impl::generateMipmaps(cubemap);
    }

    const uint8_t* getPixels(Texture& texture, uint32_t imageIndex, uint32_t mipmapLevel) {
        return Engine::priv::detail::opengl::Impl::getPixels(texture, imageIndex, mipmapLevel);
    }
    const uint8_t* getPixels(Texture& texture) {
        return Engine::priv::detail::opengl::Impl::getPixels(texture);
    }
    const uint8_t* getPixels(TextureCubemap& cubemap, uint32_t imageIndex, uint32_t mipmapLevel) {
        return Engine::priv::detail::opengl::Impl::getPixels(cubemap, imageIndex, mipmapLevel);
    }
    const uint8_t* getPixels(TextureCubemap& cubemap) {
        return Engine::priv::detail::opengl::Impl::getPixels(cubemap);
    }


    bool genTexture(Texture& texture) {
        return Engine::priv::detail::opengl::Impl::genTexture(texture);
    }
    bool genTexture(TextureCubemap& cubemap) {
        return Engine::priv::detail::opengl::Impl::genTexture(cubemap);
    }
    bool genTexture(uint32_t& address) {
        if (address == 0) {
            glGenTextures(1, &address);
            return address != 0;
        }
        return false;
    }
    void bindTexture(Texture& texture) {
        Engine::priv::detail::opengl::Impl::bindTexture(texture);
    }
    void bindTexture(TextureCubemap& cubemap) {
        Engine::priv::detail::opengl::Impl::bindTexture(cubemap);
    }
    void bindTexture(TextureType textureType, uint32_t& address) {
        glBindTexture(textureType.toGLType(), address);
    }


    void createTexImage2D(Texture& texture, const Engine::priv::ImageMipmap& mipmap, TextureType textureType) {
        Engine::priv::detail::opengl::Impl::createTexImage2D(texture, mipmap, textureType);
    }
    void createTexImage2D(TextureCubemap& cubemap, const Engine::priv::ImageMipmap& mipmap, TextureType textureType) {
        Engine::priv::detail::opengl::Impl::createTexImage2D(cubemap, mipmap, textureType);
    }
    void deleteTexture(Texture& texture) noexcept {
        Engine::priv::detail::opengl::Impl::deleteTexture(texture);
    }
    void deleteTexture(TextureCubemap& cubemap) noexcept {
        Engine::priv::detail::opengl::Impl::deleteTexture(cubemap);
    }


    bool bindFBO(uint32_t fbo) noexcept {
        auto& openglManager = Engine::priv::Core::m_APIManager->getOpenGL();
        return openglManager.GL_glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo) && openglManager.GL_glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    }
    bool bindFBO(Engine::priv::FramebufferObject& fbo) noexcept {
        return bindFBO(fbo.address());
    }
    bool bindReadFBO(uint32_t fbo) noexcept {
        auto& openglManager = Engine::priv::Core::m_APIManager->getOpenGL();
        return openglManager.GL_glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    }
    bool bindDrawFBO(uint32_t fbo) noexcept {
        auto& openglManager = Engine::priv::Core::m_APIManager->getOpenGL();
        return openglManager.GL_glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    }
    bool unbindFBO() noexcept {
        auto& openglManager = Engine::priv::Core::m_APIManager->getOpenGL();
        return openglManager.GL_glBindFramebuffer(GL_READ_FRAMEBUFFER, 0) && openglManager.GL_glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }


    bool bindRBO(Engine::priv::RenderbufferObject& rbo) noexcept {
        return bindRBO(rbo.address());
    }
    bool bindRBO(uint32_t rbo) noexcept {
        auto& openglManager = Engine::priv::Core::m_APIManager->getOpenGL();
        return openglManager.GL_glBindRenderbuffer(rbo);
    }
    bool unbindRBO() noexcept {
        return bindRBO(0);
    }
    bool colorMask(bool r, bool g, bool b, bool a) noexcept {
        auto& openglManager = Engine::priv::Core::m_APIManager->getOpenGL();
        return openglManager.GL_glColorMask(r, g, b, a);
    }
    bool setViewport(float x, float y, float width, float height) noexcept {
        auto& openglManager = Engine::priv::Core::m_APIManager->getOpenGL();
        return openglManager.GL_glViewport(GLint(x), GLint(y), GLsizei(width), GLsizei(height));
    }
    bool setViewport(const glm::vec4& dimensions) noexcept {
        return setViewport(dimensions.x, dimensions.y, dimensions.z, dimensions.w);
    }
    bool setViewport(const Viewport& viewport) noexcept {
        auto& openglManager = Engine::priv::Core::m_APIManager->getOpenGL();
        const auto& dimensions = viewport.getViewportDimensions();
        return setViewport(dimensions.x, dimensions.y, dimensions.z, dimensions.w);
    }
    bool drawBuffers(int32_t size, uint32_t* buffers) noexcept {
        glDrawBuffers(GLsizei(size), (GLenum*)buffers);
        return true;
    }
}