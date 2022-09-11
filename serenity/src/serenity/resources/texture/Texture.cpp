
#include <serenity/renderer/Renderer.h>
#include <serenity/resources/texture/Texture.h>
#include <SFML/Graphics/Image.hpp>

#include <serenity/renderer/opengl/APIStateOpenGL.h>
#include <serenity/resources/texture/DDS.h>

#include <serenity/threading/ThreadingModule.h>

#include <filesystem>

Handle Texture::White    = {};
Handle Texture::Black    = {};
Handle Texture::Checkers = {};
Handle Texture::BRDF     = {};

namespace Engine::priv {
    void TextureCPUData::init(const TextureConstructorInfo& constructorInfo) {
        auto& image         = m_ImagesDatas[0];
        image.m_Filename    = constructorInfo.filename;
        m_IsToBeMipmapped   = constructorInfo.mipmapped;
        m_Name              = constructorInfo.name.empty() ? constructorInfo.filename : constructorInfo.name;
        m_TextureType       = constructorInfo.type;
        image.setFormats(constructorInfo.internalFormat);


        //for (auto& imageData : m_ImagesDatas) {
        //}

        if (std::filesystem::is_regular_file(constructorInfo.filename)) {
            const std::string extension = std::filesystem::path(constructorInfo.filename).extension().string();
            if (extension == ".dds") {
                Engine::priv::LoadDDSFile(*this, image);
            } else {
                sfImageLoader sfImage(constructorInfo.filename.c_str());
                image.load(sfImage.getPixels(), sfImage.getWidth(), sfImage.getHeight(), constructorInfo.filename);
            }
        } else if (constructorInfo.pixels != nullptr) {
            image.setFormats(image.m_InternalFormat);
            image.load(constructorInfo.pixels, constructorInfo.width, constructorInfo.height, constructorInfo.filename);
        } else if (constructorInfo.usageType == TextureUsage::RenderTarget) {
            assert(constructorInfo.width > 0);
            assert(constructorInfo.height > 0);
            image.load(constructorInfo.width, constructorInfo.height, constructorInfo.pixelType, constructorInfo.pixelFormat, constructorInfo.internalFormat);
        } else {
            assert(false);
        }
        image.setFormats(image.m_InternalFormat);
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
        if (!texture.isLoaded()) {
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
    static const Engine::priv::ImageMipmap* GetFirstMipmap(const Texture& texture) {
        if (!texture.isLoaded() || texture.m_CPUData.m_ImagesDatas.empty() || texture.m_CPUData.m_ImagesDatas[0].m_Mipmaps.empty()) {
            return nullptr;
        }
        auto& imageData = texture.m_CPUData.m_ImagesDatas[0];
        auto& mipmap    = imageData.m_Mipmaps[0];
        return std::addressof(mipmap);
    }
};
Texture::Texture(const TextureConstructorInfo& constructorInfo, bool dispatchEventLoaded) 
    : Resource{ ResourceType::Texture, constructorInfo.name.empty() ? constructorInfo.filename : constructorInfo.name }
{
    auto load_cpu_portion = [realCPUData = std::addressof(m_CPUData)](Engine::priv::TextureCPUData& cpuDataCloned, const TextureConstructorInfo& ci) {
        cpuDataCloned.init(ci);
        if (std::addressof(cpuDataCloned) != realCPUData) {
            (*realCPUData) = std::move(cpuDataCloned);
        }
    };
    auto load_gpu_portion = [
        this, dispatchEventLoaded, 
        xWrapping = constructorInfo.xWrapping, 
        yWrapping = constructorInfo.yWrapping, 
        minFilter = constructorInfo.mipmapped ? constructorInfo.minFilter.toMipmappedType() : constructorInfo.minFilter,
        maxFilter = constructorInfo.maxFilter
    ]() {
        Engine::priv::TextureLoader::LoadGPU(*this, dispatchEventLoaded);
        setXWrapping(xWrapping);
        setYWrapping(yWrapping);
        setMinFilter(minFilter);
        setMaxFilter(maxFilter);
    };

    if (constructorInfo.loadAsync && Engine::hardware_concurrency() > 1) {
        auto load_cpu_async = [load_cpu = std::move(load_cpu_portion), constructorInfo]() {
            Engine::priv::TextureCPUData cpuDataCopy;
            load_cpu(cpuDataCopy, constructorInfo);
        };
        Engine::priv::threading::addJobWithPostCallback(std::move(load_cpu_async), std::move(load_gpu_portion));
    } else {
        load_cpu_portion(m_CPUData, constructorInfo);
        load_gpu_portion();
    }
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
    auto mipmap = Impl::GetFirstMipmap(*this);
    return mipmap ? mipmap->pixels.data() : Engine::opengl::getPixels(*this);
}
int Texture::width() const noexcept {
    auto mipmap = Impl::GetFirstMipmap(*this);
    return mipmap ? mipmap->width : 0;
}
int Texture::height() const noexcept {
    auto mipmap = Impl::GetFirstMipmap(*this);
    return mipmap ? mipmap->height : 0;
}
bool Texture::compressed() const {
    auto mipmap = Impl::GetFirstMipmap(*this);
    return mipmap ? (mipmap->compressedSize > 0) : false;
}
ImageInternalFormat Texture::internalFormat() const noexcept {
    return m_CPUData.m_ImagesDatas.empty() ? ImageInternalFormat::Unknown : m_CPUData.m_ImagesDatas[0].m_InternalFormat;
}
ImagePixelFormat Texture::pixelFormat() const noexcept {
    return m_CPUData.m_ImagesDatas.empty() ? ImagePixelFormat::Unknown : m_CPUData.m_ImagesDatas[0].m_PixelFormat;
}
ImagePixelType Texture::pixelType() const noexcept {
    return m_CPUData.m_ImagesDatas.empty() ? ImagePixelType::Unknown : m_CPUData.m_ImagesDatas[0].m_PixelType;
}