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
    m_Part.m_TextureRequests.emplace_back(std::make_shared<TextureRequest>( diffuse,    false, ImageInternalFormat::SRGB8_ALPHA8, GL_TEXTURE_2D ));
    m_Part.m_TextureRequests.emplace_back(std::make_shared<TextureRequest>( normal,     false, ImageInternalFormat::RGBA8,        GL_TEXTURE_2D ));
    m_Part.m_TextureRequests.emplace_back(std::make_shared<TextureRequest>( glow,       false, ImageInternalFormat::R8,           GL_TEXTURE_2D ));
    m_Part.m_TextureRequests.emplace_back(std::make_shared<TextureRequest>( specular,   false, ImageInternalFormat::R8,           GL_TEXTURE_2D ));
    m_Part.m_TextureRequests.emplace_back(std::make_shared<TextureRequest>( ao,         false, ImageInternalFormat::R8,           GL_TEXTURE_2D ));
    m_Part.m_TextureRequests.emplace_back(std::make_shared<TextureRequest>( metalness,  false, ImageInternalFormat::R8,           GL_TEXTURE_2D ));
    m_Part.m_TextureRequests.emplace_back(std::make_shared<TextureRequest>( smoothness, false, ImageInternalFormat::R8,           GL_TEXTURE_2D ));
}
MaterialRequest::MaterialRequest(const std::string& name, Texture* diffuse, Texture* normal, Texture* glow, Texture* specular, Texture* ao, Texture* metalness, Texture* smoothness, std::function<void()>&& callback) 
    : m_Callback{ std::move(callback) }
{
    m_Part.m_Name     = name;
    m_Part.m_Material = NEW Material(name, diffuse, normal, glow, specular, ao, metalness, smoothness);
    m_Part.m_Handle   = Core::m_Engine->m_ResourceManager.m_ResourcePool.add(m_Part.m_Material, (unsigned int)ResourceType::Material);
}

MaterialRequest::~MaterialRequest() {

}

void MaterialRequest::request(bool inAsync) {
    if (inAsync && Engine::hardware_concurrency() > 1) {
        m_Async = true;
        for (auto& textureRequest : m_Part.m_TextureRequests) {
            textureRequest->part.async = true;
        }
    }else{
        m_Async = false;
    }
    InternalMaterialRequestPublicInterface::Request(*this);
}

void InternalMaterialRequestPublicInterface::Request(MaterialRequest& request) {
    request.m_Part.m_Material = NEW Material();
    request.m_Part.m_Material->setName(request.m_Part.m_Name);
    request.m_Part.m_Handle = Core::m_Engine->m_ResourceManager.m_ResourcePool.add(request.m_Part.m_Material, (unsigned int)ResourceType::Material);

    auto size = request.m_Part.m_TextureRequests.size();
    for (size_t i = 0; i < size; ++i) {
        if (request.m_Part.m_TextureRequests[i]->fileExists) {
            auto& component = request.m_Part.m_Material->addComponent((MaterialComponentType)i, "DEFAULT");
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
    InternalMaterialPublicInterface::LoadCPU(*request.m_Part.m_Material);
}
void InternalMaterialRequestPublicInterface::LoadGPU(MaterialRequest& request) {
    const auto& texture_requests = request.m_Part.m_TextureRequests;
    unsigned int count = 0;
    for (const auto& req : texture_requests) {
        if (req->fileExists) {
            request.m_Part.m_Material->getComponent(count).layer(0).setTexture(req->file);
            ++count;
        }
    }
    InternalMaterialPublicInterface::LoadGPU(*request.m_Part.m_Material);
}