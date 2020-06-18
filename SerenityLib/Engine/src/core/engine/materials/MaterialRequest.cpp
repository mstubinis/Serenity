#include <core/engine/materials/MaterialRequest.h>
#include <core/engine/materials/Material.h>
#include <core/engine/materials/MaterialLoader.h>
#include <core/engine/materials/MaterialComponent.h>
#include <core/engine/textures/Texture.h>

#include <core/engine/system/Engine.h>

#include <boost/filesystem.hpp>

#include <iostream>

using namespace std;
using namespace Engine;
using namespace Engine::priv;


MaterialRequestPart::MaterialRequestPart() {
}
MaterialRequestPart::~MaterialRequestPart() {

}
MaterialRequestPart::MaterialRequestPart(const MaterialRequestPart& other) {
    material = other.material;
    name     = other.name;
    handle   = other.handle;
}
MaterialRequestPart& MaterialRequestPart::operator=(const MaterialRequestPart& other) {
    if (&other != this) {
        material = other.material;
        name     = other.name;
        handle   = other.handle;
    }
    return *this;
}



MaterialRequest::MaterialRequest(const string& name, const string& diffuse, const string& normal, const string& glow, const string& specular, const string& ao, const string& metalness, const string& smoothness){ 
    part.name = name;
    part.textureRequests.emplace_back(  NEW TextureRequest( diffuse, false, ImageInternalFormat::SRGB8_ALPHA8 )  );
    part.textureRequests.emplace_back(  NEW TextureRequest( normal, false, ImageInternalFormat::RGBA8 )  );
    part.textureRequests.emplace_back(  NEW TextureRequest( glow, false, ImageInternalFormat::R8 )  );
    part.textureRequests.emplace_back(  NEW TextureRequest( specular, false, ImageInternalFormat::R8 )  );
    part.textureRequests.emplace_back(  NEW TextureRequest( ao, false, ImageInternalFormat::R8 )  );
    part.textureRequests.emplace_back(  NEW TextureRequest( metalness, false, ImageInternalFormat::R8 )  );
    part.textureRequests.emplace_back(  NEW TextureRequest( smoothness, false, ImageInternalFormat::R8 )  );
}
MaterialRequest::MaterialRequest(const string& name, Texture* diffuse, Texture* normal, Texture* glow, Texture* specular, Texture* ao, Texture* metalness, Texture* smoothness) {
    part.name     = name;
    part.material = NEW Material(name, diffuse, normal, glow, specular, ao, metalness, smoothness);
    part.handle   = Core::m_Engine->m_ResourceManager.m_Resources.add(part.material, ResourceType::Material);
}

MaterialRequest::~MaterialRequest() {

}
MaterialRequest::MaterialRequest(const MaterialRequest& other) {
    async                = other.async;
    part.handle          = other.part.handle;
    part.material        = other.part.material;
    part.name            = other.part.name;
    part.textureRequests = other.part.textureRequests;
}
MaterialRequest& MaterialRequest::operator=(const MaterialRequest& other) {
    if (&other != this) {
        async                = other.async;
        part.handle          = other.part.handle;
        part.material        = other.part.material;
        part.name            = other.part.name;
        part.textureRequests = other.part.textureRequests;
    }
    return *this;
}



void MaterialRequest::request() {
    async = false;
    InternalMaterialRequestPublicInterface::Request(*this);
}
void MaterialRequest::requestAsync() {
    if (Engine::hardware_concurrency() > 1) {
        async = true;
        for (auto& textureRequest : part.textureRequests) {
            textureRequest->async = true;
        }
        InternalMaterialRequestPublicInterface::Request(*this);
    }else{
        MaterialRequest::request();
    }
}

void InternalMaterialRequestPublicInterface::Request(MaterialRequest& request) {
    request.part.material = NEW Material();
    request.part.material->setName(request.part.name);
    request.part.handle = Core::m_Engine->m_ResourceManager.m_Resources.add(request.part.material, ResourceType::Material);

    const auto size = request.part.textureRequests.size();
    for (size_t i = 0; i < size; ++i) {
        if (request.part.textureRequests[i]->fileExists) {
            auto& component = request.part.material->addComponent(static_cast<MaterialComponentType::Type>(i), "DEFAULT");
        }
    }

    if (request.async) {
        for (auto& textureRequest : request.part.textureRequests) {
            textureRequest->requestAsync();
        }
        auto job = [=]() { 
            InternalMaterialRequestPublicInterface::LoadCPU(const_cast<MaterialRequest&>(request));
        };
        auto callback = [=]() { 
            InternalMaterialRequestPublicInterface::LoadGPU(const_cast<MaterialRequest&>(request));
        };
        threading::addJobWithPostCallback(job, callback);
    }else{
        for (auto& textureRequest : request.part.textureRequests) {
            textureRequest->request();
        }
        InternalMaterialRequestPublicInterface::LoadCPU(request);
        InternalMaterialRequestPublicInterface::LoadGPU(request);
    }
}
void InternalMaterialRequestPublicInterface::LoadCPU(MaterialRequest& request) {
    InternalMaterialPublicInterface::LoadCPU(*request.part.material);
}
void InternalMaterialRequestPublicInterface::LoadGPU(MaterialRequest& request) {
    const auto& texture_requests = request.part.textureRequests;
    unsigned int count = 0;
    for (size_t i = 0; i < texture_requests.size(); ++i) {
        if (texture_requests[i]->fileExists) {
            request.part.material->getComponent(count).layer(0).setTexture(texture_requests[i]->file);
            ++count;
        }
    }
    InternalMaterialPublicInterface::LoadGPU(*request.part.material);
}