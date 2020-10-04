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
    m_Part.m_TextureRequests.emplace_back(std::make_shared<TextureRequest>( diffuse,    false, ImageInternalFormat::SRGB8_ALPHA8, TextureType::Texture2D ));
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
void MaterialRequest::request(bool inAsync) {
    if (inAsync && Engine::hardware_concurrency() > 1) {
        m_Async = true;
        for (auto& textureRequest : m_Part.m_TextureRequests) {
            textureRequest->m_Part.async = true;
        }
    }else{
        m_Async = false;
    }
    InternalMaterialRequestPublicInterface::Request(*this);
}

void InternalMaterialRequestPublicInterface::Request(MaterialRequest& request) {
    request.m_Part.m_Handle = Engine::Resources::addResource<Material>();
    request.m_Part.m_Handle.get<Material>()->setName(request.m_Part.m_Name);
        
    auto size = request.m_Part.m_TextureRequests.size();
    for (size_t i = 0; i < size; ++i) {
        if (request.m_Part.m_TextureRequests[i]->m_FileData.m_FileExists) {
            auto& component = request.m_Part.m_Handle.get<Material>()->addComponent((MaterialComponentType)i, "DEFAULT");
        }
    }

    if (request.m_Async) {
        for (auto& textureRequest : request.m_Part.m_TextureRequests) {
            textureRequest->request(true);
        }
        auto lambda_cpu = [request]() mutable {
            InternalMaterialRequestPublicInterface::LoadCPU(request);
        };
        auto lambda_gpu = [request]() mutable {
            InternalMaterialRequestPublicInterface::LoadGPU(request);
            request.m_Callback();
        };
        threading::addJobWithPostCallback(lambda_cpu, lambda_gpu);
    }else{
        for (auto& textureRequest : request.m_Part.m_TextureRequests) {
            textureRequest->request();
        }
        InternalMaterialRequestPublicInterface::LoadCPU(request);
        InternalMaterialRequestPublicInterface::LoadGPU(request);
        request.m_Callback();
    }
}
void InternalMaterialRequestPublicInterface::LoadCPU(MaterialRequest& request) {
    InternalMaterialPublicInterface::LoadCPU(*request.m_Part.m_Handle.get<Material>());
}
void InternalMaterialRequestPublicInterface::LoadGPU(MaterialRequest& request) {
    const auto& texture_requests = request.m_Part.m_TextureRequests;
    unsigned int count = 0;
    for (const auto& req : texture_requests) {
        if (req->m_FileData.m_FileExists) {
            request.m_Part.m_Handle.get<Material>()->getComponent(count).layer(0).setTexture(req->m_Part.fileOrTextureName);
            ++count;
        }
    }
    InternalMaterialPublicInterface::LoadGPU(*request.m_Part.m_Handle.get<Material>());
}