
#include <serenity/renderer/Renderer.h>
#include <serenity/resources/texture/Texture.h>
#include <SFML/Graphics/Image.hpp>

#include <serenity/renderer/opengl/APIStateOpenGL.h>
#include <serenity/resources/texture/DDS.h>

#include <filesystem>

Handle Texture::White    = {};
Handle Texture::Black    = {};
Handle Texture::Checkers = {};
Handle Texture::BRDF     = {};

namespace Engine::priv {
    void TextureCPUData::initFromMemory(const uint8_t* pixels, int inWidth, int inHeight) {
        auto& image = m_ImagesDatas[0];
        image.setInternalFormat(image.m_InternalFormat);
        image.load(pixels, inWidth, inHeight, image.m_Filename);
    }
    void TextureCPUData::initFromFile() {
        auto& image = m_ImagesDatas[0];
        const std::string extension = std::filesystem::path(image.m_Filename).extension().string();
        if (extension == ".dds") {
            Engine::priv::LoadDDSFile(*this, image);
        }
        image.setInternalFormat(image.m_InternalFormat);
    }
}

class Texture::Impl {
public:
    static void Load(Texture& texture, bool dispatchEventLoaded) {
        if (!texture.isLoaded()) {
            Engine::priv::TextureLoader::LoadCPU(texture.m_CPUData, Handle{});
            Engine::priv::TextureLoader::LoadGPU(texture, dispatchEventLoaded);
        }
    }
    static void Unload(Texture& texture) {
        if (texture.isLoaded()) {
            Engine::priv::TextureLoader::UnloadGPU(texture);
        }
    }
    static bool AddToCommandQueue(Texture& texture, auto func, auto&&... args) {
#ifdef TEXTURE_COMMAND_QUEUE
        if (texture == false) {
            texture.m_CommandQueue.emplace([&func, &texture, ... args = std::forward<decltype(args)>(args)]() {
                ((std::addressof(texture))->*func)(args...);
            });
            return true;
        }
        return false;
#else
        assert(texture == true);
        return false;
#endif
    }
};

Texture::Texture(const TextureConstructorInfo& constructorInfo, bool dispatchEventLoaded) 
    : Resource{ ResourceType::Texture, constructorInfo.name.empty() ? constructorInfo.filename : constructorInfo.name }
{
    m_CPUData.m_ImagesDatas[0].m_Filename = constructorInfo.filename;
    m_CPUData.m_Name                      = constructorInfo.name.empty() ? constructorInfo.filename : constructorInfo.name;
    m_CPUData.m_TextureType               = constructorInfo.type;
    m_CPUData.m_IsToBeMipmapped           = constructorInfo.mipmapped;

    m_CPUData.m_ImagesDatas[0].setInternalFormat(constructorInfo.internalFormat);
    //m_CPUData.initFromFile(); TODO: figure this out
    Impl::Load(*this, dispatchEventLoaded);
}
Texture::Texture(std::string_view textureName, TextureType textureType, bool mipMap, bool dispatchEventLoaded)
    : Resource{ ResourceType::Texture, textureName }
{
    //setName(std::string{ textureName }); //TODO: is this needed?
    m_CPUData.m_ImagesDatas[0].m_Filename = textureName;
    m_CPUData.m_Name                      = textureName;
    m_CPUData.m_TextureType               = textureType;
    m_CPUData.m_IsToBeMipmapped           = mipMap;
}
Texture::Texture(std::string_view filename, bool genMipMaps, ImageInternalFormat intFmt, TextureType textureType, bool dispatchEventLoaded)
    : Texture{ filename, textureType, genMipMaps }
{
    m_CPUData.m_ImagesDatas[0].setInternalFormat(intFmt);
    m_CPUData.initFromFile();
    Impl::Load(*this, dispatchEventLoaded);
}
Texture::Texture(uint32_t w, uint32_t h, ImagePixelType pxlType, ImagePixelFormat pxlFmt, ImageInternalFormat intFmt, float divisor, bool dispatchEventLoaded)
    : Texture{ "RenderTarget", TextureType::RenderTarget, false }
{
    int width       = int(float(w) * divisor);
    int height      = int(float(h) * divisor);
    m_CPUData.m_ImagesDatas[0].load(width, height, pxlType, pxlFmt, intFmt);
    Engine::priv::TextureLoader::LoadGPU(*this, dispatchEventLoaded); //nothing to load cpu side for frame buffers
}
Texture::Texture(uint8_t* pixels, uint32_t width, uint32_t height, std::string_view name, bool genMipMaps, ImageInternalFormat intFmt, TextureType textureType, bool dispatchEventLoaded)
    : Texture{ name, textureType, genMipMaps }
{
    m_CPUData.m_ImagesDatas[0].setInternalFormat(intFmt);
    m_CPUData.initFromMemory(pixels, width, height);
    Impl::Load(*this, dispatchEventLoaded);
}


Texture::Texture(Texture&& other) noexcept 
    : Resource(std::move(other))
    , Engine::priv::TextureBaseClass(std::move(other))
    , m_CPUData{ std::move(other.m_CPUData) }
{}
Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        Resource::operator=(std::move(other));
        Engine::priv::TextureBaseClass::operator=(std::move(other));
        m_CPUData = std::move(other.m_CPUData);
    }
    return *this;
}

Texture::~Texture(){
    Impl::Unload(*this);
}

bool Texture::generateMipmaps() {
    if (Impl::AddToCommandQueue(*this, &Texture::generateMipmaps)) {
        return false;
    }
    return Engine::opengl::generateMipmaps(*this);
}
void Texture::setXWrapping(TextureWrap wrap) {
    if (Impl::AddToCommandQueue(*this, &Texture::setXWrapping, wrap)) {
        return;
    }
    Engine::opengl::setXWrapping(*this, wrap);
}
void Texture::setYWrapping(TextureWrap wrap) {
    if (Impl::AddToCommandQueue(*this, &Texture::setYWrapping, wrap)) {
        return;
    }
    Engine::opengl::setYWrapping(*this, wrap);
}
void Texture::setWrapping(TextureWrap wrap) {
    if (Impl::AddToCommandQueue(*this, &Texture::setWrapping, wrap)) {
        return;
    }
    Engine::opengl::setWrapping(*this, wrap);
}
void Texture::setMinFilter(TextureFilter filter) {
    if (Impl::AddToCommandQueue(*this, &Texture::setMinFilter, filter)) {
        return;
    }
    Engine::opengl::setMinFilter(*this, filter);
}
void Texture::setMaxFilter(TextureFilter filter) {
    if (Impl::AddToCommandQueue(*this, &Texture::setMaxFilter, filter)) {
        return;
    }
    Engine::opengl::setMaxFilter(*this, filter);
}
void Texture::setFilter(TextureFilter filter) {
    if (Impl::AddToCommandQueue(*this, &Texture::setFilter, filter)) {
        return;
    }
    Engine::opengl::setFilter(*this, filter);
}
void Texture::setAnisotropicFiltering(float anisotropicFiltering) {
    if (Impl::AddToCommandQueue(*this, &Texture::setAnisotropicFiltering, anisotropicFiltering)) {
        return;
    }
    Engine::opengl::setAnisotropicFiltering(*this, anisotropicFiltering);
}
const uint8_t* Texture::pixels() {
    if (!m_CPUData.m_ImagesDatas.empty() && !m_CPUData.m_ImagesDatas[0].m_Mipmaps.empty()) {
        return m_CPUData.m_ImagesDatas[0].m_Mipmaps[0].pixels.data();
    }
    return Engine::opengl::getPixels(*this);
}
