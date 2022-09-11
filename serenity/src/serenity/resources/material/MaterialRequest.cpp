
#include <serenity/resources/material/MaterialRequest.h>
#include <serenity/resources/material/Material.h>
#include <serenity/resources/material/MaterialLoader.h>
#include <serenity/resources/material/MaterialComponent.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/system/Engine.h>
#include <boost/filesystem.hpp>

using namespace Engine;
using namespace Engine::priv;

MaterialRequest::MaterialRequest(bool async, std::string_view name, std::string_view diffuse, std::string_view normal, std::string_view glow, std::string_view specular, std::string_view ao, std::string_view metalness, std::string_view smoothness, Engine::ResourceCallback&& callback)
    : m_Callback{ std::move(callback) }
{
    m_Name = name;

    auto make_constructor_info = [async](std::string_view fileOrName, ImageInternalFormat internalFormat) {
        TextureConstructorInfo ci;
        ci.internalFormat = internalFormat;
        ci.loadAsync      = async;
        ci.filename       = fileOrName;
        return ci;
    };

    std::vector<TextureConstructorInfo> constructors;
    const size_t numTextures = 7;
    constructors.reserve(numTextures);
    m_TexturesValid.reserve(numTextures);
    m_Textures.reserve(numTextures);
    constructors.push_back(make_constructor_info(diffuse,    ImageInternalFormat::SRGB8_ALPHA8));
    constructors.push_back(make_constructor_info(normal,     ImageInternalFormat::RGBA8));
    constructors.push_back(make_constructor_info(glow,       ImageInternalFormat::R8));
    constructors.push_back(make_constructor_info(specular,   ImageInternalFormat::R8));
    constructors.push_back(make_constructor_info(metalness,  ImageInternalFormat::R8));
    constructors.push_back(make_constructor_info(smoothness, ImageInternalFormat::R8));
    constructors.push_back(make_constructor_info(ao,         ImageInternalFormat::R8));

    for (const auto& constructor : constructors) {
        m_TexturesValid.push_back(!constructor.filename.empty());
        m_Textures.push_back(!constructor.filename.empty() ? Engine::Resources::addResource<Texture>(constructor) : Handle{});
    }

    m_Async = (async && Engine::hardware_concurrency() > 1);

    m_Handle = Engine::Resources::addResource<Material>();
    m_Handle.get<Material>()->setName(m_Name);

    for (size_t i = 0; i < m_Textures.size(); ++i) {
        if (m_TexturesValid[i]) {
            auto& component = m_Handle.get<Material>()->addComponent(uint32_t(i), {});
        }
    }

    auto load_gpu = [handle = m_Handle, textures = m_Textures, callback = m_Callback]() {
        for (size_t i = 0; i < textures.size(); ++i) {
            if (textures[i]) {
                handle.get<Material>()->getComponent(uint32_t(i)).getLayer(0).setTexture(textures[i]);
            }
        }
        PublicMaterial::LoadGPU(*handle.get<Material>());
        callback(handle);
    };

    if (m_Async || !Engine::priv::threading::isMainThread()) {
        Engine::priv::threading::addJobWithPostCallback([]() {}, std::move(load_gpu));
    } else {
        load_gpu();
    }
}
MaterialRequest::MaterialRequest(bool async, std::string_view name, Handle diffuse, Handle normal, Handle glow, Handle specular, Handle ao, Handle metalness, Handle smoothness, Engine::ResourceCallback&& callback)
    : m_Callback{ std::move(callback) }
{
    m_Name   = name;
    m_Handle = Engine::Resources::addResource<Material>(name, diffuse, normal, glow, specular, ao, metalness, smoothness);
}
