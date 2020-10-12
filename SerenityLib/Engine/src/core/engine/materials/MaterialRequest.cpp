#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/materials/MaterialRequest.h>
#include <core/engine/materials/Material.h>
#include <core/engine/materials/MaterialLoader.h>
#include <core/engine/materials/MaterialComponent.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/system/Engine.h>
#include <boost/filesystem.hpp>

using namespace Engine;
using namespace Engine::priv;

MaterialRequest::MaterialRequest(const std::string& name, const std::string& diffuse, const std::string& normal, const std::string& glow, const std::string& specular, const std::string& ao, const std::string& metalness, const std::string& smoothness, std::function<void()>&& callback)
    : m_Callback{ std::move(callback) }
{
    m_Part.m_Name = name;
    m_Part.m_TextureRequests.emplace_back(std::make_shared<TextureRequest>( diffuse,    false, ImageInternalFormat::SRGB8_ALPHA8, TextureType::Texture2D));
    m_Part.m_TextureRequests.emplace_back(std::make_shared<TextureRequest>( normal,     false, ImageInternalFormat::RGBA8,        TextureType::Texture2D));
    m_Part.m_TextureRequests.emplace_back(std::make_shared<TextureRequest>( glow,       false, ImageInternalFormat::R8,           TextureType::Texture2D));
    m_Part.m_TextureRequests.emplace_back(std::make_shared<TextureRequest>( specular,   false, ImageInternalFormat::R8,           TextureType::Texture2D));
    m_Part.m_TextureRequests.emplace_back(std::make_shared<TextureRequest>( ao,         false, ImageInternalFormat::R8,           TextureType::Texture2D));
    m_Part.m_TextureRequests.emplace_back(std::make_shared<TextureRequest>( metalness,  false, ImageInternalFormat::R8,           TextureType::Texture2D));
    m_Part.m_TextureRequests.emplace_back(std::make_shared<TextureRequest>( smoothness, false, ImageInternalFormat::R8,           TextureType::Texture2D));
}
MaterialRequest::MaterialRequest(const std::string& name, Handle diffuse, Handle normal, Handle glow, Handle specular, Handle ao, Handle metalness, Handle smoothness, std::function<void()>&& callback)
    : m_Callback{ std::move(callback) }
{
    m_Part.m_Name   = name;
    m_Part.m_Handle = Engine::Resources::addResource<Material>(name, diffuse, normal, glow, specular, ao, metalness, smoothness);
}
void MaterialRequest::internal_set_async(bool inAsync) noexcept {
    if (inAsync && Engine::hardware_concurrency() > 1) {
        m_Async = true;
        for (auto& textureRequest : m_Part.m_TextureRequests) {
            textureRequest->m_Part.async = true;
        }
    }else{
        m_Async = false;
    }
}
void MaterialRequest::internal_init_material_components() noexcept {
    auto size = m_Part.m_TextureRequests.size();
    for (size_t i = 0; i < size; ++i) {
        if (m_Part.m_TextureRequests[i]->m_FileData.m_FileExists) {
            auto& component = m_Part.m_Handle.get<Material>()->addComponent((MaterialComponentType)i, "DEFAULT");
        }
    }
}
void MaterialRequest::internal_void_launch_texture_requests(bool async) noexcept {
    for (auto& textureRequest : m_Part.m_TextureRequests) {
        if (!textureRequest->m_Part.m_CPUData.m_Name.empty()) {
            textureRequest->request(async);
        }
    }
}
void MaterialRequest::request(bool inAsync) {
    internal_set_async(inAsync);

    //launch the request
    m_Part.m_Handle = Engine::Resources::addResource<Material>();
    m_Part.m_Handle.get<Material>()->setName(m_Part.m_Name);

    internal_init_material_components();
    internal_void_launch_texture_requests(m_Async);

    auto l_gpu = [materialRequest{ *this }]() mutable {
        const auto& texture_requests = materialRequest.m_Part.m_TextureRequests;
        unsigned int count = 0;
        for (const auto& textureRequest : texture_requests) {
            if (textureRequest->m_FileData.m_FileExists) {
                materialRequest.m_Part.m_Handle.get<Material>()->getComponent(count).layer(0).setTexture(textureRequest->m_Part.handle);
                ++count;
            }
        }
        InternalMaterialPublicInterface::LoadGPU(*materialRequest.m_Part.m_Handle.get<Material>());
        materialRequest.m_Callback();
    };

    if (m_Async || !Engine::priv::threading::isMainThread()) {
        if (Engine::priv::threading::isMainThread()) {
            Engine::priv::threading::addJobWithPostCallback([]() {}, std::move(l_gpu), 1U);
        }else{
            Engine::priv::threading::submitTaskForMainThread([g{ std::move(l_gpu) }]() mutable { 
                threading::addJobWithPostCallback([]() {}, std::move(g), 1U); 
            });
        }
    }else{
        l_gpu();
    }
}

